#include <string.h>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include "callbackclient.h"

int
callback_send_stream_started(ServerData *serverdata, RTSPClient *client)
{
  SoupSession *session;
  SoupMessage *msg;
  RtspConfiguration *config;
  guint status;
  const char *content;

  JsonBuilder *builder = json_builder_new();
  json_builder_begin_object(builder);
  json_builder_set_member_name(builder, "event");
  json_builder_add_string_value(builder, "streamStarted");
  json_builder_set_member_name(builder, "clientID");
  json_builder_add_int_value(builder, client->id);
  json_builder_set_member_name(builder, "mountPointID");
  json_builder_add_int_value(builder, client->mountpoint->id);
  json_builder_end_object(builder);

  // Convert request into a string
  JsonGenerator *generator = json_generator_new();
  json_generator_set_root(generator, json_builder_get_root(builder));
  gsize length;
  gchar *data = json_generator_to_data(generator, &length);

  g_object_unref(builder);
  g_object_unref(generator);

  session = soup_session_new();
  msg = soup_message_new("POST", serverdata->config->callbackUrl);
  soup_message_set_request(msg, "application/json", SOUP_MEMORY_TAKE, data, length);
  PDEBUG("sending request %s to %s", data, serverdata->config->callbackUrl);
  status = soup_session_send_message(session, msg);
  PDEBUG("status: %d", status);
  g_object_unref(session);
  g_object_unref(msg);
  return status;
}

int
callback_send_stream_ended(ServerData *serverdata, RTSPClient *client)
{
  SoupSession *session;
  SoupMessage *msg;
  RtspConfiguration *config;
  guint status;
  const char *content;

  JsonBuilder *builder = json_builder_new();
  json_builder_begin_object(builder);
  json_builder_set_member_name(builder, "event");
  json_builder_add_string_value(builder, "streamEnded");
  json_builder_set_member_name(builder, "clientID");
  json_builder_add_int_value(builder, client->id);
  json_builder_set_member_name(builder, "mountPointID");
  json_builder_add_int_value(builder, client->mountpoint->id);
  json_builder_end_object(builder);

  // Convert request into a string
  JsonGenerator *generator = json_generator_new();
  json_generator_set_root(generator, json_builder_get_root(builder));
  gsize length;
  gchar *data = json_generator_to_data(generator, &length);

  g_object_unref(builder);
  g_object_unref(generator);

  session = soup_session_new();
  msg = soup_message_new("POST", serverdata->config->callbackUrl);
  soup_message_set_request(msg, "application/json", SOUP_MEMORY_TAKE, data, length);
  PDEBUG("sending request %s to %s", data, serverdata->config->callbackUrl);
  status = soup_session_send_message(session, msg);
  PDEBUG("status: %d", status);
  g_object_unref(session);
  g_object_unref(msg);
  return status;
}
