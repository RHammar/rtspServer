
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "mediamonitor.h"

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
  g_signal_connect(media, "new-stream", (GCallback)new_stream,
                   NULL);
  g_signal_connect(media, "prepared", (GCallback)media_prepared,
                   NULL);
}

static void
media_constructed(GstRTSPMediaFactory *factory,
                  GstRTSPMedia *media,
                  gpointer user_data)
{
  g_print("media constructed\n");
  gst_rtsp_media_set_shared(media, TRUE);
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
  g_print("media prepared\n");
}