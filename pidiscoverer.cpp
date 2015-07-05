#include "pidiscoverer.h"

PiNode::PiNode(QObject *parent) : QObject(parent)
{
    caps = PiNode::None;
}

PiDiscoverer::PiDiscoverer(QObject *parent) : QObject(parent)
{
    m_socket.bind(31311);
    connect(&m_socket, SIGNAL(readyRead()), SLOT(datagramReceived()));
}

void PiDiscoverer::datagramReceived()
{

    qint64 datagramSize = m_socket.pendingDatagramSize();
    QByteArray datagram;
    datagram.resize(datagramSize);
    QHostAddress addr;
    m_socket.readDatagram(datagram.data(), datagramSize, &addr);

    qDebug() << "data received " << datagram.data();
    PiNodeShared node(new PiNode(this));
    if (datagram.startsWith("raspberry")) {
        if (datagram.contains("picam")) {
            node->caps &= PiNode::PiCam;
        }

        if (datagram.contains("thermal")) {
            node->caps &= PiNode::Thermal;
        }
        node->address = addr;
        node->addressString = addr.toString().split(":").last();
        onNodeDiscovered(node);
        Q_EMIT nodeDiscovered(node);
    }
}

PiNodeList PiDiscoverer::discoveredNodes() const
{
    return m_discoveredNodes;
}

void PiDiscoverer::onNodeDiscovered(PiNodeShared node)
{
   QString completeAddress = node->address.toString();
   QString addr = completeAddress.split(":").last();
    qDebug() << "node discovered " << addr;
    if (!m_discoveredNodes.contains(node)) {
        qDebug() << "new unique node adding";
        m_discoveredNodes << node;
    }
}
