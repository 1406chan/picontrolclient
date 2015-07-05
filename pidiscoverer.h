#ifndef PIDISCOVERER_H
#define PIDISCOVERER_H

#include <QObject>
#include <QtNetwork/QUdpSocket>
#include <QSharedPointer>

class PiNode: public QObject
{
    Q_OBJECT
public:
    PiNode(QObject *parent = 0);

    enum NodeCap {
        None = 0x00,
        PiCam = 0x01,
        Thermal = 0x10
    };

    Q_DECLARE_FLAGS(NodeCaps, NodeCap)
    Q_FLAG(NodeCaps)

    NodeCaps caps;
    QHostAddress address;
    QString addressString;
};

#define PiNodeShared QSharedPointer<PiNode>
#define PiNodeList QList<PiNodeShared>

class PiDiscoverer : public QObject
{
    Q_OBJECT
public:
    explicit PiDiscoverer(QObject *parent = 0);
    PiNodeList discoveredNodes() const;

Q_SIGNALS:
    void nodeDiscovered(PiNodeShared node);

private Q_SLOTS:
    void datagramReceived();
    void onNodeDiscovered(PiNodeShared node);

private:
    QUdpSocket m_socket;
    PiNodeList m_discoveredNodes;
};

#endif // PIDISCOVERER_H
