#include <gst/gst.h>

#include <gst/rtsp-server/rtsp-media-factory.h>

#ifndef __GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_H__
#define __GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_H__

G_BEGIN_DECLS

/* types for the media factory */
#define GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY (gst_rtsp_media_factory_rtsp_proxy_get_type())
#define GST_IS_RTSP_MEDIA_FACTORY_RTSP_PROXY(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY))
#define GST_IS_RTSP_MEDIA_FACTORY_RTSP_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY))
#define GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY, GstRTSPMediaFactoryRtspProxyClass))
#define GST_RTSP_MEDIA_FACTORY_RTSP_PROXY(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY, GstRTSPMediaFactoryRtspProxy))
#define GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_RTSP_MEDIA_FACTORY_RTSP_PROXY, GstRTSPMediaFactoryRtspProxyClass))
#define GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_CAST(obj) ((GstRTSPMediaFactoryRtspProxy *)(obj))
#define GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_CLASS_CAST(klass) ((GstRTSPMediaFactoryRtspProxyClass *)(klass))

typedef struct _GstRTSPMediaFactoryRtspProxy GstRTSPMediaFactoryRtspProxy;
typedef struct _GstRTSPMediaFactoryRtspProxyClass GstRTSPMediaFactoryRtspProxyClass;

/**
 * GstRTSPMediaFactoryRtspProxy:
 * @bin: the bin used for streaming
 *
 * The definition and logic for constructing the pipeline for a media. The media
 * can contain multiple streams like audio and video.
 */
struct _GstRTSPMediaFactoryRtspProxy
{
  GstRTSPMediaFactory parent;
  gchar *uri;
  gchar *proxy;
};

/**
 * GstRTSPMediaFactoryRtspProxyClass:
 * @get_element: Construct and return a #GstElement that is a #GstBin containing
 *       the elements to use for streaming the media. The bin should contain
 *       payloaders pay%d for each stream. The default implementation of this
 *       function returns the bin created from the launch parameter.
 *
 * The #GstRTSPMediaFactoryRtspProxy class structure.
 */
struct _GstRTSPMediaFactoryRtspProxyClass
{
  GstRTSPMediaFactoryClass parent_class;
};

GType gst_rtsp_media_factory_rtsp_proxy_get_type(void);

/* creating the factory */
GstRTSPMediaFactoryRtspProxy *gst_rtsp_media_factory_rtsp_proxy_new(void);

/* configuring the factory */
void gst_rtsp_media_factory_rtsp_proxy_configure(GstRTSPMediaFactoryRtspProxy *factory, const gchar *uri, const gchar *proxy);
// GstElement *gst_rtsp_media_factory_rtsp_proxy_get_bin(GstRTSPMediaFactoryRtspProxy *factory);

G_END_DECLS

#endif /* __GST_RTSP_MEDIA_FACTORY_RTSP_PROXY_H__ */