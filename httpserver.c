#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>

#include "httpserver.h"
#include "log.h"
#include "rtsp.h"
#include "pipelinebuilder.h"
#include "pipelinemonitor.h"
#include "server.h"

static void handle_new_pad(GstElement *decodebin, GstPad *pad,
                           GstElement *target);

static gint
compareMountPoints(gconstpointer a,
                   gconstpointer b)
{
  MountPoint *mounta = (MountPoint*) a;
  MountPoint *mountb = (MountPoint*) b;
  if (mounta->id == mountb->id){
    return 0;
  }
}

static void
live_callback(SoupServer *server,
              SoupMessage *msg,
              const char *path,
              GHashTable *query,
              SoupClientContext *context,
              gpointer user_data)
{
  const char *mime_type;
  GByteArray *body;
  JsonParser *parser;
  JsonReader *reader;
  GError *error = NULL;
  const gchar *uri = NULL;
  const gchar *proxy = NULL;
  const gchar *mountpath = NULL;
  ServerData *serverdata = (ServerData*)user_data;
  GstRTSPServer *rtspserver = serverdata->server;
  MountPoint *mountpoint;

  PDEBUG("live_callback");
  if (msg->method == SOUP_METHOD_PUT)
  {
    // Create new stream
    parser = json_parser_new();
    json_parser_load_from_data(parser, msg->request_body->data, -1, &error);
    reader = json_reader_new(json_parser_get_root(parser));
    if (json_reader_read_member(reader, "uri"))
    {
      uri = json_reader_get_string_value(reader);
      PDEBUG("uri: %s", uri);
    }
    json_reader_end_member (reader);
    if (json_reader_read_member(reader, "proxy"))
    {
      proxy = json_reader_get_string_value(reader);
      PDEBUG("proxy: %s", proxy);
    }
    json_reader_end_member (reader);
    if (json_reader_read_member(reader, "mountpath"))
    {
      mountpath = json_reader_get_string_value(reader);
      PDEBUG("mountpath: %s", mountpath);
    }
    json_reader_end_member (reader);

    /* Working live pipeline */
    // char *pipeline = "( rtspsrc location=rtsp://camroot:password@172.25.100.128/axis-media/media.amp?resolution=800x600&videocodec=h264&fps=25&audio=1 protocols=4 name=src  src. ! queue ! rtph264depay ! queue ! rtph264pay name=pay0 pt=96 src. ! decodebin ! audioconvert ! audioresample ! voaacenc ! rtpmp4gpay name=pay1 pt=97 )";

    /* No decodebin */
    char *pipeline = "( rtspsrc location=rtsp://camroot:password@172.25.100.128/axis-media/media.amp?resolution=800x600&videocodec=h264&fps=25&audio=1 protocols=4 name=src  src. ! queue ! rtph264depay ! queue ! rtph264pay name=pay0 pt=96 src. ! rtpmp4gdepay ! aacparse ! avdec_aac ! audioconvert ! audioresample ! voaacenc ! rtpmp4gpay name=pay1 pt=97 )";

    /* rtsp over http via o3c */
    // char *pipeline = "( rtspsrc proxy=172.25.118.246:3128 location=rtsph://00408CD61AC3/axis-media/media.amp?resolution=800x600&videocodec=h264&fps=25&audio=1 protocols=4 name=src  src. ! queue ! rtph264depay ! queue ! rtph264pay name=pay0 pt=96 src. ! decodebin ! audioconvert ! audioresample ! voaacenc ! rtpmp4gpay name=pay1 pt=97 )";

    // run decodebin
    // char *pipeline = "( rtspsrc protocols=4 location=rtsp://camroot:password@172.25.100.136/axis-media/media.amp?resolution=320x240&videocodec=h264&fps=7 ! queue ! rtph264depay ! avdec_h264 debug-mv=true ! videoconvert ! queue ! x264enc tune=2 ! queue ! rtph264pay name=pay0 pt=96 )";

    // run decodebin + rippletv
    // char *pipeline = "( rtspsrc protocols=4 location=rtsp://camroot:password@172.25.100.136/axis-media/media.amp?resolution=320x240&videocodec=h264&fps=7 ! queue ! rtph264depay ! decodebin ! videoconvert ! queue ! rippletv ! queue ! x264enc tune=4 ! queue ! rtph264pay name=pay0 pt=96 )";
    // rtsp_setup_stream(rtspserver, pipeline, "/live");
    mountpoint = rtsp_setup_proxy_stream(serverdata, uri, proxy, mountpath);
    if (!mountpoint) {
      goto error;
    }
    gchar *uri;
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object (builder);
    json_builder_set_member_name (builder, "id");
    json_builder_add_int_value (builder, mountpoint->id);
    json_builder_set_member_name (builder, "uri");
    gchar *address = gst_rtsp_server_get_address(serverdata->server);
    asprintf(&uri, "%s:%d%s", address, gst_rtsp_server_get_bound_port(serverdata->server), mountpoint->path);
    json_builder_add_string_value (builder, uri);
    json_builder_end_object (builder);
    JsonGenerator *gen = json_generator_new ();
    JsonNode *root = json_builder_get_root(builder);
    json_generator_set_root(gen, root);
    gchar *response = json_generator_to_data(gen, NULL);

    json_node_free(root);
    g_object_unref(gen);
    g_object_unref(builder);
    g_free(address);
    free(uri);

    soup_message_set_status(msg, SOUP_STATUS_OK);
    soup_message_set_response(msg, "application/json", SOUP_MEMORY_COPY,
                              response, strlen(response));
    g_object_unref(parser);
    g_object_unref(reader);
  }
  else if (msg->method == SOUP_METHOD_GET)
  {
    // Get status of mountpoint(s)
    soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
  else if (msg->method == SOUP_METHOD_DELETE)
  {
    // Remove mountpoint
    int id = 0;
    GList *found = NULL;
    MountPoint *mountpoint;
    parser = json_parser_new();
    json_parser_load_from_data(parser, msg->request_body->data, -1, &error);
    reader = json_reader_new(json_parser_get_root(parser));
    if (json_reader_read_member(reader, "id"))
    {
      id = json_reader_get_int_value(reader);
      PDEBUG("id: %d", id);
    }

    found = g_list_find_custom(serverdata->mountPoints, &id, compareMountPoints);
    if (found){
      mountpoint = (MountPoint*) found->data;
      GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points (serverdata->server);
      gst_rtsp_mount_points_remove_factory(mounts, mountpoint->path);
      g_object_unref(mounts);
      // remove from serverdata->mountpoints
      serverdata->mountPoints = g_list_remove(serverdata->mountPoints, mountpoint);
      //g_object_unref(mountpoint->factory);
      free(mountpoint);
    }
    soup_message_set_status(msg, SOUP_STATUS_OK);
    g_object_unref(parser);
    g_object_unref(reader);
  }
  else
  {
    soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }

 return;
error:
    soup_message_set_status(msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
}

static void
startfile_callback(SoupServer *server,
                   SoupMessage *msg,
                   const char *path,
                   GHashTable *query,
                   SoupClientContext *context,
                   gpointer user_data)
{

  GstElement *pipeline;
  /* Build the pipeline */
  // pipeline = createOggPipeline();
  // char *pipeline = "(filesrc location=/home/rickardh/Videos/big_buck_bunny.ogv "
  //                  " ! oggdemux name=d "
  //                  "d. ! queue ! rtptheorapay name=pay0 pt=96 "
  //                  "d. ! queue ! rtpvorbispay name=pay1 pt=97 )";
  rtsp_setup_vod_pipeline(user_data, "/vod");
  // monitor_pipeline(pipeline);
  soup_message_set_status(msg, SOUP_STATUS_OK);
  soup_message_set_response(msg, "text/plain", SOUP_MEMORY_COPY,
                            HTTP_OK, strlen(HTTP_OK));
}

static void
startfile_rate_callback(SoupServer *server,
                        SoupMessage *msg,
                        const char *path,
                        GHashTable *query,
                        SoupClientContext *context,
                        gpointer user_data)
{

  GstElement *pipeline;
  gdouble rate = 2.0;
  rtsp_setup_vod_rate_pipeline(rate, user_data, "/vod");
  soup_message_set_status(msg, SOUP_STATUS_OK);
  soup_message_set_response(msg, "text/plain", SOUP_MEMORY_COPY,
                            HTTP_OK, strlen(HTTP_OK));
}

static void
status_callback(SoupServer *server,
                SoupMessage *msg,
                const char *path,
                GHashTable *query,
                SoupClientContext *context,
                gpointer user_data)
{
  JsonBuilder *builder;
  JsonGenerator *generator;
  const gchar *body;
  ServerData *serverdata = (ServerData*)user_data;
  GstRTSPServer *rtsp_server = serverdata->server;

  if (msg->method != SOUP_METHOD_GET)
  {
    soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
    return;
  }
  builder = json_builder_new();
  json_builder_begin_object (builder);
  json_builder_set_member_name(builder, "clients");
  json_builder_add_int_value (builder, get_number_of_clients(rtsp_server));
  json_builder_set_member_name(builder, "mountpoints");
  json_builder_add_int_value (builder, g_list_length(serverdata->mountPoints));
  json_builder_end_object(builder);
  generator = json_generator_new ();
  json_generator_set_root (generator, json_builder_get_root (builder));
  body = json_generator_to_data (generator, NULL);
  soup_message_set_status(msg, SOUP_STATUS_OK);
  soup_message_set_response(msg, "application/json", SOUP_MEMORY_COPY,
                            body, strlen(body));
}

SoupServer *httpserver_start(ServerData *server)
{
  SoupServer *soupServer;
  GError *error = NULL;
  GSList *uris, *u;
  char *str;

  soupServer = soup_server_new(SOUP_SERVER_SERVER_HEADER, "rtsp-httpd ", NULL);
  soup_server_add_handler(soupServer, "/mountpoints", live_callback, server, NULL);
  soup_server_add_handler(soupServer, "/startfile", startfile_callback, server, NULL);
  soup_server_add_handler(soupServer, "/startfile/rate", startfile_rate_callback, server, NULL);
  soup_server_add_handler(soupServer, "/status", status_callback, server, NULL);
  if (!soup_server_listen_all(soupServer, 1500, 0, &error))
  {
    PDEBUG("Unable to bind to port");
    goto error;
  }
  uris = soup_server_get_uris(soupServer);
  for (u = uris; u; u = u->next)
  {
    str = soup_uri_to_string(u->data, FALSE);
    PDEBUG("Listening on %s", str);
    g_free(str);
    soup_uri_free(u->data);
  }
  g_slist_free(uris);

  PDEBUG("Waiting for requests...");
  return soupServer;

error:
  PDEBUG("Error starting server");
  return NULL;
}
