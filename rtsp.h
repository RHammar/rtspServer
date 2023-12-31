/*
 * rtsp.h
 */
#ifndef __RTSP_H__
#define __RTSP_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "server.h"
#include "rtsp-media-factory-rtsp-proxy.h"
#include "config.h"

typedef struct MountPoint {
  guint32 id;
  gchar *path;
  GstRTSPMediaFactoryRtspProxy* factory;
} MountPoint;

GstRTSPServer * rtsp_start(ServerData *server, RtspConfiguration *config, int argc, char *argv[]);
int rtsp_setup_stream(GstRTSPServer *server, gchar *pipeline, char *path);
int rtsp_setup_vod_pipeline(ServerData *server, char *path);
int rtsp_setup_vod_rate_pipeline(gdouble rate, GstRTSPServer *server, char *path);
MountPoint *rtsp_setup_proxy_stream(ServerData *serverdata, const gchar *uri, const gchar *proxy, const gchar *path);
guint get_number_of_clients(GstRTSPServer *server);

#endif /* __RTSP_H__ */
