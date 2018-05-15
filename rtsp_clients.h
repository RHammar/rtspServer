#ifndef __RTSP_CLIENTS_H__
#define __RTSP_CLIENTS_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "rtsp.h"

typedef struct RTSPClient {
  guint32 id;
  GstRTSPClient *client;
  MountPoint *mountpoint;
} RTSPClient;

RTSPClient *
add_client(GstRTSPClient *client);

gint
close_client(guint32 id);

gint
remove_client(GstRTSPClient *client);

GList *
get_clients();

RTSPClient *
get_client(GstRTSPClient *client);

#endif // __RTSP_CLIENTS_H__