#include "mjpegimagegrabber.h"
#include <QDebug>
#include <QFile>
const char streamSperator[] = "--jpgboundary";

MJPEGImageGrabber::MJPEGImageGrabber(QNetworkAccessManager *nam, const QUrl &url, QObject *parent) :
    QObject(parent), m_reply(0), m_nam(nam), m_imageStarted(false), m_mjpegStreamUrl(url), m_fps(0)
{
    qDebug() << "creating image grabber with url : " << url;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Unknown; Linux i686) AppleWebKit/537.21 (KHTML, like Gecko)");
    m_reply = m_nam->get(request);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), SLOT(onFPSTImer()));
    m_timer->start();

    connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
}

void MJPEGImageGrabber::onFPSTImer()
{
//    qDebug() << "FPS" << m_fps;
    m_fps = 0;
}


void MJPEGImageGrabber::onReadyRead()
{
//    qDebug() << "got ready read";
    m_imageData.append(m_reply->readAll());
//    qDebug() << "number of bytes in imageData" << m_imageData.size();

    QByteArray marker(streamSperator, strlen(streamSperator));
//    qDebug() << "Marker " << marker;

    if (!m_imageStarted) {
        int index = m_imageData.indexOf(marker);
        if (index < 0) { // current data doesn't have a image start
//            qDebug() << "not started yet, leaving... looked for marker";
            m_imageData.clear();
            return;
        }
        m_imageStarted = true; //started
        m_imageData = m_imageData.mid(index+marker.length()); // discard data before the start
    }

    int index = m_imageData.indexOf(marker);

    if (index < 0) { // no end yet
//        qDebug() << "frame started but no end yet, leaving...";
        return;
    }

    // end reached, get the frame
//    qDebug() << "one frame complete, process it";
    QByteArray fullFrame = m_imageData.left(index);
    Q_EMIT processData(fullFrame);
    m_imageStarted = false;
    m_imageData = m_imageData.mid(index+marker.length()); // discard data upto the extracted frame
}

void MJPEGImageGrabber::processData(const QByteArray &data)
{
    m_fps++;

    QImage image = QImage::fromData(data, "JPG");
    if (image.isNull()) {
        qDebug() << "image is invalid";
        m_imageData.clear();
        return;
    }

    Q_EMIT newFrame(QPixmap::fromImage(image));
}

void MJPEGImageGrabber::onError(QNetworkReply::NetworkError error)
{
    qDebug() << "got error from reply" << error;
}

void MJPEGImageGrabber::onFinished()
{
    qDebug() << "got finished signal from network" << m_reply->error() << " " << m_reply->errorString();
    Q_EMIT finished(m_reply->errorString());
    m_reply->deleteLater();
}




