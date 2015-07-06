#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include "pidiscoverer.h"
#include "GStreamerPlayer.h"

#define playCommand "udpsrc port=$PORT ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! h264parse ! avdec_h264"
#define serverCommand "raspivid -t 0 -h $RESh -w $RESw -fps 49 -b 2000000 -o - | gst-launch-1.0 -v fdsrc ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=$CLIENT_IP port=$UDP_PORT"

class NodeSelector : public QObject
{
    Q_OBJECT
public:
    explicit NodeSelector(PiDiscoverer *discoverer, QNetworkAccessManager *nam,
                          GStreamerPlayer *player, QObject *parent = 0);
    virtual ~NodeSelector();
    QString deviceAddress() const;

Q_SIGNALS:
    void thermalUrl(const QUrl &thermalUrl);

public Q_SLOTS:
    void selectNext();
    void selectPrevious();
    void play();
    void checkPlayback();

private Q_SLOTS:
    void replyFinished();

private:
    PiDiscoverer *m_discoverer;
    GStreamerPlayer *m_player;
    int m_currentIndex;
    const QString m_playCommand, m_serverCommand;
    QNetworkAccessManager *m_nam;
    QList<QString> m_ports;
};

#endif // NODESELECTOR_H
