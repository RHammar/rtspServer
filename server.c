#include <gst/gst.h>
#include <libsoup/soup.h>

#include "httpserver.h"
#include "rtsp.h"
#include "log.h"
#include "server.h"

int main(int argc, char *argv[])
{
  ServerData serverdata;
  serverdata.mountPoints = NULL;
  serverdata.clients = NULL;
  char *manage = "127.0.0.1:7777";

  PDEBUG("rtsp_start");
  serverdata.loop = g_main_loop_new(NULL, FALSE);
  serverdata.server = rtsp_start(&serverdata, argc, argv);
  serverdata.soup = httpserver_start(&serverdata);
  if (!serverdata.soup){
    goto error;
  }
  g_main_loop_run(serverdata.loop);

  return 0;
error:
  return -1;
}
