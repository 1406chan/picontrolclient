#ifndef PIDISCOVERER_H
#define PIDISCOVERER_H

#include <QObject>
#include <QtNetwork/QUdpSocket>

class PiNode
{
public:
    PiNode();
    bool operator ==(const PiNode &node) const;

    enum NodeCaps {
        None = 0x00,
        PiCam = 0x01,
        Thermal = 0x10,
        MAVProxy = 0x100
    };

    int caps;
    int capsRunning;
    QHostAddress address;
    QString addressString;
};

#define PiNodeList QList<PiNode>

class PiDiscoverer : public QObject
{
    Q_OBJECT
public:
    explicit PiDiscoverer(QObject *parent = 0);
    PiNodeList discoveredNodes() const;

Q_SIGNALS:
    void nodeDiscovered(const PiNode &node);

private Q_SLOTS:
    void datagramReceived();
    void onNodeDiscovered(const PiNode &node);

private:
    QUdpSocket m_socket;
    PiNodeList m_discoveredNodes;
};

#endif // PIDISCOVERER_H
