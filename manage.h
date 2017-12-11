/*
 * manage.h
 */
#ifndef __MANAGE_H__
#define __MANAGE_H__

#include <gio/gio.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <libsoup/soup.h>

/*
 * HTML response codes.
 */
#define MANAGE_HTML_200 "200 OK"
#define MANAGE_HTML_400 "400 Bad Request"
#define MANAGE_HTML_404 "404 Not Found"
#define MANAGE_HTML_500 "500 Internal Server Error"

/*
 * General.
 */
#define MANAGE_OK               "OK 200\n"
#define MANAGE_ERR              "ERROR 400\n"

SoupServer * manage_start(GstRTSPServer *server);

#endif /* __MANAGE_H__ */
