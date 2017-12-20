#include <string.h>
#include <stdio.h>
#include <gst/gst.h>

#include "rtsp-media-factory-rtsp-proxy.h"
#include "rtspproxypipeline.h"
#include "log.h"

G_DEFINE_TYPE(GstRTSPMediaFactoryRtspProxy, gst_rtsp_media_factory_rtsp_proxy, GST_TYPE_RTSP_MEDIA_FACTORY /*parent class*/);

static GstElement *
rtsp_media_factory_custom_create_element(GstRTSPMediaFactory *factory, const GstRTSPUrl *url);

static void
gst_rtsp_media_factory_rtsp_proxy_class_init(GstRTSPMediaFactoryRtspProxyClass *klass)
{
  GstRTSPMediaFactoryClass *gstrtspmediafactory_class;

  gstrtspmediafactory_class = (GstRTSPMediaFactoryClass *)klass;
  gstrtspmediafactory_class->create_element = rtsp_media_factory_custom_create_element;
}

static void
gst_rtsp_media_factory_rtsp_proxy_init(GstRTSPMediaFactoryRtspProxy *factory)
{
  factory->uri = NULL;
  factory->proxy = NULL;
}

GstRTSPMediaFactoryRtspProxy *
gst_rtsp_media_factory_rtsp_proxy_new(void)
{
  GstRTSPMediaFactoryRtspProxy *result;

  result = g_object_new(GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY, NULL);

  return result;
}

void gst_rtsp_media_factory_rtsp_proxy_configure(GstRTSPMediaFactoryRtspProxy *factory, const gchar *uri, const gchar *proxy)
{
  PDEBUG("configuring pipeline");
  g_return_if_fail(GST_IS_RTSP_MEDIA_FACTORY_RTSP_PROXY(factory));
  g_return_if_fail(uri != NULL);

  if(factory->uri != NULL) {
    g_free(factory->uri);
  }
  if(factory->proxy != NULL) {
    g_free(factory->proxy);
  }
  factory->uri = strdup(uri);
  if(proxy) {
    factory->proxy = strdup(proxy);
  }
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
  pipeline = createRtspProxyPipeline(factoryCustom->uri, factoryCustom->proxy);
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
