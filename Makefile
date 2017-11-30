SRCS = server.c manage.c rtsp.c rtsp-media-factory-custom.c pipelinebuilder.c pipelinemonitor.c mediamonitor.c
PROGS = rtsp_server
LDFLAGS =-g
CFLAGS =-g

all:
	gcc $(CFLAGS) `pkg-config --cflags glib-2.0 gstreamer-1.0 gio-2.0 libsoup-2.4 gstreamer-rtsp-server-1.0`\
	 $(SRCS) -o $(PROGS)\
	 `pkg-config glib-2.0 gstreamer-1.0 gio-2.0 libsoup-2.4 --libs`\
	 -lgstrtspserver-1.0 -levent -lsoup-2.4 $(LDFLAGS)\


clean:
	rm -rf $(PROGS)