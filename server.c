#include <gst/gst.h>
#include <libsoup/soup.h>

#include "manage.h"
#include "rtsp.h"
#include "log.h"

int main(int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  SoupServer *soup;
  char *manage = "127.0.0.1:7777";

  PDEBUG("rtsp_start");
  loop = g_main_loop_new(NULL, FALSE);
  server = rtsp_start(argc, argv);
  soup = manage_start(server);
  if (!soup){
    goto error;
  }
  g_main_loop_run(loop);

  return 0;
error:
  return -1;
}
