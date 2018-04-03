SRCS = server.c httpserver.c rtsp.c rtsp-media-factory-custom.c pipelinebuilder.c pipelinemonitor.c mediamonitor.c rtsp-media-factory-rtsp-proxy.c rtspproxypipeline.c config.c rtsp_clients.c
PROGS = rtsp_server
#LDFLAGS =-g -L/home/rickardh/Development/gst-rtsp-server-1.9.2/gst/rtsp-server/.libs
LDFLAGS =-g
CFLAGS =-g

all:
	gcc $(CFLAGS) `pkg-config --cflags glib-2.0 gstreamer-1.0 gio-2.0 libsoup-2.4 json-glib-1.0 gstreamer-rtsp-1.0 gstreamer-rtsp-server-1.0`\
	 $(SRCS) -o $(PROGS)\
	 `pkg-config glib-2.0 gstreamer-1.0 gstreamer-rtsp-1.0 gio-2.0 libsoup-2.4 json-glib-1.0 gstreamer-rtsp-server-1.0 --libs`\
	 -levent -lsoup-2.4 $(LDFLAGS)\


clean:
	rm -rf $(PROGS)
