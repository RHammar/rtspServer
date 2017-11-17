/*
 * rtsp.h
 */
#ifndef __RTSP_H__
#define __RTSP_H__

#include <gst/rtsp-server/rtsp-server.h>

GstRTSPServer * rtsp_start(int argc, char *argv[]);
int rtsp_setup_stream(GstRTSPServer *server, char *pipeline, char *path);

#endif /* __RTSP_H__ */