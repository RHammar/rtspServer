#include "rtsp_clients.h"
#include "log.h"
#include "server.h"

static GList *rtsp_clients = NULL;

static int
gen_new_client_id()
{
  static guint32 id = 0;
  return ++id;
}

static gint
compare_clients(gconstpointer a,
               gconstpointer b)
{
  RTSPClient *clienta = (RTSPClient*) a;
  GstRTSPClient *clientb = (GstRTSPClient*) b;
  PDEBUG("compare_clients a: %p, b: %p", clienta->client, clientb);
  if (clienta->client == clientb){
    PDEBUG("client matched by ref");
    return 0;
  }
  return -1;
}

static gint
compare_client_by_id(gconstpointer a,
                     gconstpointer b)
{
  RTSPClient *clienta = (RTSPClient*) a;
  guint32 *id_b = (guint32*) b;
  PDEBUG("compare_clients a: %u, b: %u", clienta->id, *id_b);
  if (clienta->id == *id_b){
    PDEBUG("client matched by id");
    return 0;
  }
  return -1;
}

RTSPClient *
add_client(GstRTSPClient *client)
{
  g_object_ref(client);
  RTSPClient *rtspclient;
  rtspclient = (RTSPClient *)g_malloc(sizeof(RTSPClient));
  rtspclient->id = gen_new_client_id();
  rtspclient->client = client;
  rtspclient->mountpoint = NULL;
  rtsp_clients = g_list_append(rtsp_clients, rtspclient);
  return rtspclient;
}

gint
close_client(guint32 id)
{
  RTSPClient *rtspclient;
  GList *found = NULL;
  found = g_list_find_custom(rtsp_clients, &id, compare_client_by_id);
  if (found)
  {
    rtspclient = (RTSPClient*) found->data;
    PDEBUG("closing client with id %d", id);
    gst_rtsp_client_close(rtspclient->client);
    PDEBUG("client closed");
    // remove_client(rtspclient->client);
    return 0;
  }
  return -1;
}

gint
remove_client(GstRTSPClient *client)
{
  RTSPClient *rtspclient;
  GList *found = NULL;
  found = g_list_find_custom(rtsp_clients, client, compare_clients);
  if (found)
  {
    rtspclient = (RTSPClient*) found->data;
    rtsp_clients = g_list_remove(rtsp_clients, rtspclient);
    g_object_unref(client);
    g_free(rtspclient);
    return 0;
  }
  return -1;
}

GList *
get_clients()
{
  return rtsp_clients;
}

RTSPClient *
get_client(GstRTSPClient *client)
{
  GList *found = NULL;
  RTSPClient *rtspclient = NULL;
  found = g_list_find_custom(rtsp_clients, client, compare_clients);
  if (found)
  {
    rtspclient = (RTSPClient*) found->data;
  }
  return rtspclient;
}