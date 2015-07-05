#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QGst/Init>
#include <QGst/Quick/VideoSurface>
#include "GStreamerPlayer.h"
#include "pidiscoverer.h"
#include "nodeselector.h"
#include "thermalimageprovider.h"

int main(int argc, char *argv[])
{
#if defined(QTVIDEOSINK_PATH)
    //this allows the example to run from the QtGStreamer build tree without installing QtGStreamer
    qputenv("GST_PLUGIN_PATH", QTVIDEOSINK_PATH);
#endif

    QGuiApplication app(argc, argv);
    QGst::init(&argc, &argv);

    PiDiscoverer discoverer;

    QQuickView view;

    QGst::Quick::VideoSurface *surface1 = new QGst::Quick::VideoSurface;
    view.rootContext()->setContextProperty(QLatin1String("videoSurface1"), surface1);

    GStreamerPlayer *player1 = new GStreamerPlayer(&view);
    player1->setVideoSink(surface1->videoSink());
    view.rootContext()->setContextProperty(QLatin1String("player1"), player1);

    QNetworkAccessManager *nam = new QNetworkAccessManager;
    NodeSelector nodeSelector(&discoverer, nam, player1);
    QObject::connect(&discoverer, SIGNAL(nodeDiscovered(QSharedPointer<PiNode>)),
                     &nodeSelector, SLOT(checkPlayback()));

    view.rootContext()->setContextProperty(QLatin1String("nodeSelector"), &nodeSelector);

    ThermalImageProvider *thermalImageProvider = new ThermalImageProvider(nam);
    QObject::connect(&nodeSelector, SIGNAL(thermalUrl(QUrl)),
                     thermalImageProvider, SLOT(onNewThermalUrl(QUrl)));
    view.rootContext()->setContextProperty(QLatin1String("thermal"), thermalImageProvider);
    view.engine()->addImageProvider(QLatin1String("theramlprovider"), thermalImageProvider);


#if defined(UNINSTALLED_IMPORTS_DIR)
    //this allows the example to run from the QtGStreamer build tree without installing QtGStreamer
    view.engine()->addImportPath(QLatin1String(UNINSTALLED_IMPORTS_DIR));
#endif

    view.setSource(QUrl(QLatin1String("qrc:///main.qml")));
    view.show();

    return app.exec();
}
