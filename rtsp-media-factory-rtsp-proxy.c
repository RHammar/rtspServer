#include <string.h>
#include <stdio.h>
#include <gst/gst.h>

#include "rtsp-media-factory-rtsp-proxy.h"
#include "rtspproxypipeline.h"
#include "log.h"

#define GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE((obj), GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY, GstRTSPMediaFactoryRtspProxyPrivate))

struct _GstRTSPMediaFactoryRtspProxyPrivate
{
  GMutex lock;
  gchar *uri;
  gchar *proxy;
};

enum
{
  PROP_0,
  PROP_URI,
  PROP_PROXY,
  PROP_LAST
};

GST_DEBUG_CATEGORY_STATIC(rtsp_media_factory_rtsp_proxy_debug);
#define GST_CAT_DEFAULT rtsp_media_factory_rtsp_proxy_debug

G_DEFINE_TYPE(GstRTSPMediaFactoryRtspProxy, gst_rtsp_media_factory_rtsp_proxy, GST_TYPE_RTSP_MEDIA_FACTORY /*parent class*/);

static void
gst_rtsp_media_factory_rtsp_proxy_get_property(GObject *object,
                                               guint propid,
                                               GValue *value,
                                               GParamSpec *pspec);

static void
gst_rtsp_media_factory_rtsp_proxy_set_property(GObject *object,
                                               guint propid,
                                               const GValue *value,
                                               GParamSpec *pspec);

static void gst_rtsp_media_factory_rtsp_proxy_finalize(GObject *obj);

static GstElement *
rtsp_media_factory_custom_create_element(GstRTSPMediaFactory *factory, const GstRTSPUrl *url);
static gchar *gst_rtsp_media_factory_rtsp_proxy_gen_key (GstRTSPMediaFactory * factory,
    const GstRTSPUrl * url);

