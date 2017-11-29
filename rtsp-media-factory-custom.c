#include "rtsp-media-factory-custom.h"

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

  g_print("setting pipeline\n");
  g_return_if_fail(GST_IS_RTSP_MEDIA_FACTORY_CUSTOM(factory));
  g_return_if_fail(bin != NULL);

  if (factory->bin)
    gst_object_unref(factory->bin);
  if (bin)
    gst_object_ref(bin);
  factory->bin = bin;
}

static GstElement *
rtsp_media_factory_custom_create_element(GstRTSPMediaFactory *factory, const GstRTSPUrl *url)
{
  GstElement *topbin, *element;

  g_print("custom create element\n");

  topbin = gst_bin_new("GstRTSPMediaFactoryCustom");
  if (GST_RTSP_MEDIA_FACTORY_CUSTOM(factory)->bin == NULL)
  {
    goto error;
  }
  else
  {
    element = GST_RTSP_MEDIA_FACTORY_CUSTOM(factory)->bin;
    gst_bin_add(GST_BIN_CAST(topbin), element);
  }
  return topbin;

error:

  g_critical("no bin set");
  return NULL;
}