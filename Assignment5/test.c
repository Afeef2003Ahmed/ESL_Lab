#include <gst/gst.h>
#include <glib.h>

static gboolean
bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar *debug;
      GError *error;
      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);
      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

int main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstElement *pipeline, *source, *encoder, *decoder, *sink;
  GstBus *bus;
  guint bus_watch_id;
  GstCaps *caps;

  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  if (argc != 2) {
    g_printerr ("Usage: %s <output_filename.yuv>\n", argv[0]);
    return -1;
  }

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("yuv-recorder");
  source   = gst_element_factory_make ("videotestsrc", "test-source");  
  encoder  = gst_element_factory_make ("jpegenc",      "jpeg-encoder");
  decoder  = gst_element_factory_make ("jpegdec",      "jpeg-decoder");
  sink     = gst_element_factory_make ("filesink",     "file-sink");

  if (!pipeline || !source || !encoder || !decoder || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Configure elements */
  
  g_object_set (G_OBJECT (source), "pattern", 0, NULL);
  
  
  g_object_set (G_OBJECT (source), "num-buffers", 300, NULL);
  
  g_object_set (G_OBJECT (sink), "location", argv[1], NULL);

  /* Create caps to specify video format */
  caps = gst_caps_new_simple ("video/x-raw",
                              "width", G_TYPE_INT, 320,
                              "height", G_TYPE_INT, 240,
                              "framerate", GST_TYPE_FRACTION, 30, 1,
                              NULL);

  /* Add message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* Add all elements to pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, encoder, decoder, sink, NULL);

  /* Link elements with caps */
  if (!gst_element_link_filtered (source, encoder, caps)) {
    g_printerr ("Failed to link source -> encoder\n");
    return -1;
  }

  if (!gst_element_link_many (encoder, decoder, sink, NULL)) {
    g_printerr ("Failed to link encoder -> decoder -> sink\n");
    return -1;
  }

  gst_caps_unref (caps);

  g_print ("Recording test pattern to: %s\n", argv[1]);
  g_print ("Resolution: 320x240 @ 30fps\n");
  
  
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  g_print ("Running...\n");
  g_main_loop_run (loop);

  g_print ("Stopping recording\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}