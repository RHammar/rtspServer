#include <string.h>
#include <gst/gst.h>
#include "pipelinebuilder.h"

static void
handle_new_pad_mp4(GstElement *decodebin, GstPad *pad, GstElement *pipeline);

static void
handle_new_pad(GstElement *decodebin, GstPad *pad,
               GstElement *target);

static void
add_ogg_audio_payloader(GstElement *pipeline, GstPad *pad);

static void
add_ogg_video_payloader(GstElement *pipeline, GstPad *pad);

static void
add_mp4_audio_payloader(GstElement *pipeline, GstPad *pad);

static void
add_mp4_video_payloader(GstElement *pipeline, GstPad *pad);

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

static void
add_ogg_audio_payloader(GstElement *pipeline, GstPad *pad)
{
  GstElement *element, *rtpvorbispay, *audioqueue;
  GstPad *sinkpad, *srcpad, *sink;
  gchar *padname;

  // element = gst_bin_new(NULL);
  audioqueue = create_element("queue", NULL);
  rtpvorbispay = create_element("rtpvorbispay", "pay0");
  g_object_set(rtpvorbispay, "pt", 96, NULL);
  gst_bin_add_many(GST_BIN(pipeline), audioqueue, rtpvorbispay, NULL);
  gst_element_link_many(audioqueue, rtpvorbispay, NULL);

  /* Get the target sink and link it */
  sink = gst_element_get_static_pad(audioqueue, "sink");
  /* try and link the pads */
  gst_pad_link(pad, sink);
  // sinkpad = gst_element_get_static_pad(audioqueue, "sink");
  // gst_element_add_pad(element, gst_ghost_pad_new("sink", sinkpad));
  // gst_object_unref(sinkpad);

  // srcpad = gst_element_get_static_pad(rtpvorbispay, "src");
  // padname = gst_pad_get_name (srcpad);
  // gst_element_add_pad(element, gst_ghost_pad_new(padname, srcpad));
  // gst_object_unref(srcpad);

  // return element;
}

static void
add_ogg_video_payloader(GstElement *pipeline, GstPad *pad)
{
  GstElement *element, *theorapay, *videoqueue;
  GstPad *srcpad, *ghostPad, *sink;
  gboolean ok;
  gchar *padname;

  // element = gst_bin_new(NULL);
  videoqueue = create_element("queue", NULL);
  theorapay = create_element("rtptheorapay", "pay1");
  g_object_set(theorapay, "pt", 97, NULL);
  gst_bin_add_many(GST_BIN(pipeline), videoqueue, theorapay, NULL);
  if (!gst_element_link_many(videoqueue, theorapay, NULL))
  {
    g_print("failed to link\n");
  }

  sink = gst_element_get_static_pad(videoqueue, "sink");
  if (gst_pad_is_linked(pad))
  {
    g_print("pad is linked\n");
  }
  gst_pad_link(pad, sink);
  // ghostPad = gst_ghost_pad_new("sink", pad);
  // if (!gst_element_add_pad(element, ghostPad))
  // {
  //   g_print("failed to add pad\n");
  // }
  // gst_object_unref(pad);

  // srcpad = gst_element_get_static_pad(theorapay, "src");
  // padname = gst_pad_get_name (srcpad);
  // gst_element_add_pad(element, gst_ghost_pad_new(padname, srcpad));
  // gst_pad_set_active(srcpad,TRUE);
  // gst_object_unref(srcpad);

  // return element;
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
    add_ogg_audio_payloader(pipeline, pad);
  }
  else if (strcmp(name, "video/x-theora") == 0)
  {
    g_print("got video\n");
    add_ogg_video_payloader(pipeline, pad);
  }

  // ret = gst_bin_add(GST_BIN_CAST(pipeline), target);
  // if (ret == GST_STATE_CHANGE_FAILURE)
  //   goto state_change_error;

  // /* When adding elements to a started pipeline, need to synchronize
  //  * the element state explicitly */
  // ret = gst_element_sync_state_with_parent(target);

  // /* Get the target sink ghostpad and link it */
  // sink = gst_element_get_static_pad(GST_ELEMENT_CAST(target), "sink");

  // /* try and link the pads */
  // if (gst_pad_link(pad, sink) != GST_PAD_LINK_OK)
  // {
  //   gst_object_unref(sink);
  //   goto link_error;
  // }

  // srcpad = gst_element_get_static_pad(target, "src");
  // padname = gst_pad_get_name (srcpad);
  // gst_pad_set_active(srcpad,TRUE);
  // gst_element_add_pad(pipeline, gst_ghost_pad_new(padname, srcpad));
  // gst_object_unref(srcpad);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(GST_ELEMENT_PARENT(pipeline)), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
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
  // return gst_parse_launch("( filesrc location=/home/rickardh/Videos/big_buck_bunny.ogv ! oggdemux name=d  d. ! queue ! rtptheorapay name=pay0 pt=96 d. ! queue ! rtpvorbispay name=pay1 pt=97 )", NULL);

  return gst_parse_launch("filesrc location=/home/rickardh/Videos/big_buck_bunny.mp4 ! qtdemux name=d d. ! queue ! rtph264pay name=pay0 pt=96 d. ! queue ! rtpmp4apay name=pay1 pt=97", NULL);
}

