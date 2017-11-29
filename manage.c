
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <libsoup/soup.h>


#include "manage.h"
#include "log.h"
#include "rtsp.h"
#include "pipelinebuilder.h"
#include "pipelinemonitor.h"

/*
 * Length of buffers for incoming RTSP commands, the management
 * protocol and viewer side RTCP packets.
 */
#define BLOCK_SIZE 4096
#define START_URI "/start/"

#define MANAGE_OK               "OK 200\n"
#define MANAGE_ERR              "ERROR 400\n"

#define MANAGE_HTML_200 "200 OK"
#define MANAGE_HTML_400 "400 Bad Request"

typedef struct Client
{
  gchar *name;
  GSocket *socket;
  GInputStream *istream;
  GOutputStream *ostream;
  GSource *tosource;
  GSocketConnection *connection;
  gchar message[BLOCK_SIZE];
  GstRTSPServer *server;
} Client;

static void handle_new_pad (GstElement * decodebin, GstPad * pad,
    GstElement * target);

static gboolean
on_timeout(Client *client)
{
  PDEBUG("Client %s Timeout\n", client->name);

  return FALSE;
}

static void write_bytes (Client * client, const gchar * data, guint len)
{
  gssize w;
  GError *err = NULL;

  do {
    w = g_output_stream_write (client->ostream, data, len, NULL, &err);
    if (w > 0) {
      len -= w;
      data += w;
    }
  } while (w > 0 && len > 0);

  if (w <= 0) {
    if (err) {
      g_print ("Write error %s\n", err->message);
      g_clear_error (&err);
    }
  }
}

static void manage_send_response(Client *client,
                                 const char *code,
                                 const char *msg)
{

   gchar *response = g_strdup_printf ("HTTP/1.0 200 OK\r\n" "\r\n");
  write_bytes (client, response, strlen (response));
}

static void
start_callback(SoupServer *server,
               SoupMessage *msg,
		           const char *path,
               GHashTable *query,
               SoupClientContext *context,
               gpointer user_data)
{
  const char *mime_type;
  GByteArray *body;
  PDEBUG("start_callback");
  if (msg->method != SOUP_METHOD_GET && msg->method != SOUP_METHOD_POST) {
        soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
        return;
    }

  //Working live pipeline
  char *pipeline = "( rtspsrc location=rtsp://camroot:password@172.25.100.136/axis-media/media.amp?resolution=1920x1080&videocodec=h264&fps=25 ! queue ! rtph264depay ! queue ! rtph264pay name=pay0 pt=96 )";

  // run decodebin
  // char *pipeline = "( rtspsrc protocols=4 location=rtsp://camroot:password@172.25.100.136/axis-media/media.amp?resolution=320x240&videocodec=h264&fps=7 ! queue ! rtph264depay ! avdec_h264 debug-mv=true ! videoconvert ! queue ! x264enc tune=2 ! queue ! rtph264pay name=pay0 pt=96 )";

  // run decodebin + rippletv
  // char *pipeline = "( rtspsrc protocols=4 location=rtsp://camroot:password@172.25.100.136/axis-media/media.amp?resolution=320x240&videocodec=h264&fps=7 ! queue ! rtph264depay ! decodebin ! videoconvert ! queue ! rippletv ! queue ! x264enc tune=4 ! queue ! rtph264pay name=pay0 pt=96 )";
  // rtsp_setup_stream(user_data, pipeline, "/live");
  soup_message_set_status (msg, SOUP_STATUS_OK);
  soup_message_set_response (msg, "text/plain", SOUP_MEMORY_COPY,
                             MANAGE_OK, strlen(MANAGE_OK));
}

static void
startfile_callback(SoupServer *server,
               SoupMessage *msg,
		           const char *path,
               GHashTable *query,
               SoupClientContext *context,
               gpointer user_data)
{


  // GstElement *pipeline;
  /* Build the pipeline */
  // pipeline = createOggPipeline();
  // videoqueue = create_element ("queue", NULL);
  // theorapay = create_element ("rtptheorapay", "pay0");
  // g_object_set (theorapay, "pt", "96", NULL);
  // audioqueue = create_element ("queue", NULL);
  // rtpvorbispay = create_element ("rtpvorbispay", "pay1");
  // g_object_set (rtpvorbispay, "pt", "97", NULL);

  
  // gst_element_link_many (oggdemux, videoqueue, theorapay, NULL);
  // gst_element_link_many (oggdemux, audioqueue, rtpvorbispay, NULL);
  char *pipeline = "(filesrc location=/home/rickardh/Videos/big_buck_bunny.ogv "
      " ! oggdemux name=d "
      "d. ! queue ! rtptheorapay name=pay0 pt=96 "
      "d. ! queue ! rtpvorbispay name=pay1 pt=97 )";
  rtsp_setup_stream(user_data, pipeline, "/vod");
  // monitor_pipeline(pipeline);
  soup_message_set_status (msg, SOUP_STATUS_OK);
  soup_message_set_response (msg, "text/plain", SOUP_MEMORY_COPY,
                             MANAGE_OK, strlen(MANAGE_OK));
}


GSocketService *manage_start(GstRTSPServer *rtspServer)
{
  SoupServer *soupServer;
  GError *error = NULL;
  GSList *uris, *u;
  char *str;

  soupServer = soup_server_new (SOUP_SERVER_SERVER_HEADER, "rtsp-httpd ", NULL);
  soup_server_add_handler (soupServer, "/start", start_callback, rtspServer, NULL);
  soup_server_add_handler (soupServer, "/startfile", startfile_callback, rtspServer, NULL);
  soup_server_listen_all(soupServer, 1500, 0, &error);
  uris = soup_server_get_uris (soupServer);
  for (u = uris; u; u = u->next) {
		str = soup_uri_to_string (u->data, FALSE);
		g_print ("Listening on %s\n", str);
		g_free (str);
		soup_uri_free (u->data);
	}
	g_slist_free (uris);

	g_print ("\nWaiting for requests...\n");
}
