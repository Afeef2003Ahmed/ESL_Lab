#include "camera.h"

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <pthread.h>
#include <stdio.h>

static struct {
    GstElement   *pipeline;
    GstElement   *source, *decoder, *convert, *sink;
    GMainLoop    *loop;
    pthread_t     thread;
    FrameCallback cb;
    void         *user;
} cam;

static GstFlowReturn on_new_sample(GstElement *sink, gpointer user_data)
{
    (void)user_data;

    GstSample  *sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if (!sample) return GST_FLOW_OK;

    GstBuffer  *buffer = gst_sample_get_buffer(sample);
    GstMapInfo  map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    GstCaps      *caps = gst_sample_get_caps(sample);
    GstStructure *s    = gst_caps_get_structure(caps, 0);
    int width = 0, height = 0;
    gst_structure_get_int(s, "width",  &width);
    gst_structure_get_int(s, "height", &height);

    if (cam.cb && width > 0 && height > 0)
        cam.cb(map.data, width, height, cam.user);

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

static gboolean on_bus_message(GstBus *bus, GstMessage *msg, gpointer data)
{
    (void)bus;
    GMainLoop *loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: {
            GError *err; gchar *dbg;
            gst_message_parse_error(msg, &err, &dbg);
            g_printerr("[Camera] Error: %s\n%s\n", err->message, dbg);
            g_error_free(err);
            g_free(dbg);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

static void *gst_thread(void *arg)
{
    (void)arg;
    g_main_loop_run(cam.loop);
    return NULL;
}

int camera_start(FrameCallback cb, void *user)
{
    cam.cb   = cb;
    cam.user = user;

    cam.pipeline = gst_pipeline_new("cam_pipeline");
    cam.source   = gst_element_factory_make("v4l2src",      "camera-source");
    cam.decoder  = gst_element_factory_make("jpegdec",      "jpeg-decoder");
    cam.convert  = gst_element_factory_make("videoconvert", "converter");
    cam.sink     = gst_element_factory_make("appsink",      "app-sink");

    if (!cam.pipeline || !cam.source || !cam.decoder ||
        !cam.convert  || !cam.sink) {
        g_printerr("[Camera] Failed to create elements\n");
        return -1;
    }

    g_object_set(cam.source, "device", CAM_DEVICE, NULL);

    GstCaps *src_caps = gst_caps_new_simple("image/jpeg",
        "width",     G_TYPE_INT,        CAM_WIDTH,
        "height",    G_TYPE_INT,        CAM_HEIGHT,
        "framerate", GST_TYPE_FRACTION, CAM_FPS, 1,
        NULL);

    GstCaps *sink_caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "I420",
        NULL);

    g_object_set(cam.sink, "emit-signals", TRUE, "sync", FALSE,
                 "caps", sink_caps, NULL);
    gst_caps_unref(sink_caps);

    g_signal_connect(cam.sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

    gst_bin_add_many(GST_BIN(cam.pipeline),
        cam.source, cam.decoder, cam.convert, cam.sink, NULL);

    if (!gst_element_link_filtered(cam.source, cam.decoder, src_caps)) {
        g_printerr("[Camera] Failed to link source->decoder\n");
        gst_caps_unref(src_caps);
        return -1;
    }
    gst_caps_unref(src_caps);

    if (!gst_element_link_many(cam.decoder, cam.convert, cam.sink, NULL)) {
        g_printerr("[Camera] Failed to link decoder->convert->sink\n");
        return -1;
    }

    cam.loop    = g_main_loop_new(NULL, FALSE);
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(cam.pipeline));
    gst_bus_add_watch(bus, on_bus_message, cam.loop);
    gst_object_unref(bus);

    gst_element_set_state(cam.pipeline, GST_STATE_PLAYING);
    pthread_create(&cam.thread, NULL, gst_thread, NULL);

    printf("[Camera] Pipeline started: MJPEG %dx%d @ %dfps -> I420\n",
           CAM_WIDTH, CAM_HEIGHT, CAM_FPS);
    return 0;
}

void camera_stop(void)
{
    g_main_loop_quit(cam.loop);
    pthread_join(cam.thread, NULL);
    gst_element_set_state(cam.pipeline, GST_STATE_NULL);
    gst_object_unref(cam.pipeline);
    g_main_loop_unref(cam.loop);
    printf("[Camera] Pipeline stopped\n");
}
