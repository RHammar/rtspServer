#include <string.h>
#include <gst/gst.h>
#include <gst/rtsp/gstrtsptransport.h>
#include "rtspproxypipeline.h"
#include "log.h"

static void
handle_new_pad(GstElement *rtspsrc, GstPad *pad, GstElement *pipeline);
static void
no_more_pads_cb(GstElement *rtspsrc, GstElement *element);
static void
add_video_payloder(GstElement *pipeline, GstPad *pad);
static void
add_audio_payloder(GstElement *pipeline, GstPad *pad);

static GstElement *
create_element(const gchar *type, const gchar *name)
{
  GstElement *e;

  e = gst_element_factory_make(type, name);
  if (!e)
  {
    PDEBUG("Failed to create element %s", type);
    return NULL;
  }

  return e;
}

GstElement *
createRtspProxyPipeline(const gchar *uri, const gchar *proxy)
{
  GstElement *pipeline, *rtspsrc, *element;

  PDEBUG("uri: %s, proxy: %s", uri, proxy);
  pipeline = gst_bin_new("dynpay0");
  rtspsrc = create_element("rtspsrc", "src");
  g_object_set(rtspsrc, "location", uri, NULL);
  if (proxy)
  {
    g_object_set(rtspsrc, "proxy", proxy, NULL);
    g_object_set(rtspsrc, "protocols", GST_RTSP_LOWER_TRANS_HTTP, NULL);
  }
  else
  {
    g_object_set(rtspsrc, "protocols", GST_RTSP_LOWER_TRANS_TCP, NULL);
  }
  gst_bin_add(GST_BIN(pipeline), rtspsrc);
  g_signal_connect(rtspsrc, "pad-added", G_CALLBACK(handle_new_pad), pipeline);
  g_signal_connect(rtspsrc, "no-more-pads", (GCallback)no_more_pads_cb,
                   pipeline);

  return pipeline;
}

static void
handle_new_pad(GstElement *rtspsrc, GstPad *pad, GstElement *pipeline)
{
  GstElement *target;
  GstCaps *caps;
  GstStructure *str;
  const gchar *name, *media;
  gchar *padname;
  GstPad *sink, *srcpad;
  GstStateChangeReturn ret;

  caps = gst_pad_get_current_caps(pad);
  str = gst_caps_get_structure(caps, 0);
  name = gst_structure_get_name(str);
  media = gst_structure_get_string(str, "media");
  PDEBUG("got caps: %s %s", name, media);
  /* Check the caps type in the enclosed structure */
  if (strcmp(media, "audio") == 0)
  {
    PDEBUG("got audio");
    add_audio_payloder(pipeline, pad);
  }
  else if (strcmp(media, "video") == 0)
  {
    PDEBUG("got video");
    add_video_payloder(pipeline, pad);
  }
}

static void
no_more_pads_cb(GstElement *rtspsrc, GstElement *element)
{
  PDEBUG("no-more-pads");
  gst_element_no_more_pads(element);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(element), GST_DEBUG_GRAPH_SHOW_ALL, "rtsp-media");
}

static void
add_audio_payloder(GstElement *pipeline, GstPad *pad)
{
  GstElement  *audioqueue, *rtpmp4gdepay, *aacparse, *avdec_aac, *audioconvert, *audioresample, *voaacenc, *rtpmp4gpay;
  GstPad *sinkpad, *srcpad, *ghostpad;
  gchar *padname;

  audioqueue = create_element("queue", NULL);
  rtpmp4gdepay = create_element("rtpmp4gdepay", NULL);
  aacparse = create_element("aacparse", NULL);
  avdec_aac = create_element("avdec_aac", NULL);
  audioconvert = create_element("audioconvert", NULL);
  audioresample = create_element("audioresample", NULL);
  voaacenc = create_element("voaacenc", NULL);
  rtpmp4gpay = create_element("rtpmp4gpay", "pay1");
  g_object_set(rtpmp4gpay, "pt", 97, NULL);
  gst_bin_add_many(GST_BIN(pipeline), audioqueue, rtpmp4gdepay, aacparse, avdec_aac, audioconvert, audioresample, voaacenc, rtpmp4gpay, NULL);
  gst_element_link_many(audioqueue, rtpmp4gdepay, aacparse, avdec_aac, audioconvert, audioresample, voaacenc, rtpmp4gpay, NULL);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  /* Get the target sinkpad and link it */
  sinkpad = gst_element_get_static_pad(audioqueue, "sink");
  /* try and link the pads */
  gst_pad_link(pad, sinkpad);

  /* now expose the srcpad of the payloader as a ghostpad with the same name
   * as the decoder pad name. */
  srcpad = gst_element_get_static_pad (rtpmp4gpay, "src");
  padname = gst_pad_get_name (pad);
  ghostpad = gst_ghost_pad_new (padname, srcpad);
  gst_object_unref (srcpad);
  g_free (padname);

  //gst_element_sync_state_with_parent(audioqueue);
  //gst_element_sync_state_with_parent(rtpmp4gdepay);
   //gst_element_set_state (audioqueue, GST_STATE_PAUSED);

  gst_pad_set_active (ghostpad, TRUE);
  gst_element_add_pad (pipeline, ghostpad);
}

static void
add_video_payloder(GstElement *pipeline, GstPad *pad)
{
  GstElement  *videoqueue, *rtph264depay, *queue, *rtph264pay;
  GstPad *sinkpad, *srcpad, *ghostpad;
  gchar *padname;

  videoqueue = create_element("queue", NULL);
  rtph264depay = create_element("rtph264depay", NULL);
  queue = create_element("queue", NULL);
  rtph264pay = create_element("rtph264pay", "pay0");
  g_object_set(rtph264pay, "pt", 96, NULL);
  gst_bin_add_many(GST_BIN(pipeline), videoqueue, rtph264depay, queue, rtph264pay, NULL);
  gst_element_link_many(videoqueue, rtph264depay, queue, rtph264pay, NULL);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  /* Get the target sinkpad and link it */
  sinkpad = gst_element_get_static_pad(videoqueue, "sink");
  /* try and link the pads */
  gst_pad_link(pad, sinkpad);

  /* now expose the srcpad of the payloader as a ghostpad with the same name
   * as the decoder pad name. */
  srcpad = gst_element_get_static_pad (rtph264pay, "src");
  padname = gst_pad_get_name (pad);
  ghostpad = gst_ghost_pad_new (padname, srcpad);
  gst_object_unref (srcpad);
  g_free (padname);

  //gst_element_sync_state_with_parent(audioqueue);
  //gst_element_sync_state_with_parent(rtpmp4gdepay);
  //gst_element_set_state (audioqueue, GST_STATE_PAUSED);

  gst_pad_set_active (ghostpad, TRUE);
  gst_element_add_pad (pipeline, ghostpad);
}

//  rtspsrc location=rtsp://camroot:password@172.25.100.128/axis-media/media.amp?resolution=800x600&videocodec=h264&fps=25&audio=1 protocols=4 name=src  src. ! queue ! rtph264depay ! queue ! rtph264pay name=pay0 pt=96 src. ! queue ! rtpmp4gdepay ! aacparse ! avdec_aac ! audioconvert ! audioresample ! voaacenc ! rtpmp4gpay name=pay1 pt=97
