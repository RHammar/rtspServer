#ifndef __SERVER__
#define __SERVER__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <libsoup/soup.h>
#include "config.h"

typedef struct ServerData {
  GMainLoop *loop;
  GstRTSPServer *server;
  SoupServer *soup;
  GList *mountPoints;
  RtspConfiguration *config;
} ServerData;

#endif //__SERVER__
