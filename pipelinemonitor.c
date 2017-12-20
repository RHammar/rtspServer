#include <gst/gst.h>
#include "pipelinemonitor.h"
#include "log.h"
#include "server.h"

static gboolean
bus_call(GstBus *bus,
         GstMessage *msg,
         gpointer data);

void monitor_pipeline(ServerData *serverdata, GstElement *pipeline)
{
  GstBus *bus;
  GstMessage *msg;
  guint bus_watch_id;

  PDEBUG("monitoring pipeline");
  bus = gst_element_get_bus(pipeline);
  gst_bus_add_signal_watch(bus);
  g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(bus_call), pipeline);
  gst_object_unref(bus);
}

static gboolean
bus_call(GstBus *bus,
         GstMessage *msg,
         gpointer data)
{
  GstMessageType type;

  PDEBUG("bus call: %s", GST_MESSAGE_TYPE_NAME(msg));
  switch (GST_MESSAGE_TYPE(msg))
  {

  case GST_MESSAGE_STATE_CHANGED:
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data))
    {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
      PDEBUG("Pipeline state changed from %s to %s:",
             gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
    }
    break;
  case GST_MESSAGE_EOS:
    PDEBUG("End of stream");
    break;

  case GST_MESSAGE_ERROR:
  {
    gchar *debug;
    GError *error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    PDEBUG("Error: %s", error->message);
    g_error_free(error);

    break;
  }
  case GST_MESSAGE_STREAM_STATUS:
  {
    GstStreamStatusType type;
    GstElement *owner;
    gchar *path;
    gst_message_parse_stream_status (msg, &type, &owner);
    path = gst_object_get_path_string (GST_MESSAGE_SRC (msg));
    PDEBUG("stream status: %d path: %s", type, path);
    break;
  }
  default:
    break;
  }

  return TRUE;
}
