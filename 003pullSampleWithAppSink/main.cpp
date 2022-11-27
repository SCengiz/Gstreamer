#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QImage>
#include <QtGui>
#include <QPixmap>

#include <iostream>
#include <thread>
#include <chrono>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

struct data_ {
    GstElement* pipeline;
    GstElement* source;
    GstElement* converter;
    GstElement* sink;

    QImage image;
};

cv::Mat QImageToMat(const QImage & image){
    cv::Mat out(image.height(),image.width(),CV_8UC4,(void *)image.constBits(),image.bytesPerLine());
    return out;
}
//cv::Mat QImageToMat(QImage& image) {
//    std::cout << "width:" << image.width() << " height:" << image.height() << std::endl;
//    return cv::Mat(image.height(), image.width(), CV_16SC1 , image.bits(), image.bytesPerLine());
//}

GstFlowReturn newSample(GstAppSink* sink, gpointer data) {
    GstSample* sample;
    GstBuffer* buffer;
    GstMapInfo mapInfo;
    struct data_* inData = static_cast<struct data_*>(data);

    sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if(sample) {
        std::cout << "Sample TRUE\n";
        buffer = gst_sample_get_buffer(sample);
        if(buffer) {
            std::cout << "Buffer TRUE\n";
            gst_buffer_map(buffer, &mapInfo, GST_MAP_READ);
            inData->image = QImage(mapInfo.data, 600, 400, QImage::Format_RGB32);
            std::cout << "::" << inData->image.depth() << std::endl;
            std::cout << "::" << inData->image.bits() << std::endl;

            cv::Mat mat = ::QImageToMat(inData->image);
//            cv::imwrite("as.jpg", mat);

            if(mat.size) {
                std::cout << "Hello\n";
                cv::imshow("Window", mat);
                cv::waitKey(0);

            }

            gst_buffer_unmap(buffer, &mapInfo);
        }
        gst_sample_unref(sample);
    }

//    delete inData;
    return GST_FLOW_OK;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    gst_init(&argc, &argv);

    struct data_* data = new struct data_();

    data->source = gst_element_factory_make("videotestsrc", "source");
    data->converter = gst_element_factory_make("autovideoconvert", "converter");
    data->sink = gst_element_factory_make("appsink", "sink");
    data->pipeline = gst_pipeline_new("Pipeline");

//    g_object_set(data->source, "pattern", 18, NULL);

    if(!data->source || !data->converter || !data->sink || !data->pipeline) {
        g_printerr("Elements can't create properly...\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(data->pipeline), data->source, data->converter, data->sink, NULL);

    if(!gst_element_link_many(data->source, data->converter, data->sink, NULL)) {
        g_printerr("Elements can't linked properly...\n");
        return -1;
    }

// Default value is FALSE, make it TRUE
    gst_app_sink_set_emit_signals(GST_APP_SINK(data->sink), TRUE);
//    g_signal_connect(data->sink, "new-sample", G_CALLBACK(::newSample), (gpointer)data);
    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
//    std::this_thread::sleep_for(std::chrono::seconds(2));
    for(int kk = 0; kk < 10; ++kk) {
        ::newSample(GST_APP_SINK(data->sink), (gpointer)data);
    }




    delete data;
    return a.exec();
}