static void
gst_rtsp_media_factory_rtsp_proxy_class_init(GstRTSPMediaFactoryRtspProxyClass *klass)
{
  GstRTSPMediaFactoryClass *gstrtspmediafactory_class;
  GObjectClass *gobject_class;
  gobject_class = (GObjectClass *)klass;

  g_type_class_add_private(klass, sizeof(GstRTSPMediaFactoryRtspProxyPrivate));
  gstrtspmediafactory_class = (GstRTSPMediaFactoryClass *)klass;
  gstrtspmediafactory_class->create_element = rtsp_media_factory_custom_create_element;
  gstrtspmediafactory_class->gen_key = gst_rtsp_media_factory_rtsp_proxy_gen_key;
  gobject_class->get_property = gst_rtsp_media_factory_rtsp_proxy_get_property;
  gobject_class->set_property = gst_rtsp_media_factory_rtsp_proxy_set_property;
  gobject_class->finalize = gst_rtsp_media_factory_rtsp_proxy_finalize;

  g_object_class_install_property(gobject_class, PROP_URI,
                                  g_param_spec_object("uri", "URI", "URI used", GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_PROXY,
                                  g_param_spec_object("proxy", "Proxy", "Proxy used", GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  GST_DEBUG_CATEGORY_INIT(rtsp_media_factory_rtsp_proxy_debug, "rtspmediafactoryrtspproxy", 0,
                          "GstRTSPMediaFactoryRTSPProxy");
}

static void
gst_rtsp_media_factory_rtsp_proxy_init(GstRTSPMediaFactoryRtspProxy *factory)
{
  PDEBUG("gst_rtsp_media_factory_rtsp_proxy_init");
  GstRTSPMediaFactoryRtspProxyPrivate *priv =
      GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_GET_PRIVATE(factory);
  factory->priv = priv;
  priv->uri = NULL;
  priv->proxy = NULL;
  g_mutex_init(&priv->lock);
}

static void
gst_rtsp_media_factory_rtsp_proxy_finalize(GObject *obj)
{
  PDEBUG("gst_rtsp_media_factory_rtsp_proxy_finalize");
  GstRTSPMediaFactoryRtspProxy *factory = GST_RTSP_MEDIA_FACTORY_RTSP_PROXY(obj);
  GstRTSPMediaFactoryRtspProxyPrivate *priv = factory->priv;
  if (priv->uri)
  {
    g_free(priv->uri);
  }
  if (priv->proxy)
  {
    g_free(priv->proxy);
  }
  g_mutex_clear(&priv->lock);

  G_OBJECT_CLASS(gst_rtsp_media_factory_rtsp_proxy_parent_class)->finalize(obj);
}

GstRTSPMediaFactoryRtspProxy *
gst_rtsp_media_factory_rtsp_proxy_new(void)
{
  GstRTSPMediaFactoryRtspProxy *result;
  PDEBUG("gst_rtsp_media_factory_rtsp_proxy_new");
  result = g_object_new(GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY, NULL);

  return result;
}

static void
gst_rtsp_media_factory_rtsp_proxy_get_property(GObject *object,
                                               guint propid,
                                               GValue *value,
                                               GParamSpec *pspec)
{
  GstRTSPMediaFactoryRtspProxy *factory = GST_RTSP_MEDIA_FACTORY_RTSP_PROXY(object);

  switch (propid)
  {
  case PROP_URI:
    g_value_set_object(value, gst_rtsp_media_factory_rtsp_proxy_get_uri(factory));
    break;
  case PROP_PROXY:
    g_value_set_object(value, gst_rtsp_media_factory_rtsp_proxy_get_proxy(factory));
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propid, pspec);
  }
}

static void
gst_rtsp_media_factory_rtsp_proxy_set_property(GObject *object,
                                               guint propid,
                                               const GValue *value,
                                               GParamSpec *pspec)
{
  GstRTSPMediaFactoryRtspProxy *factory = GST_RTSP_MEDIA_FACTORY_RTSP_PROXY(object);

  switch (propid)
  {
  case PROP_URI:
    gst_rtsp_media_factory_rtsp_proxy_set_uri(factory, g_value_get_object(value));
    break;
  case PROP_PROXY:
    gst_rtsp_media_factory_rtsp_proxy_set_uri(factory, g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propid, pspec);
  }
}

void gst_rtsp_media_factory_rtsp_proxy_set_uri(GstRTSPMediaFactoryRtspProxy *factory,
                                               const gchar *uri)
{
  g_return_if_fail(GST_IS_RTSP_MEDIA_FACTORY_RTSP_PROXY(factory));
  g_return_if_fail(uri != NULL);

  g_mutex_lock(&factory->priv->lock);
  g_free(factory->priv->uri);
  factory->priv->uri = g_strdup(uri);
  g_mutex_unlock(&factory->priv->lock);
}

void gst_rtsp_media_factory_rtsp_proxy_set_proxy(GstRTSPMediaFactoryRtspProxy *factory,
                                                 const gchar *proxy)
{
  g_return_if_fail(GST_IS_RTSP_MEDIA_FACTORY_RTSP_PROXY(factory));
  g_return_if_fail(proxy != NULL);

  g_mutex_lock(&factory->priv->lock);
  g_free(factory->priv->proxy);
  factory->priv->proxy = g_strdup(proxy);
  g_mutex_unlock(&factory->priv->lock);
}

gchar *
gst_rtsp_media_factory_rtsp_proxy_get_uri(GstRTSPMediaFactoryRtspProxy *factory)
{
  gchar *result;
  g_return_val_if_fail(GST_IS_RTSP_MEDIA_FACTORY_RTSP_PROXY(factory), NULL);
  g_mutex_lock(&factory->priv->lock);
  result = g_strdup(factory->priv->uri);
  g_mutex_unlock(&factory->priv->lock);

  return result;
}

gchar *
gst_rtsp_media_factory_rtsp_proxy_get_proxy(GstRTSPMediaFactoryRtspProxy *factory)
{
  gchar *result;
  g_return_val_if_fail(GST_IS_RTSP_MEDIA_FACTORY_RTSP_PROXY(factory), NULL);
  g_mutex_lock(&factory->priv->lock);
  result = g_strdup(factory->priv->proxy);
  g_mutex_unlock(&factory->priv->lock);

  return result;
}

static gchar *
gst_rtsp_media_factory_rtsp_proxy_gen_key (GstRTSPMediaFactory * factory, const GstRTSPUrl * url)
{
  gchar *result;
  const gchar *pre_query;
  const gchar *query;

  pre_query = url->query ? "?" : "";
  query = url->query ? url->query : "";

  result = g_strdup_printf ("%s%s%s", url->abspath, pre_query, query);

  return result;
}

static void
no_more_pads_cb(GstElement *uribin, GstElement *element)
{
  PDEBUG("no-more-pads factory");
  gst_element_no_more_pads(element);
}

static GstElement *
rtsp_media_factory_custom_create_element(GstRTSPMediaFactory *factory, const GstRTSPUrl *url)
{
  GstRTSPMediaFactoryRtspProxy *factoryCustom = GST_RTSP_MEDIA_FACTORY_RTSP_PROXY(factory);
  GstElement *topbin, *element, *pipeline;
  PDEBUG("custom create element");
  // if (!factoryCustom->bin)
  // {
  topbin = gst_bin_new("GstRTSPMediaFactoryRtspProxy");
  g_object_set(topbin, "name", "customtopbin", NULL);
  /* our bin will dynamically expose payloaded pads */
  // element = gst_bin_new ("dynpay0");
  // pipeline = createOggPipeline();
  pipeline = createRtspProxyPipeline(factoryCustom->priv->uri, factoryCustom->priv->proxy);
  //g_signal_connect (pipeline, "no-more-pads", (GCallback) no_more_pads_cb,
  //          pipeline);
  //gst_bin_add (GST_BIN_CAST(element), pipeline);
  gst_bin_add(GST_BIN_CAST(topbin), pipeline);
  // if (GST_RTSP_MEDIA_FACTORY_CUSTOM(factory)->bin == NULL)
  // {
  //   goto error;
  // }
  // else
  // {
  //   element = GST_RTSP_MEDIA_FACTORY_CUSTOM(factory)->bin;
  //   gst_bin_add(GST_BIN_CAST(topbin), element);
  // }
  // factoryCustom->bin = topbin;
  // }
  // return factoryCustom->bin;
  return topbin;
  // return GST_RTSP_MEDIA_FACTORY_CUSTOM(factory)->bin;

error:

  g_critical("no bin set");
  return NULL;
}

int
sendRenewStream(GstRTSPClient *client, GstRTSPSession *session, const GstRTSPUrl *uri)
{
    GstRTSPMessage request = { 0 };
    gchar *msg = "Renew-Stream: yes";
    guint msglen = strlen(msg);

    gst_rtsp_message_init_request(&request, GST_RTSP_SET_PARAMETER, (gchar *) uri);
    gst_rtsp_message_set_body(&request, msg, msglen);
    gst_rtsp_client_send_message(client, session, &request);
}
