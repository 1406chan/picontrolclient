#ifndef THERMALIMAGEPROVIDER_H
#define THERMALIMAGEPROVIDER_H
#include <QQuickImageProvider>
#include <QNetworkAccessManager>
#include <QUrl>
#include "mjpegimagegrabber.h"

class ThermalImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    ThermalImageProvider(QNetworkAccessManager *nam, QObject *parent = 0);
    virtual QPixmap	requestPixmap(const QString & id, QSize * size,
                                  const QSize & requestedSize);

Q_SIGNALS:
    void refresh();

private Q_SLOTS:
    void onNewThermalUrl(const QUrl &url);
    void onNewPixmap(const QPixmap &pixmap);

private:
    MJPEGImageGrabber *m_mjpegGrabber;
    QNetworkAccessManager *m_nam;
    QPixmap m_currentPixmap;
};

#endif // THERMALIMAGEPROVIDER_H
