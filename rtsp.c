#include <gst/gst.h>

#include "rtsp.h"
#include "log.h"

GstRTSPServer* rtsp_start(int argc, char *argv[])
{
  GstRTSPServer *server;

  /* Init GST */
  gst_init(&argc, &argv);

  /* create a server instance */
  server = gst_rtsp_server_new();
  /* start serving */
  PDEBUG("rtsp server started");

  return server;
}

int rtsp_setup_stream(GstRTSPServer *server, char *pipeline)
{
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  mounts = gst_rtsp_server_get_mount_points(server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_launch(factory, pipeline);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

  /* don't need the ref to the mapper anymore */
  g_object_unref(mounts);

  /* attach the server to the default maincontext */
  PDEBUG("stream available at 127.0.0.1:8554/test");
  gst_rtsp_server_attach(server, NULL);
}