static void
no_more_pads_ogg_cb (GstElement * oggmux, GstElement * element)
{
    g_print("no-more-pads\n");
    gst_element_no_more_pads (element);
}

GstElement *
createOggPipeline()
{
  GstElement *pipeline, *src, *oggdemux, *oggparse;
  g_print("creating ogg pipeline\n");

  // return parse_launch();
  // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "gst_parse_pipe");
  // return pipeline;

  // pipeline = create_element("pipeline", NULL);
  pipeline = gst_bin_new(NULL);
  g_object_set(pipeline, "name", "oggbin", NULL);
  src = create_element("filesrc", "src");
  g_object_set(src, "location", "/home/rickardh/Videos/small.ogv", NULL);
  oggparse = create_element("oggparse", NULL);
  oggdemux = create_element("oggdemux", NULL);
  gst_bin_add_many(GST_BIN(pipeline), src, oggparse, oggdemux, NULL);
  gst_element_link_many(src, oggdemux, NULL);
  g_signal_connect(oggdemux, "pad-added", G_CALLBACK(handle_new_pad), pipeline);
  g_signal_connect(oggdemux, "no-more-pads", (GCallback) no_more_pads_ogg_cb,
          pipeline);

  return pipeline;
}

static void
add_mp4_audio_payloader(GstElement *pipeline, GstPad *pad)
{
  GstElement *rtpmp4apay, *audioqueue;
  GstPad *sinkpad, *srcpad, *ghostpad;
  gchar *padname;

  audioqueue = create_element("queue", NULL);
  rtpmp4apay = create_element("rtpmp4apay", "pay0");
  g_object_set(rtpmp4apay, "pt", 96, NULL);
  gst_bin_add_many(GST_BIN(pipeline), audioqueue, rtpmp4apay, NULL);
  gst_element_link_many(audioqueue, rtpmp4apay, NULL);

  /* Get the target sinkpad and link it */
  sinkpad = gst_element_get_static_pad(audioqueue, "sink");
  /* try and link the pads */
  gst_pad_link(pad, sinkpad);

  /* now expose the srcpad of the payloader as a ghostpad with the same name
   * as the decoder pad name. */
  srcpad = gst_element_get_static_pad (rtpmp4apay, "src");
  padname = gst_pad_get_name (pad);
  ghostpad = gst_ghost_pad_new (padname, srcpad);
  gst_object_unref (srcpad);
  g_free (padname);

  gst_element_sync_state_with_parent(audioqueue);
  gst_element_sync_state_with_parent(rtpmp4apay);
  
  gst_pad_set_active (ghostpad, TRUE);
  gst_element_add_pad (pipeline, ghostpad);

}

