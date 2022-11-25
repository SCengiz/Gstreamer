#include <QCoreApplication>
#include <iostream>

#include <gst/gst.h>

typedef struct data_ {
    GstElement* pipeline;
    GstElement* rtspsrc;
    GstElement* decodebin;
    GstElement* autovideoconvert;
    GstElement* autovideosink;
} data;

static void cb_new_pad(GstElement* element, GstPad* pad, gpointer userData) {
    GstElement* other = (GstElement*)userData;
    gchar* padName = gst_pad_get_name(pad);
    g_print("padName: %s and otherElementName: %s\n", padName, gst_element_get_name(element));

    gst_element_link(element, other);
    g_free(padName);
}

static void on_pad_added(GstElement*, GstPad *pad, gpointer data){
    GstPad *sinkpad;
    GstElement *other = (GstElement*)data;

    sinkpad = gst_element_get_static_pad(other, "sink");
    if(GST_PAD_LINK_FAILED(gst_pad_link(pad, sinkpad))) {
        g_printerr("Failed Linking Pads...\n");
    }

    gst_object_unref(sinkpad);
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    gst_init(&argc, &argv);

    data data;
    data.rtspsrc = gst_element_factory_make ("rtspsrc", "source");
    data.decodebin = gst_element_factory_make ("decodebin", "decodebin");
    data.autovideoconvert = gst_element_factory_make ("autovideoconvert", "convert");
    data.autovideosink = gst_element_factory_make ("autovideosink", "sink");
    data.pipeline = gst_pipeline_new ("Pipeline");

    if(!data.rtspsrc || !data.decodebin || !data.autovideoconvert || !data.autovideosink || !data.pipeline) {
        g_printerr("Elements could not created...\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(data.pipeline), data.rtspsrc, data.decodebin,
                     data.autovideoconvert, data.autovideosink, NULL);

    g_object_set(data.rtspsrc, "location", "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4", NULL);
/*
            | RTSPSRC ¦ -> | DECODEBIN | **** CALLBACK **** | CONVERT | -> | SINK |
Here we use a callback for linking the some elements.
*/
    if(!gst_element_link(data.autovideoconvert, data.autovideosink)) {
        g_printerr ("Elements could not be linked...\n");
    }

    g_signal_connect(data.rtspsrc, "pad-added", G_CALLBACK(::on_pad_added), data.decodebin);
    g_signal_connect(data.decodebin, "pad-added", G_CALLBACK(::on_pad_added), data.autovideoconvert);
    gst_element_set_state(data.pipeline, GST_STATE_PLAYING);

    return a.exec();
}
