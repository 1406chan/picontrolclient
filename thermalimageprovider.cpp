#include "thermalimageprovider.h"

ThermalImageProvider::ThermalImageProvider(QNetworkAccessManager *nam, QObject *parent) :
    QObject(parent), QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    m_mjpegGrabber = 0;
    m_nam = nam;
}

void ThermalImageProvider::onNewPixmap(const QPixmap &pixmap)
{
    m_currentPixmap = pixmap;
    qDebug() << "new thermal image";
    Q_EMIT refresh();
}

void ThermalImageProvider::onNewThermalUrl(const QUrl &url)
{
    if (m_mjpegGrabber) {
        m_mjpegGrabber->deleteLater();
    }
    m_mjpegGrabber = new MJPEGImageGrabber(m_nam, url, this);
    connect(m_mjpegGrabber, SIGNAL(newFrame(QPixmap &pixmap)),
            this, SLOT(onNewPixmap(QPixmap &pixmap)));
}

QPixmap	ThermalImageProvider::requestPixmap(const QString & id, QSize * size, const QSize & requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(requestedSize)

    *size = m_currentPixmap.size();
    return m_currentPixmap;
}


