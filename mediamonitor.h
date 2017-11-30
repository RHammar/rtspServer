#ifndef __MEDIAMONITOR_H__
#define __MEDIAMONITOR_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

void
monitor_media(GstRTSPMediaFactory *factory);

#endif /* __MEDIAMONITOR_H__ */