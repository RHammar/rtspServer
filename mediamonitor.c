
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "mediamonitor.h"

GList *media_list = NULL;

static void
media_configure(GstRTSPMediaFactory *,
                GstRTSPMedia *,
                gpointer);

static void
new_stream(GstRTSPMedia *,
           GstRTSPStream *,
           gpointer);

static void
media_prepared(GstRTSPMedia *, gpointer);

static void
media_constructed(GstRTSPMediaFactory *,
                  GstRTSPMedia *,
                  gpointer);

static void
media_new_state(GstRTSPMedia *,
                gint,
                gpointer);

void monitor_media(GstRTSPMediaFactory *factory)
{
  g_signal_connect(factory, "media-configure", (GCallback)media_configure,
                   NULL);
  g_signal_connect(factory, "media-constructed", (GCallback)media_constructed,
                   NULL);
};

/* called when a new media pipeline is constructed */
static void
media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media,
                gpointer user_data)
{
  guint nbrstreams = gst_rtsp_media_n_streams(media);
  g_print("media_configure, %d streams\n", nbrstreams);
}

static void
media_constructed(GstRTSPMediaFactory *factory,
                  GstRTSPMedia *media,
                  gpointer user_data)
{
  g_print("media constructed\n");
  media_list = g_list_append(media_list, media);
  gst_rtsp_media_set_shared(media, TRUE);
  g_signal_connect(media, "new-stream", (GCallback)new_stream,
                   NULL);
  g_signal_connect(media, "prepared", (GCallback)media_prepared,
                   NULL);
  g_signal_connect(media, "new-state", (GCallback)media_new_state,
                   NULL);
  gst_rtsp_media_get_element(media);
}

static void
new_stream(GstRTSPMedia *media, GstRTSPStream *stream,
           gpointer user_data)
{
  g_print("new stream\n");
}

static void
media_prepared(GstRTSPMedia *media, gpointer user_data)
{
  GstElement *topbin;

  g_print("media prepared\n");
  // topbin = GST_ELEMENT_PARENT(gst_rtsp_media_get_element(media));
  // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(topbin), GST_DEBUG_GRAPH_SHOW_ALL, "media");
}

static void
media_new_state(GstRTSPMedia *media,
                gint state,
                gpointer user_data)
{
  GstElement *topbin;

  g_print("new state: %s\n", gst_element_state_get_name((GstState)state));
  if (state == GST_STATE_PLAYING)
  {
    topbin = GST_ELEMENT_PARENT(gst_rtsp_media_get_element(media));
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(topbin), GST_DEBUG_GRAPH_SHOW_ALL, "media");
  }
}