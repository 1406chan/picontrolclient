#ifndef MJPEGIMAGEGRABBER_H
#define MJPEGIMAGEGRABBER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QPixmap>
#include <QTimer>

class MJPEGImageGrabber : public QObject
{
    Q_OBJECT
public:
    explicit MJPEGImageGrabber(QNetworkAccessManager *nam, const QUrl &url, QObject *parent = 0);

Q_SIGNALS:
    void newFrame(const QPixmap &pixmap);
    void newFrameData(const QString &data);
    void finished(const QString &errorString);

private Q_SLOTS:
    void onFPSTImer();
    void onReadyRead();
    void onFinished();
    void onError(QNetworkReply::NetworkError);

private:
    void processData(const QByteArray &data);

    QNetworkReply                   *m_reply;
    QNetworkAccessManager           *m_nam;
    bool                             m_imageStarted;
    QByteArray                       m_imageData;
    QUrl                             m_mjpegStreamUrl;
    int                              m_fps;
    QTimer                           *m_timer;

};

#endif // MJPEGIMAGEGRABBER_H
