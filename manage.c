
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <libsoup/soup.h>


#include "manage.h"
#include "log.h"

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

static gboolean
on_timeout(Client *client)
{
  PDEBUG("Client %s Timeout\n", client->name);

  return FALSE;
}

/*
 * Sends response and close connection to client.
 *
 * Returns number of bytes written, or -1 upon error.
 */
// static void manage_write_response(struct evbuffer *outbuf, const char *code, struct evbuffer *body)
// {
//     size_t len = evbuffer_get_length(body);
//     evbuffer_add_printf(outbuf,
//                         "HTTP/1.1 %s\r\n"
//                         "Content-Length: %zd\r\n"
//                         "Content-Type: text/plain\r\n"
//                         "Connection: Keep-Alive\r\n"
//                         "\r\n",
//                         code,
//                         len);
//     evbuffer_remove_buffer(body, outbuf, len);
//     evbuffer_free(body);
// }

// static void manage_send_response(struct evbuffer *outbuf, const char *code, const char *msg)
// {
//     struct evbuffer *body = evbuffer_new();
//     evbuffer_add_printf(body, "%s", msg);
//     manage_write_response(outbuf, code, body);
// }

// static void manage_read_cb(struct bufferevent *bev, void *server)
// {
//   struct evbuffer *inbuf = bufferevent_get_input(bev);
//   struct evbuffer *outbuf = bufferevent_get_output(bev);
//   struct evbuffer_ptr findpos;                     /* Position where end string is found. */
//   char cinput[BUFLEN];                             /* Request input char buffer. */
//   int inputlen = 0;                                /* Request length. */
//   const char end_str[] = {'\r', '\n', '\r', '\n'}; /* End string. */
//   char *uri;                                       /* The URI of the request. */
//   char *tmp;

//   /* Do we have the entire request? */
//   findpos = evbuffer_search(inbuf, end_str, sizeof(end_str), NULL);
//   if (-1 == findpos.pos)
//   {
//     return;
//   }

//   /* Is the input buffer big enough to hold the request? */
//   if (findpos.pos + sizeof(end_str) >= BUFLEN)
//   {
//     PDEBUG("Indata (%zd) exceeds buffer size (%zd) - Request not handled!\n",
//            findpos.pos + sizeof(end_str), sizeof(cinput));
//     return;
//   }

//   /* Retrieve request data. */
//   inputlen = evbuffer_remove(inbuf, (void *)cinput, findpos.pos + sizeof(end_str));
//   cinput[inputlen] = '\0';

//   /* Extract the command. */
//   if (strncmp(cinput, "GET ", 4) == 0)
//   {
//     uri = (cinput + 4);
//   }
//   else if (strncmp(cinput, "POST ", 5) == 0)
//   {
//     uri = (cinput + 5);
//   }
//   else
//   {
//     PDEBUG("Request not recognized:\n%s\n", cinput);
//     manage_send_response(outbuf, MANAGE_HTML_400, MANAGE_ERR "Malformed request.\n");
//     goto end;
//   }

//   tmp = strchr(uri, ' ');
//   if (tmp)
//   {
//     *tmp = '\0';
//   }
//   else
//   {
//     PDEBUG("Request not recognized:\n%s\n", cinput);
//     manage_send_response(outbuf, MANAGE_HTML_400, MANAGE_ERR "Malformed request.\n");
//     goto end;
//   }

//   /* GET /start/... HTTP */
//   if (strncmp(uri, START_URI, strlen(START_URI)) == 0)
//   {
//     char *cmd_opt = uri + strlen(START_URI);
//     // manage_record_parse(cmd_opt, outbuf);
//     char *pipeline = "( rtspsrc protocols=4 location=rtsp://camroot:password@172.25.100.128/axis-media/media.amp?resolution=320x240&videocodec=h264&fps=3 ! queue ! rtph264depay ! queue ! rtph264pay name=pay0 pt=96 )";
//     rtsp_setup_stream(server, pipeline);
//   }

//   /* GET unknown HTTP */
//   else
//   {
//     manage_send_response(outbuf, MANAGE_HTML_404, MANAGE_ERR "Not found\n.");
//   }

// end:
//   /* There shouldn't be more requests in inbuf, but if there are,
//      * handle them as well. */
//   manage_read_cb(bev, server);

//   return;
// }

// static void manage_write_cb(struct bufferevent *bev, void *server)
// {
// }

// static void manage_event_cb(struct bufferevent *bev, short events, void *server)
// {
//   bufferevent_free(bev);
// }

// static void manage_accept_con_cb(struct evconnlistener *listener,
//                                  evutil_socket_t fd, struct sockaddr *address, int socklen, void *server)
// {
//   /* We got a new connection! Set up a bufferevent for it. */
//   struct sockaddr_in *sin = (struct sockaddr_in *)address;
//   struct event_base *base = evconnlistener_get_base(listener);
//   struct bufferevent *bufev = bufferevent_socket_new(base, fd,
//                                                      BEV_OPT_CLOSE_ON_FREE);

//   bufferevent_setcb(bufev, manage_read_cb, manage_write_cb, manage_event_cb, NULL);
//   bufferevent_enable(bufev, EV_READ);

//   PDEBUG("New management connection: %p from %d:%d\n", listener, inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));
//   return;
// }

