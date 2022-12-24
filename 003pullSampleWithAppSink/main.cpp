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
    GstElement* rtspsrc;
    GstElement* decodebin;
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

void first() {

}

GstFlowReturn newSample(GstAppSink* sink, gpointer data) {
    GstSample* sample;
    GstBuffer* buffer;
    GstMapInfo mapInfo;
    struct data_* inData = static_cast<struct data_*>(data);

//    sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    sample = ::gst_app_sink_try_pull_sample(GST_APP_SINK(sink), GST_SECOND * 10);
    if(sample) {
        std::cout << "Sample TRUE\n";
        buffer = gst_sample_get_buffer(sample);
        if(buffer) {
            std::cout << "Buffer TRUE\n";
            gst_buffer_map(buffer, &mapInfo, GST_MAP_READ);
            QImage image = QImage(mapInfo.data, 640, 360, QImage::Format_RGB888).copy();
            image.save("dalga.jpg", "JPG");
//            cv::Mat mat = ::QImageToMat(image);
//            cv::imwrite("as.jpg", mat);

//            if(mat.size) {
//                std::cout << "Hello\n";
//                cv::imshow("Window", mat);
//                cv::waitKey(0);

//            }

            gst_buffer_unmap(buffer, &mapInfo);
        }
        gst_sample_unref(sample);
    } else {
        std::cout << "sample NULL\n";
    }

//    delete inData;
    return GST_FLOW_OK;
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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    gst_init(&argc, &argv);

    struct data_* data = new struct data_();

    data->rtspsrc = gst_element_factory_make("rtspsrc", "source");
    data->decodebin = gst_element_factory_make ("decodebin", "decodebin");
    data->converter = gst_element_factory_make("autovideoconvert", "converter");
    data->sink = gst_element_factory_make("appsink", "sink");
    data->pipeline = gst_pipeline_new("Pipeline");

//    g_object_set(data->source, "pattern", 18, NULL);
    g_object_set(data->sink, "sync", (gboolean)true, NULL);

    if(!data->rtspsrc || !data->decodebin || !data->converter || !data->sink || !data->pipeline) {
        g_printerr("Elements can't create properly...\n");
        return -1;
    }
    g_object_set(data->rtspsrc, "location", "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4", NULL);

    gst_bin_add_many(GST_BIN(data->pipeline), data->rtspsrc, data->decodebin, data->converter, data->sink, NULL);

    if(!gst_element_link_many(data->converter, data->sink, NULL)) {
        g_printerr("Elements can't linked properly...\n");
        return -1;
    }

    g_signal_connect(data->rtspsrc, "pad-added", G_CALLBACK(::on_pad_added), data->decodebin);
    g_signal_connect(data->decodebin, "pad-added", G_CALLBACK(::on_pad_added), data->converter);


    //Video framelerinin cekilebilecegi yapi, ilk frame cekme islemi yapilir ve video bilgileri alinir

// Default value is FALSE, make it TRUE
    gst_app_sink_set_emit_signals(GST_APP_SINK(data->sink), TRUE);
//    g_signal_connect(data->sink, "new-sample", G_CALLBACK(::newSample), (gpointer)data);
    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
//    std::this_thread::sleep_for(std::chrono::seconds(2));
//    for(int kk = 0; kk < 10; ++kk) {
        ::newSample(GST_APP_SINK(data->sink), (gpointer)data);
//    }



    delete data;
    return a.exec();
}
