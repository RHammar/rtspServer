
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "mediamonitor.h"
#include "pipelinemonitor.h"
#include "log.h"
#include "rtsp-media-factory-custom.h"
#include "server.h"

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

void monitor_media(ServerData *serverdata, GstRTSPMediaFactory *factory)
{
  g_signal_connect(factory, "media-configure", (GCallback)media_configure,
                   serverdata);
  g_signal_connect(factory, "media-constructed", (GCallback)media_constructed,
                   serverdata);
};

/* called when a new media pipeline is constructed */
static void
media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media,
                gpointer user_data)
{
  guint nbrstreams = gst_rtsp_media_n_streams(media);
  PDEBUG("media_configure, %d streams", nbrstreams);
}

static void
media_constructed(GstRTSPMediaFactory *factory,
                  GstRTSPMedia *media,
                  gpointer user_data)
{
  GstElement * mediaElement;
  ServerData* serverdata = (ServerData*) user_data;
  PDEBUG("media constructed");
  media_list = g_list_append(media_list, media);
  gst_rtsp_media_set_shared(media, TRUE);
  g_signal_connect(media, "new-stream", (GCallback)new_stream,
                   NULL);
  g_signal_connect(media, "prepared", (GCallback)media_prepared,
                   NULL);
  g_signal_connect(media, "new-state", (GCallback)media_new_state,
                   NULL);
  mediaElement = gst_rtsp_media_get_element(media);
  // monitor_pipeline(serverdata,  GST_ELEMENT_PARENT(mediaElement));
}

static void
new_stream(GstRTSPMedia *media, GstRTSPStream *stream,
           gpointer user_data)
{
  PDEBUG("new stream");
}

static void
media_prepared(GstRTSPMedia *media, gpointer user_data)
{
  GstElement *topbin;

  PDEBUG("media prepared");
  // topbin = GST_ELEMENT_PARENT(gst_rtsp_media_get_element(media));
  // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(topbin), GST_DEBUG_GRAPH_SHOW_ALL, "media");
}

static void
media_new_state(GstRTSPMedia *media,
                gint state,
                gpointer user_data)
{
  GstElement *topbin;

  PDEBUG("new state: %s", gst_element_state_get_name((GstState)state));
  if (state == GST_STATE_PLAYING)
  {
    topbin = GST_ELEMENT_PARENT(gst_rtsp_media_get_element(media));
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(topbin), GST_DEBUG_GRAPH_SHOW_ALL, "media");
  }
}