static void
add_mp4_video_payloader(GstElement *pipeline, GstPad *pad)
{
  GstElement *rtph264pay, *videoqueue;
  GstPad *srcpad, *ghostpad, *sinkpad;
  gboolean ok;
  gchar *padname;

  videoqueue = create_element("queue", NULL);
  rtph264pay = create_element("rtph264pay", "pay1");
  g_object_set(rtph264pay, "pt", 97, NULL);
  gst_bin_add_many(GST_BIN(pipeline), videoqueue, rtph264pay, NULL);
  if (!gst_element_link_many(videoqueue, rtph264pay, NULL))
  {
    g_print("failed to link\n");
  }
  gst_element_set_state (videoqueue, GST_STATE_PLAYING);
  gst_element_set_state (rtph264pay, GST_STATE_PLAYING);
  sinkpad = gst_element_get_static_pad(videoqueue, "sink");
  gst_pad_link(pad, sinkpad);

  /* now expose the srcpad of the payloader as a ghostpad with the same name
   * as the decoder pad name. */
  srcpad = gst_element_get_static_pad (rtph264pay, "src");
  padname = gst_pad_get_name (pad);
  ghostpad = gst_ghost_pad_new (padname, srcpad);
  gst_object_unref (srcpad);
  g_free (padname);

  gst_pad_set_active (ghostpad, TRUE);
  gst_element_add_pad (pipeline, ghostpad);


  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(GST_ELEMENT_PARENT(pipeline)), GST_DEBUG_GRAPH_SHOW_ALL, "mp4");
}

static void
handle_new_pad_mp4(GstElement *decodebin, GstPad *pad, GstElement *pipeline)
{
  GstStructure *str;
  const gchar *name;
  GstCaps *caps;

  caps = gst_pad_get_current_caps(pad);
  str = gst_caps_get_structure(caps, 0);
  name = gst_structure_get_name(str);
  g_print("got caps: %s\n", name);
  if (strcmp(name, "audio/mpeg") == 0)
  {
    g_print("got audio\n");
    add_mp4_audio_payloader(pipeline, pad);
  }
  else if (strcmp(name, "video/x-h264") == 0)
  {
    g_print("got video\n");
    add_mp4_video_payloader(pipeline, pad);
  }
}

static void
no_more_pads_mp4_cb (GstElement * qtdemux, GstElement * element)
{
    g_print("no-more-pads\n");
    gst_element_no_more_pads (element);
}


GstElement *
createMp4Pipeline()
{
  GstElement *pipeline, *src, *qtdemux, *element;

  g_print("creating mp4 pipeline\n");

  // return parse_launch();
  /* our bin will dynamically expose payloaded pads */
  pipeline = gst_bin_new ("dynpay0");
  //pipeline = gst_bin_new(NULL);
  //g_object_set(pipeline, "name", "mp4bin", NULL);
  src = create_element("filesrc", "src");
  g_object_set(src, "location", "/home/rickardh/Videos/big_buck_bunny.mp4", NULL);
  qtdemux = create_element("qtdemux", NULL);
  gst_bin_add_many(GST_BIN(pipeline), src, qtdemux, NULL);
  gst_element_link_many(src, qtdemux, NULL);
  g_signal_connect(qtdemux, "pad-added", G_CALLBACK(handle_new_pad_mp4), pipeline);
  g_signal_connect (qtdemux, "no-more-pads", (GCallback) no_more_pads_mp4_cb,
          pipeline);
  //gst_bin_add (GST_BIN_CAST (element), pipeline);
  return pipeline;

  //filesrc location=/home/rickardh/Videos/big_buck_bunny.mp4 ! qtdemux name=d d. ! queue ! avdec_h264 ! autovideosink d. ! queue ! avdec_aac ! audioconvert ! audioresample ! autoaudiosink
}
