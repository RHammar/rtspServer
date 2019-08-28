/*
 * httpserver.h
 */
#ifndef __CALLBACKCLIENT__
#define __CALLBACKCLIENT__

// #include <gio/gio.h>
// #include <gst/rtsp-server/rtsp-server.h>
// #include <libsoup/soup.h>
// #include "server.h"
#include "config.h"
#include "rtsp_clients.h"

/*
 * General.
 */
#define HTTP_OK               "OK 200\n"
#define HTTP_ERR              "ERROR 400\n"

int callback_send_stream_started(ServerData *serverdata, RTSPClient *client);
int callback_send_stream_ended(ServerData *serverdata, RTSPClient *client);

#endif /* __CALLBACKCLIENT__ */
