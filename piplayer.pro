TEMPLATE = app

QT += qml quick widgets opengl

SOURCES += main.cpp \
    GStreamerPlayer.cpp \
    pidiscoverer.cpp \
    nodeselector.cpp \
    mjpegimagegrabber.cpp \
    thermalimageprovider.cpp

RESOURCES += qml.qrc

# Tell qmake to use pkg-config to find QtGStreamer.
CONFIG += link_pkgconfig

# Now tell qmake to link to QtGStreamer and also use its include path and Cflags.
PKGCONFIG += Qt5GStreamerQuick-1.0 Qt5GStreamer-1.0

INCLUDEPATH+=/usr/include/Qt5GStreamer/

# Recommended if you are using g++ 4.5 or later. Must be removed for other compilers.
QMAKE_CXXFLAGS += -std=c++0x

# Recommended, to avoid possible issues with the "Q_EMIT" keyword
# You can otherwise also define QT_NO_Q_EMIT, but notice that this is not a documented Qt macro.
DEFINES += QT_NO_KEYWORDS

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES +=

HEADERS += \
    GStreamerPlayer.h \
    pidiscoverer.h \
    nodeselector.h \
    mjpegimagegrabber.h \
    thermalimageprovider.h
