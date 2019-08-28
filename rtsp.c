#include <gst/gst.h>
#include <string.h>
#include <stdlib.h>

#include "server.h"
#include "rtsp.h"
#include "log.h"
#include "rtsp-media-factory-custom.h"
#include "rtsp-media-factory-rtsp-proxy.h"
#include "mediamonitor.h"
#include "rtsp_clients.h"
#include "callbackclient.h"

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

static void
client_play(GstRTSPClient *,
            GstRTSPContext *,
            gpointer);

static gint
findMountPointFactory(gconstpointer a,
                      gconstpointer b)
{
  MountPoint *mounta = (MountPoint*) a;
  GstRTSPMediaFactoryRtspProxy *factory = (GstRTSPMediaFactoryRtspProxy*) b;
  PDEBUG("findmountpointfactory a: %p, b: %p", mounta->factory, factory);
  if (mounta->factory == factory){
    return 0;
  }
}

GstRTSPServer *rtsp_start(ServerData *serverdata, RtspConfiguration *config, int argc, char *argv[])
{
  GstRTSPServer *server;

  /* Init GST */
  gst_init(&argc, &argv);

  /* create a server instance */
  server = gst_rtsp_server_new();
  gst_rtsp_server_set_address(server, config->rtspListenIp);
  gst_rtsp_server_set_service(server, config->rtspListenPort);

  /* start serving */
  PDEBUG("rtsp server started");

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach(server, NULL);

  if(gst_rtsp_server_get_bound_port(server) != atoi(config->rtspListenPort))
  {
    PERROR("Could not bind to port: %s", config->rtspListenPort);
    goto error;
  }
  g_signal_connect(server, "client-connected", (GCallback)client_connected,
                   serverdata);

  return server;

error:
  return NULL;
}

static int
gen_new_mount_point_id()
{
  static guint32 id = 0;
  return ++id;
}

MountPoint *rtsp_setup_proxy_stream(ServerData *serverdata, const gchar *uri, const gchar *proxy, const gchar *path)
{
  GstRTSPMountPoints *mounts;
  GstRTSPServer *rtspserver = serverdata->server;
  MountPoint * mountpoint;
  GstRTSPMediaFactoryRtspProxy *factory;

  factory = gst_rtsp_media_factory_rtsp_proxy_new();
  gst_rtsp_media_factory_rtsp_proxy_set_uri(factory, uri);
  if(proxy != NULL)
  {
    gst_rtsp_media_factory_rtsp_proxy_set_proxy(factory, proxy);
  }
  gst_rtsp_media_factory_set_shared(GST_RTSP_MEDIA_FACTORY(factory), TRUE);

  mountpoint = (MountPoint *)g_malloc(sizeof(MountPoint));
  mountpoint->id = gen_new_mount_point_id();
  mountpoint->path = g_malloc(sizeof(gchar) * strlen(path) + 1);
  strcpy(mountpoint->path, path);
  mountpoint->factory = factory;
  serverdata->mountPoints = g_list_append(serverdata->mountPoints, mountpoint);
  mounts = gst_rtsp_server_get_mount_points(serverdata->server);
  gst_rtsp_mount_points_add_factory(mounts, path, GST_RTSP_MEDIA_FACTORY(factory));
  monitor_media(serverdata, GST_RTSP_MEDIA_FACTORY(factory));
  g_object_unref(mounts);
  return mountpoint;
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
  ServerData *serverdata = (ServerData*) user_data;
  clients = get_number_of_clients(server);
  // +1 since current client is not in list yet
  PDEBUG("client connected, total clients: %d", clients + 1);
  g_signal_connect(client, "closed", (GCallback)client_closed,
                   serverdata);
  g_signal_connect(client, "play-request", (GCallback)client_play,
                   serverdata);

  add_client(client);
}

static void
client_closed(GstRTSPClient *client,
              gpointer user_data)
{
  guint clients;
  ServerData *serverdata = (ServerData *) user_data;
  GstRTSPServer *server = serverdata->server;

  // clients = get_number_of_clients(server);
  // // -1 since current client is still in the list
  // PDEBUG("client closed, total clients: %d", clients - 1);
  callback_send_stream_ended(serverdata, get_client(client));
  remove_client(client);
  PDEBUG("client was removed");
}

static int
sendRenewStream(GstRTSPSession *session, gchar *mountPath)
{
    gint matched;
    GstRTSPSessionMedia *sessionMedia;
    GstRTSPMedia *RTSPMedia;
    GstElement *topBin, *dynpay, *rtspsrc;
    GstPromise *promise;

    sessionMedia = gst_rtsp_session_get_media(session, mountPath, &matched);
    if (!sessionMedia)
    {
        PERROR("Could not find session media");
        return -1;
    }
    RTSPMedia = gst_rtsp_session_media_get_media(sessionMedia);
    if(!RTSPMedia)
    {
        PERROR("Could not find RTSP media");
        return -1;
    }
    topBin = gst_rtsp_media_get_element(RTSPMedia);
    if (!GST_IS_ELEMENT(topBin))
    {
        PERROR("Could not get pipeline");
        return -1;
    }

    dynpay = gst_bin_get_by_name((GstBin *)topBin, "dynpay0");
    if (!GST_IS_ELEMENT(dynpay))
    {
        PERROR("Could not get payloader");
        g_object_unref(topBin);
        return -1;
    }
    rtspsrc = gst_bin_get_by_name((GstBin *)dynpay, "src");
    if (!GST_IS_ELEMENT(rtspsrc))
    {
        PERROR("Could not get rtspsrc");
        g_object_unref(topBin);
        g_object_unref(dynpay);
        return -1;
    }
    promise = gst_promise_new();
    g_signal_emit_by_name(rtspsrc, "set-parameter", "Renew-Stream", "yes", NULL, promise);

    g_object_unref(topBin);
    g_object_unref(dynpay);
    g_object_unref(rtspsrc);
    gst_promise_unref(promise);
    return 0;
}

static void
client_play(GstRTSPClient *client,
            GstRTSPContext *contex,
            gpointer user_data)
{
  GList *foundMountPoint = NULL;
  RTSPClient *rtspclient = NULL;
  MountPoint *mountpoint;
  GstRTSPMountPoints *rtspMountpoints;
  ServerData *serverdata = (ServerData*) user_data;
  gint matched;
  GstRTSPMediaFactory *mediaFactory;

  PDEBUG("client play");
  // find rtspclient
  rtspclient = get_client(client);
  if (rtspclient)
  {
    PDEBUG("found rtspclient");
    rtspMountpoints = gst_rtsp_client_get_mount_points(client);
    gchar *mountPath = gst_rtsp_mount_points_make_path(rtspMountpoints, contex->uri);
    mediaFactory = gst_rtsp_mount_points_match(rtspMountpoints, mountPath, &matched);
    // find mountpoint with same factory as in context
    PDEBUG("contex: uri %s", gst_rtsp_url_get_request_uri(contex->uri));
    foundMountPoint = g_list_find_custom(serverdata->mountPoints, mediaFactory, findMountPointFactory);
    if (foundMountPoint)
    {
      // set mountpoint on client
      mountpoint = (MountPoint*) foundMountPoint->data;
      rtspclient->mountpoint = mountpoint;
      PDEBUG("found mountpoint with path: %s", mountpoint->path);
    }
    sendRenewStream(contex->session, mountPath);
    callback_send_stream_started(serverdata, rtspclient);
    g_free(mountPath);
    g_object_unref(mediaFactory);
  }
}

