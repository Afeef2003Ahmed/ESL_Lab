#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <stdio.h>

typedef struct {
    GMainLoop *loop;
    GstElement *pipeline;
    int frame_count;
} ProgramData;

static GstFlowReturn
on_new_sample(GstElement *sink, ProgramData *data)
{
    GstSample *sample;
    GstBuffer *buffer;
    GstMapInfo map;
    
    sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    
    if (sample) {
        buffer = gst_sample_get_buffer(sample);
        gst_buffer_map(buffer, &map, GST_MAP_READ);
        
        data->frame_count++;
        printf("Frame %d: %zu bytes\n", data->frame_count, map.size);
        
        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);
    }
    
    return GST_FLOW_OK;
}

static gboolean
bus_callback(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    ProgramData *data = (ProgramData *)user_data;
    
    switch(GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            printf("Total frames: %d\n", data->frame_count);
            g_main_loop_quit(data->loop);
            break;
        case GST_MESSAGE_ERROR:
            g_printerr("Error\n");
            g_main_loop_quit(data->loop);
            break;
        default:
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    ProgramData data = {0};
    GstElement *source, *encoder, *decoder, *sink;
    GstBus *bus;
    GstCaps *caps;
    
    gst_init(&argc, &argv);
    
    if (argc != 2) {
        g_printerr("Usage: %s <num_frames>\n", argv[0]);
        return -1;
    }
    
    data.loop = g_main_loop_new(NULL, FALSE);
    data.frame_count = 0;
    
    data.pipeline = gst_pipeline_new("pipeline");
    source = gst_element_factory_make("camera-source", NULL);
    encoder = gst_element_factory_make("jpegenc", NULL);
    decoder = gst_element_factory_make("jpegdec", NULL);
    sink = gst_element_factory_make("appsink", NULL);
    
    if (!data.pipeline || !source || !encoder || !decoder || !sink) {
        g_printerr("Failed to create elements\n");
        return -1;
    }
    
    g_object_set(source, "num-buffers", atoi(argv[1]), NULL);
    g_object_set(sink, "emit-signals", TRUE, "sync", FALSE, NULL);
    
    g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), &data);
    
    /* caps = gst_caps_new_simple ("image/jpeg",
                              "width", G_TYPE_INT, 160,
                              "height", G_TYPE_INT, 120,
                              "framerate", GST_TYPE_FRACTION, 30, 1,
                              NULL); */


    /*caps = gst_caps_new_simple("video/x-raw",  
                           "width", G_TYPE_INT, 320,
                           "height", G_TYPE_INT, 240,
                           "framerate", GST_TYPE_FRACTION, 30, 1,
                           NULL); */
    
    gst_bin_add_many(GST_BIN(data.pipeline), source, encoder, decoder, sink, NULL);
    
    if (!gst_element_link_filtered(source, encoder, caps)) {
        g_printerr("Failed to link source->encoder\n");
        return -1;
    }
    
    if (!gst_element_link_many(encoder, decoder, sink, NULL)) {
        g_printerr("Failed to link encoder->decoder->sink\n");
        return -1;
    }
    
    gst_caps_unref(caps);
    
    bus = gst_pipeline_get_bus(GST_PIPELINE(data.pipeline));
    gst_bus_add_watch(bus, bus_callback, &data);
    gst_object_unref(bus);
    
    gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    g_main_loop_run(data.loop);
    
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    gst_object_unref(data.pipeline);
    g_main_loop_unref(data.loop);
    
    return 0;
}