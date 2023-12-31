#include <string.h>
#include <stdio.h>
#include <gst/gst.h>

#include "rtsp-media-factory-custom.h"
#include "pipelinebuilder.h"
#include "log.h"
G_DEFINE_TYPE(GstRTSPMediaFactoryCustom, gst_rtsp_media_factory_custom, GST_TYPE_RTSP_MEDIA_FACTORY /*parent class*/);

static GstElement *
rtsp_media_factory_custom_create_element(GstRTSPMediaFactory *factory, const GstRTSPUrl *url);

static void
gst_rtsp_media_factory_custom_class_init(GstRTSPMediaFactoryCustomClass *klass)
{
  GstRTSPMediaFactoryClass *gstrtspmediafactory_class;

  gstrtspmediafactory_class = (GstRTSPMediaFactoryClass *)klass;
  gstrtspmediafactory_class->create_element = rtsp_media_factory_custom_create_element;
}

static void
gst_rtsp_media_factory_custom_init(GstRTSPMediaFactoryCustom *factory)
{
  factory->bin = NULL;
}

GstRTSPMediaFactoryCustom *
gst_rtsp_media_factory_custom_new(void)
{
  GstRTSPMediaFactoryCustom *result;

  result = g_object_new(GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM, NULL);

  return result;
}

void gst_rtsp_media_factory_custom_set_bin(GstRTSPMediaFactoryCustom *factory,
                                           GstElement *bin)
{

  PDEBUG("setting pipeline");
  g_return_if_fail(GST_IS_RTSP_MEDIA_FACTORY_CUSTOM(factory));
  g_return_if_fail(bin != NULL);

  if (factory->bin)
    gst_object_unref(factory->bin);
  if (bin)
    gst_object_ref(bin);
  factory->bin = bin;
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
  GstRTSPMediaFactoryCustom *factoryCustom = GST_RTSP_MEDIA_FACTORY_CUSTOM(factory);
  GstElement *topbin, *element, *pipeline;
  PDEBUG("custom create element");
  // if (!factoryCustom->bin)
  // {
  topbin = gst_bin_new("GstRTSPMediaFactoryCustom");
  g_object_set(topbin, "name", "customtopbin", NULL);
  /* our bin will dynamically expose payloaded pads */
  // element = gst_bin_new ("dynpay0");
  // pipeline = createOggPipeline();
  pipeline = createMp4Pipeline();
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
