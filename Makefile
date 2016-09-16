SRCS = server.c manage.c rtsp.c
PROGS = rtsp_server

all:
	gcc -g -I/home/rickardh/Development/gst-rtsp-server-1.2.3/ -L/home/rickardh/Development/gst-rtsp-server-1.2.3/gst/rtsp-server/.libs  $(SRCS) -o $(PROGS) `pkg-config gstreamer-1.0 gio-2.0 libsoup-2.4 --libs --cflags` -lgstrtspserver-1.0 -levent

clean:
	rm -rf $(PROGS)