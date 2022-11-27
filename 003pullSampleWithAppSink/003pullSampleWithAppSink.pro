QT += gui core

CONFIG += c++2a console
CONFIG -= app_bundle

linux {

INCLUDEPATH += /usr/include/gstreamer-1.0 \
               /usr/include/glib-2.0  \
               /usr/lib/x86_64-linux-gnu/glib-2.0/include \
               /usr/include/opencv4 \

LIBS += -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core -ldl
LIBS += -lavcodec -lavformat -lavutil -lswscale -lgstrtp-1.0 -lgstrtspserver-1.0
LIBS += -lpthread -lgstvideo-1.0 -lgstbase-1.0 -lgstapp-1.0 -lgstreamer-1.0 -lgstrtsp-1.0 -lgobject-2.0 -lglib-2.0 -lgstrtsp-1.0
LIBS += -lgstwebrtc-1.0 -lgstsdp-1.0 -ljson-glib-1.0 -lsoup-2.4 -lgio-2.0

}

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
