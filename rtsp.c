#include <gst/gst.h>

#include "server.h"
#include "rtsp.h"
#include "log.h"
#include "rtsp-media-factory-custom.h"
#include "rtsp-media-factory-rtsp-proxy.h"
#include "mediamonitor.h"

typedef struct _CustomData
{
  GstElement *pipeline;
  GstElement *video_sink;
  GMainLoop *loop;

  gboolean playing; /* Playing or Paused */
  gdouble rate;     /* Current playback rate (can be negative) */
} CustomData;

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

int rtsp_setup_proxy_stream(ServerData *serverdata, const gchar *uri, const gchar *proxy, const gchar *path)
{
  GstRTSPMountPoints *mounts;
  GstRTSPServer *rtspserver = serverdata->server;
  MountPoint * mountpoint;
  GstRTSPMediaFactoryRtspProxy *factory;

  factory = gst_rtsp_media_factory_rtsp_proxy_new();
  gst_rtsp_media_factory_rtsp_proxy_configure(factory, uri, proxy);
  gst_rtsp_media_factory_set_shared(GST_RTSP_MEDIA_FACTORY(factory), TRUE);

  mountpoint = (MountPoint *)g_malloc(sizeof(MountPoint));
  mountpoint->id = 1;
  mountpoint->path = path;
  mountpoint->factory = GST_RTSP_MEDIA_FACTORY(factory);
  serverdata->mountPoints = g_list_append(serverdata->mountPoints, mountpoint);
  mounts = gst_rtsp_server_get_mount_points(serverdata->server);
  gst_rtsp_mount_points_add_factory(mounts, path, GST_RTSP_MEDIA_FACTORY(factory));
  monitor_media(serverdata, GST_RTSP_MEDIA_FACTORY(factory));
  g_object_unref(mounts);
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
  monitor_media(NULL, factory);
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

int rtsp_setup_vod_pipeline(ServerData *serverdata, char *path)
{
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactoryCustom *factory;
  GObject *value;
  GstRTSPServer *rtspServer = serverdata->server;

  g_object_get(rtspServer, "address", &value, NULL);
  PDEBUG("address: %s", (char *)value);
  mounts = gst_rtsp_server_get_mount_points(rtspServer);

  factory = gst_rtsp_media_factory_custom_new();
  // gst_rtsp_media_factory_custom_set_bin(factory, pipeline);
  monitor_media(serverdata, GST_RTSP_MEDIA_FACTORY(factory));

  // protocols = gst_rtsp_media_factory_get_protocols(factory);
  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory(mounts, path, GST_RTSP_MEDIA_FACTORY(factory));
  g_object_unref(mounts);
  PDEBUG("stream available at 127.0.0.1:8554%s", path);
}

void send_seek_event(CustomData *data)
{
  PDEBUG("send seek event, rate: %f", data->rate);
  gint64 position = 0;
  GstFormat format = GST_FORMAT_TIME;
  GstEvent *seek_event;

  // /* Obtain the current position, needed for the seek event */
  // // if (!gst_element_query_position (data->pipeline, format, &position)) {
  // //   PDEBUG ("Unable to retrieve current position.");
  // //   return;
  // // }

  /* Create the seek event */
  if (data->rate > 0)
  {
    seek_event = gst_event_new_seek(data->rate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
                                    GST_SEEK_TYPE_SET, position, GST_SEEK_TYPE_NONE, 0);
  }
  else
  {
    seek_event = gst_event_new_seek(data->rate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
                                    GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, position);
  }

  // if (data->video_sink == NULL) {
  //   /* If we have not done so, obtain the sink through which we will send the seek events */
  //   g_object_get (data->pipeline, "video-sink", &data->video_sink, NULL);
  // }

  /* Send the event */
  gst_element_send_event(data->pipeline, seek_event);

  PDEBUG("Current rate: %g", data->rate);
}

static void
media_constructed(GstRTSPMediaFactory *factory,
                  GstRTSPMedia *media,
                  gpointer user_data)
{
  GstElement *topbin;
  CustomData *data = (CustomData *)user_data;
  topbin = GST_ELEMENT_PARENT(gst_rtsp_media_get_element(media));
  data->pipeline = topbin;
  send_seek_event(data);
}

void rate_setter(GstRTSPMediaFactory *factory, CustomData *data)
{

  g_signal_connect(factory, "media-constructed", (GCallback)media_constructed,
                   data);
}

int rtsp_setup_vod_rate_pipeline(gdouble rate, GstRTSPServer *server, char *path)
{
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactoryCustom *factory;
  GObject *value;
  CustomData data;

  g_object_get(server, "address", &value, NULL);
  PDEBUG("address: %s", (char *)value);
  mounts = gst_rtsp_server_get_mount_points(server);

  factory = gst_rtsp_media_factory_custom_new();
  data.rate = rate;
  rate_setter(GST_RTSP_MEDIA_FACTORY(factory), &data);
  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory(mounts, path, GST_RTSP_MEDIA_FACTORY(factory));
  g_object_unref(mounts);
  PDEBUG("stream available at 127.0.0.1:8554%s", path);
}

static GstRTSPFilterResult
get_all_clients_filter(GstRTSPServer *server,
                       GstRTSPClient *client,
                       gpointer user_data)
{
  return GST_RTSP_FILTER_REF;
}

guint get_number_of_clients(GstRTSPServer *server)
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
  GstRTSPMountPoints *mountpoints;
  clients = get_number_of_clients(server);
  // +1 since current client is not in list yet
  PDEBUG("client connected, total clients: %d", clients + 1);
  g_signal_connect(client, "closed", (GCallback)client_closed,
                   server);
  mountpoints = gst_rtsp_client_get_mount_points(client);
}

static void
client_closed(GstRTSPClient *client,
              gpointer user_data)
{
  guint clients;
  GstRTSPServer *server = (GstRTSPServer *)user_data;

  clients = get_number_of_clients(server);
  // -1 since current client is still in the list
  PDEBUG("client closed, total clients: %d", clients - 1);
}
