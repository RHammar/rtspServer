#include <gst/gst.h>
#include "pipelinemonitor.h"

static gboolean
bus_call(GstBus *bus,
         GstMessage *msg,
         gpointer data);

void monitor_pipeline(GstElement *pipeline)
{
  GstBus *bus;
  GstMessage *msg;
  guint bus_watch_id;

  g_print("monitoring pipeline\n");
  bus = gst_element_get_bus (pipeline);
  bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc) bus_call, pipeline);
  gst_object_unref (bus);
}

static gboolean
bus_call(GstBus *bus,
         GstMessage *msg,
         gpointer data)
{

  g_print("bus call\n");
  switch (GST_MESSAGE_TYPE(msg))
  {

  case GST_MESSAGE_STATE_CHANGED:
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data))
    {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
      g_print("Pipeline state changed from %s to %s:\n",
              gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
    }
    break;
  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    break;

  case GST_MESSAGE_ERROR:
  {
    gchar *debug;
    GError *error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    g_printerr("Error: %s\n", error->message);
    g_error_free(error);

    break;
  }
  default:
    break;
  }

  return TRUE;
}
