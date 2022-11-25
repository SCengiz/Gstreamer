#include <QCoreApplication>
#include <iostream>

#include <gst/gst.h>

typedef struct data_ {
    GstElement* pipeline;
    GstElement* source;
    GstElement* sink;
} data;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    gst_init(&argc, &argv);

    data data;
    data.source = gst_element_factory_make ("videotestsrc", "source");
    data.sink = gst_element_factory_make ("autovideosink", "sink");
    data.pipeline = gst_pipeline_new ("Pipeline");

    if(!data.source || !data.sink || !data.pipeline) {
        g_printerr("Elements could not created...\n");
        return -1;
    }

// NOTE: videotestsrc elements has a pattern attribute and 18 means "ball".
    g_object_set(data.source, "pattern", 18, NULL);

    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.sink, NULL);
    if (!gst_element_link_many(data.source, data.sink, NULL)) {
      g_printerr ("Elements could not be linked...\n");
      gst_object_unref(data.pipeline);
      return -1;
    }

    /* Start playing */
    gst_element_set_state (data.pipeline, GST_STATE_PLAYING);

    return a.exec();
}
