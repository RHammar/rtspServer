/*
 * httpserver.h
 */
#ifndef __HTTPSERVER__
#define __HTTPSERVER__

#include <gio/gio.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <libsoup/soup.h>
#include "server.h"

/*
 * HTML response codes.
 */
#define HTTP_HTML_200 "200 OK"
#define HTTP_HTML_400 "400 Bad Request"
#define HTTP_HTML_404 "404 Not Found"
#define HTTP_HTML_500 "500 Internal Server Error"

/*
 * General.
 */
#define HTTP_OK               "OK 200\n"
#define HTTP_ERR              "ERROR 400\n"

SoupServer * httpserver_start(ServerData *server);

#endif /* __HTTPSERVER__ */
