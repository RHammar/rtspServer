#ifndef __RTSP_PROXY_PIPELINE__
#define __RTSP_PROXY_PIPELINE__

#include <gst/gst.h>

GstElement *
createRtspProxyPipeline(const gchar *uri, const gchar *proxy);

#endif //__RTSP_PROXY_PIPELINE__