static void
write_bytes (Client * client, const gchar * data, guint len)
{
  gssize w;
  GError *err = NULL;

  /* TODO: We assume this never blocks */
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

void manage_read_cb(GObject *source_object,
                    GAsyncResult *res,
                    gpointer user_data)
{
  GInputStream *istream = G_INPUT_STREAM(source_object);
  GError *error = NULL;
  struct Client *client = user_data;
  int count;
  gchar *findpos;                                   /* Position where end string is found. */
  const gchar end_str[] = {'\r', '\n', '\r', '\n'}; /* End string. */
  char *uri;                                        /* The URI of the request. */
  char *tmp;

  count = g_input_stream_read_finish(istream,
                                     res,
                                     &error);
  if (count == -1)
  {
    g_error("Error when receiving message");
    if (error != NULL)
    {
      g_error("%s", error->message);
      g_clear_error(&error);
    }
  }
  g_message("Message was: \"%s\"\n", client->message);
  /* Do we have the entire request? */
  findpos = g_strrstr_len(client->message, sizeof(client->message), end_str);
  if (NULL == findpos)
  {
    return;
  }

  /* Extract the command. */
  if (strncmp(client->message, "GET ", 4) == 0)
  {
    uri = (client->message + 4);
  }
  else if (strncmp(client->message, "POST ", 5) == 0)
  {
    uri = (client->message + 5);
  }
  else
  {
    PDEBUG("Request not recognized:\n%s\n", client->message);
    // manage_send_response(outbuf, MANAGE_HTML_400, MANAGE_ERR "Malformed request.\n");
    goto end;
  }

  tmp = strchr(uri, ' ');
  if (tmp)
  {
    *tmp = '\0';
  }
  else
  {
    PDEBUG("Request not recognized:\n%s\n", client->message);
    // manage_send_response(outbuf, MANAGE_HTML_400, MANAGE_ERR "Malformed request.\n");
    goto end;
  }

  /* GET /start/... HTTP */
  PDEBUG("manage_read_cb %s\n", uri);
  if (strncmp(uri, START_URI, strlen(START_URI)) == 0)
  {
    char *cmd_opt = uri + strlen(START_URI);
    // manage_record_parse(cmd_opt, outbuf);
    char *pipeline = "( rtspsrc protocols=4 location=rtsp://camroot:password@172.25.100.128/axis-media/media.amp?resolution=320x240&videocodec=h264&fps=3 ! queue ! rtph264depay ! queue ! rtph264pay name=pay0 pt=96 )";
    rtsp_setup_stream(client->server, pipeline);
    manage_send_response(client, MANAGE_HTML_200, MANAGE_OK);
  }

  /* GET unknown HTTP */
  else
  {
    // manage_send_response(outbuf, MANAGE_HTML_404, MANAGE_ERR "Not found\n.");
  }

end:
  /* There shouldn't be more requests in inbuf, but if there are,
     * handle them as well. */
  // manage_read_cb(bev, server);

  g_source_destroy(client->tosource);
  g_source_unref(client->tosource);
  client->tosource = NULL;
  g_object_unref(G_SOCKET_CONNECTION(client->connection));
  g_free(client);
}

/* this function will get called everytime a client attempts to connect */
gboolean
manage_accept_con_cb(GSocketService *service,
                     GSocketConnection *connection,
                     GObject *source_object,
                     gpointer user_data)
{
  GSocketAddress *addr;
  GInetAddress *iaddr;
  gchar *ip;
  guint16 port;

  addr = g_socket_connection_get_remote_address(connection, NULL);
  iaddr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(addr));
  port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(addr));
  ip = g_inet_address_to_string(iaddr);

  GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
  // struct ConnData *data = g_new(struct ConnData, 1);
  struct Client *client = g_new(Client, 1);
  client->server = user_data;
  client->connection = g_object_ref(connection);

  client->name = g_strdup_printf("%s:%u", ip, port);
  g_free(ip);
  g_object_unref(addr);

  PDEBUG("New connection %s\n", client->name);

  client->socket = g_socket_connection_get_socket(connection);
  client->istream =
      g_io_stream_get_input_stream(G_IO_STREAM(client->connection));
  client->ostream =
      g_io_stream_get_output_stream(G_IO_STREAM(client->connection));

  client->tosource = g_timeout_source_new_seconds(5);
  g_source_set_callback(client->tosource, (GSourceFunc)on_timeout, client,
                        NULL);
  g_source_attach(client->tosource, NULL);
  g_input_stream_read_async(istream,
                            client->message,
                            sizeof(client->message),
                            G_PRIORITY_DEFAULT,
                            NULL,
                            manage_read_cb,
                            client);

  return FALSE;
}

GSocketService *manage_start(GstRTSPServer *server)
{
  GError *error = NULL;

  /* create the new socketservice */
  GSocketService *service = g_socket_service_new();

  /* connect to the port */
  g_socket_listener_add_inet_port((GSocketListener *)service,
                                  1500, /* your port goes here */
                                  NULL,
                                  &error);

  /* don't forget to check for errors */
  if (error != NULL)
  {
    syslog(LOG_CRIT, "Couldn't start listening on management port.");
    exit(1);
  }

  /* listen to the 'incoming' signal */
  g_signal_connect(service,
                   "incoming",
                   G_CALLBACK(manage_accept_con_cb),
                   server);

  /* start the socket service */
  g_socket_service_start(service);
  PDEBUG("Waiting for client!");

  //   static struct evconnlistener *managecon; /* Our management listener. */
  //   struct sockaddr addr;
  //   int addrlen = sizeof(addr);

  //   evutil_parse_sockaddr_port(straddr, &addr, &addrlen);
  //   PDEBUG("adress: %d", addrlen);

  // /* Start listening on our incoming management socket. */
  //   managecon = evconnlistener_new_bind(evbase, manage_accept_con_cb,
  //                                       server, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, &addr, sizeof(addr));
  //   if (!managecon)
  //   {
  //     syslog(LOG_CRIT, "Couldn't start listening on management port.");
  //     exit(1);
  //   }

  return service;
}
