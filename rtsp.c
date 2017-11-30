#include <gst/gst.h>

#include "rtsp.h"
#include "log.h"
#include "rtsp-media-factory-custom.h"
#include "mediamonitor.h"

static void
client_connected(GstRTSPServer *,
                 GstRTSPClient *,
                 gpointer);
static void
client_closed(GstRTSPClient *,
              gpointer);

GstRTSPServer *rtsp_start(int argc, char *argv[])
{
  GstRTSPServer *server;

  /* Init GST */
  gst_init(&argc, &argv);

  /* create a server instance */
  server = gst_rtsp_server_new();
  /* start serving */
  PDEBUG("rtsp server started");

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach(server, NULL);

  g_signal_connect(server, "client-connected", (GCallback)client_connected,
                   NULL);

  return server;
}

int rtsp_setup_stream(GstRTSPServer *server, gchar *pipeline, char *path)
{
  GstRTSPMountPoints *mounts;
  // GstRTSPMediaFactoryCustom *factory;
  GstRTSPMediaFactory *factory;
  GType type;
  GstRTSPMedia *media;
  GstRTSPLowerTrans protocols;
  GObject *value;
  GstRTSPUrl *url;
  GstElement *pipeline_element;

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  g_object_get(server, "address", &value, NULL);
  PDEBUG("address: %s", (char *)value);
  mounts = gst_rtsp_server_get_mount_points(server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */

  // factory = gst_rtsp_media_factory_custom_new();
  // gst_rtsp_media_factory_custom_set_bin(factory, pipeline);

  factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_shared(factory, TRUE);
  gst_rtsp_media_factory_set_launch(factory, pipeline);
  monitor_media(factory);
  // pipeline_element = gst_rtsp_media_factory_create_element(factory, url);
  // media = gst_rtsp_media_factory_construct(factory, url);
  // pipeline_element = gst_rtsp_media_get_element(media);
  // monitor_pipeline(pipeline_element);
  // gst_rtsp_media_factory_set_protocols(factory, GST_RTSP_LOWER_TRANS_HTTP);
  // gst_rtsp_media_factory_custom_set_launch(factory, pipeline);

  // protocols = gst_rtsp_media_factory_get_protocols(factory);
  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory(mounts, path, GST_RTSP_MEDIA_FACTORY(factory));

  // PDEBUG("protocols: %d", protocols);
  /* don't need the ref to the mapper anymore */
  g_object_unref(mounts);

  // PDEBUG("pipeline used: %s", pipeline);
  PDEBUG("stream available at 127.0.0.1:8554%s", path);
}

static GstRTSPFilterResult
get_all_clients_filter(GstRTSPServer *server,
                       GstRTSPClient *client,
                       gpointer user_data)
{
  return GST_RTSP_FILTER_REF;
}

static guint
get_number_of_clients(GstRTSPServer *server)
{
  GList *client_list;
  guint list_length;

  client_list = gst_rtsp_server_client_filter(server, get_all_clients_filter, NULL);

  list_length = g_list_length(client_list);
  g_list_foreach(client_list,
                 (GFunc)g_object_unref,
                 NULL);
  g_list_free(client_list);
  return list_length;
}

static void
client_connected(GstRTSPServer *server,
                 GstRTSPClient *client,
                 gpointer user_data)
{
  guint clients;
  clients = get_number_of_clients(server);
  // +1 since current client is not in list yet
  g_print("client connected, total clients: %d\n", clients + 1);
  g_signal_connect(client, "closed", (GCallback)client_closed,
                   server);
}

static void
client_closed(GstRTSPClient *client,
              gpointer user_data)
{
  guint clients;
  GstRTSPServer *server = (GstRTSPServer*) user_data;

  clients = get_number_of_clients(server);
  // -1 since current client is still in the list
  g_print("client closed, total clients: %d\n", clients - 1);
}
