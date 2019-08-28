#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <gst/gst.h>
#include "log.h"

#define CALLBACK_URL     "Callback URL"
#define LOGLEVEL         "Loglevel"
#define HTTP_LISTEN_IP   "HTTP listen IP"
#define HTTP_LISTEN_PORT "HTTP listen port"
#define RTSP_LISTEN_IP   "RTSP listen IP"
#define RTSP_LISTEN_PORT "RTSP listen port"
#define LOGFILE          "Logfile"

typedef struct RtspConfiguration {
  gchar *callbackUrl;
  Loglevel loglevel;
  gchar *httpListenIp;
  gchar *httpListenPort;
  gchar *rtspListenIp;
  gchar *rtspListenPort;
  gchar *logfile;
} RtspConfiguration;

gint getConfig(RtspConfiguration *config, gchar *filename);


#endif //__CONFIG_H__
