#include <gst/gst.h>
#include <libsoup/soup.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "httpserver.h"
#include "rtsp.h"
#include "log.h"
#include "server.h"
#include "config.h"

int main(int argc, char *argv[])
{
  guint major;
  guint minor;
  guint micro;
  guint nano;
  gst_version(&major, &minor, &micro, &nano);
  PINFO("%s started. GST version %u.%u.%u.%u", argv[0], major, minor, micro, nano);
  ServerData serverdata;
  RtspConfiguration config;
  serverdata.mountPoints = NULL;
  serverdata.config = NULL;
  int opt = 0;
  gchar *filename = NULL;
  char *manage = "127.0.0.1:7777";
  while ((opt = getopt(argc, argv, "f:")) != -1) {
    switch(opt) {
      case 'f':
        filename = optarg;
        break;
      case '?':
        if (optopt == 'f') {
        PERROR("Missing mandatory input option");
        exit(-1);
        } else {
          PERROR("Invalid option received");
          exit(-1);
        }
      break;
    }
  }
  if(!filename)
  {
    PERROR("no config file set");
    exit(-1);
  }
  if (getConfig(&config, filename) != 0)
  {
    PERROR("Error while reading config");
    exit(-1);
  }
  serverdata.config = &config;
  PDEBUG("rtsp_start");
  PDEBUG("config calback url: %s", config.callbackUrl);
  serverdata.loop = g_main_loop_new(NULL, FALSE);
  serverdata.server = rtsp_start(&serverdata, &config, argc, argv);
  serverdata.soup = httpserver_start(&serverdata, &config);
  if (!serverdata.soup || !serverdata.server){
    goto error;
  }
  g_main_loop_run(serverdata.loop);

  return 0;
error:
  return -1;
}
