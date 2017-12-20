#ifndef __MEDIAMONITOR_H__
#define __MEDIAMONITOR_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "server.h"

void
monitor_media(ServerData *serverdata, GstRTSPMediaFactory *factory);

#endif /* __MEDIAMONITOR_H__ */