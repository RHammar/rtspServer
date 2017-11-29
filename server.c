#include <gst/gst.h>

#include "manage.h"
#include "rtsp.h"
#include "log.h"

int main(int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GSocketService *service;
  char *manage = "127.0.0.1:7777";

  PDEBUG("rtsp_start");
  loop = g_main_loop_new(NULL, FALSE);
  server = rtsp_start(argc, argv);
  service = manage_start(server);
  g_main_loop_run(loop);

  /* Stop service when out of the main loop */
  g_socket_service_stop(service);
  return 0;
}
