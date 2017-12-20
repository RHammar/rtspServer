#ifndef __SERVER__
#define __SERVER__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <libsoup/soup.h>

typedef struct ServerData {
  GMainLoop *loop;
  GstRTSPServer *server;
  SoupServer *soup;
  GList *mountPoints;
  GList *clients;
} ServerData;

#endif //__SERVER__
