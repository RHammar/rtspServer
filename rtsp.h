/*
 * rtsp.h
 */
#ifndef __RTSP_H__
#define __RTSP_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "server.h"

typedef struct MountPoint {
  guint32 id;
  gchar *path;
  GstRTSPMediaFactory* factory;
} MountPoint;

typedef struct RTSPClient {
  guint32 id;
  GstRTSPClient *client;
  MountPoint *mountpoint;
} RTSPClient;

GstRTSPServer * rtsp_start(ServerData *server, int argc, char *argv[]);
int rtsp_setup_stream(GstRTSPServer *server, gchar *pipeline, char *path);
int rtsp_setup_vod_pipeline(ServerData *server, char *path);
int rtsp_setup_vod_rate_pipeline(gdouble rate, GstRTSPServer *server, char *path);
MountPoint *rtsp_setup_proxy_stream(ServerData *serverdata, const gchar *uri, const gchar *proxy, const gchar *path);
guint get_number_of_clients(GstRTSPServer *server);

#endif /* __RTSP_H__ */
