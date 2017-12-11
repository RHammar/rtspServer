/*
 * rtsp.h
 */
#ifndef __RTSP_H__
#define __RTSP_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

GstRTSPServer * rtsp_start(int argc, char *argv[]);
int rtsp_setup_stream(GstRTSPServer *server, gchar *pipeline, char *path);
int rtsp_setup_vod_pipeline(GstRTSPServer *server, char *path);

#endif /* __RTSP_H__ */