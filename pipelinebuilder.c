#include <string.h>
#include <gst/gst.h>
#include "pipelinebuilder.h"

static void
handle_new_pad(GstElement *decodebin, GstPad *pad,
               GstElement *target);

static GstElement *
gen_audio_output(void);

static GstElement *
gen_video_output(void);

static GstElement *
create_element(const gchar *type, const gchar *name)
{
  GstElement *e;

  e = gst_element_factory_make(type, name);
  if (!e)
  {
    g_print("Failed to create element %s\n", type);
    return NULL;
  }

  return e;
}

static GstElement *
gen_audio_output(void)
{
  GstElement *element, *rtpvorbispay, *audioqueue;
  GstPad *sinkpad, *srcpad;
  gchar *padname;

  element = gst_bin_new(NULL);
  audioqueue = create_element("queue", NULL);
  rtpvorbispay = create_element("rtpvorbispay", "pay0");
  g_object_set(rtpvorbispay, "pt", 96, NULL);
  gst_bin_add_many(GST_BIN(element), audioqueue, rtpvorbispay, NULL);
  gst_element_link_many(audioqueue, rtpvorbispay, NULL);

  sinkpad = gst_element_get_static_pad(audioqueue, "sink");
  gst_element_add_pad(element, gst_ghost_pad_new("sink", sinkpad));
  gst_object_unref(sinkpad);

  srcpad = gst_element_get_static_pad(rtpvorbispay, "src");
  padname = gst_pad_get_name (srcpad);
  gst_element_add_pad(element, gst_ghost_pad_new(padname, srcpad));
  gst_object_unref(srcpad);

  return element;
}

static GstElement *
gen_video_output(void)
{
  GstElement *element, *theorapay, *videoqueue;
  GstPad *pad, *srcpad, *ghostPad;
  gboolean ok;
  gchar *padname;

  element = gst_bin_new(NULL);
  videoqueue = create_element("queue", NULL);
  theorapay = create_element("rtptheorapay", "pay1");
  g_object_set(theorapay, "pt", 97, NULL);
  gst_bin_add_many(GST_BIN(element), videoqueue, theorapay, NULL);
  if (!gst_element_link_many(videoqueue, theorapay, NULL))
  {
    g_print("failed to link\n");
  }

  pad = gst_element_get_static_pad(videoqueue, "sink");
  if (gst_pad_is_linked(pad))
  {
    g_print("pad is linked\n");
  }
  ghostPad = gst_ghost_pad_new("sink", pad);
  if (!gst_element_add_pad(element, ghostPad))
  {
    g_print("failed to add pad\n");
  }
  gst_object_unref(pad);

  srcpad = gst_element_get_static_pad(theorapay, "src");
  padname = gst_pad_get_name (srcpad);
  gst_element_add_pad(element, gst_ghost_pad_new(padname, srcpad));
  gst_pad_set_active(srcpad,TRUE);
  gst_object_unref(srcpad);

  return element;
}

static void
handle_new_pad(GstElement *decodebin, GstPad *pad, GstElement *pipeline)
{
  GstElement *target;
  GstCaps *caps;
  GstStructure *str;
  const gchar *name;
  gchar *padname;
  GstPad *sink, *srcpad;
  GstStateChangeReturn ret;

  caps = gst_pad_get_current_caps(pad);
  str = gst_caps_get_structure(caps, 0);
  name = gst_structure_get_name(str);
  g_print("got caps: %s\n", name);
  /* Check the caps type in the enclosed structure */
  if (strcmp(name, "audio/x-vorbis") == 0)
  {
    g_print("got audio\n");
    target = gen_audio_output();
  }
  else if (strcmp(name, "video/x-theora") == 0)
  {
    g_print("got video\n");
    target = gen_video_output();
  }

  ret = gst_bin_add(GST_BIN_CAST(pipeline), target);
  if (ret == GST_STATE_CHANGE_FAILURE)
    goto state_change_error;

  /* When adding elements to a started pipeline, need to synchronize
   * the element state explicitly */
  ret = gst_element_sync_state_with_parent(target);

  /* Get the target sink ghostpad and link it */
  sink = gst_element_get_static_pad(GST_ELEMENT_CAST(target), "sink");

  /* try and link the pads */
  if (gst_pad_link(pad, sink) != GST_PAD_LINK_OK)
  {
    gst_object_unref(sink);
    goto link_error;
  }

  srcpad = gst_element_get_static_pad(target, "src");
  padname = gst_pad_get_name (srcpad);
  gst_pad_set_active(srcpad,TRUE);
  gst_element_add_pad(pipeline, gst_ghost_pad_new(padname, srcpad));
  gst_object_unref(srcpad);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
  return;

link_error:
  {
    gchar *capsstr;
    gchar *pad_name;

    caps = gst_pad_get_current_caps(pad);
    capsstr = gst_caps_to_string(caps);
    pad_name = gst_pad_get_name(pad);
    g_print("Failed to link decodebin pad %s with caps %s to output chain\n",
            pad_name, capsstr);

    gst_caps_unref(caps);
    g_free(capsstr);
    g_free(pad_name);

    gst_element_set_state(target, GST_STATE_NULL);
    gst_bin_remove(GST_BIN_CAST(pipeline), target);
    return;
  }

state_change_error:
  g_print("Failed to set the state of output chain\n");
  gst_bin_remove(GST_BIN_CAST(pipeline), target);
  return;
}

GstElement *
parse_launch()
{
  return gst_parse_launch("( filesrc location=/home/rickardh/Videos/big_buck_bunny.ogv ! oggdemux name=d  d. ! queue ! rtptheorapay name=pay0 pt=96 d. ! queue ! rtpvorbispay name=pay1 pt=97 )", NULL);

  // return gst_parse_launch("filesrc location=/home/rickardh/Videos/big_buck_bunny.ogv ! oggdemux name=d d. ! queue ! rtptheorapay name=pay0 pt=96 d. ! queue ! rtpvorbispay name=pay1 pt=97", NULL);
}

GstElement *
createOggPipeline()
{
  GstElement *pipeline, *src, *oggdemux;
  g_print("creating ogg pipeline\n");

  return parse_launch();
  // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "gst_parse_pipe");
  // return pipeline;


  // pipeline = create_element("pipeline", NULL);
  pipeline = gst_bin_new(NULL);
  src = create_element("filesrc", "src");
  g_object_set(src, "location", "/home/rickardh/Videos/small.ogv", NULL);
  oggdemux = create_element("oggdemux", NULL);
  gst_bin_add_many(GST_BIN(pipeline), src, oggdemux, NULL);
  gst_element_link_many(src, oggdemux, NULL);
  g_signal_connect(oggdemux, "pad-added", G_CALLBACK(handle_new_pad), pipeline);

  return pipeline;
}
