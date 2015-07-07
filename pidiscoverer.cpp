#include "pidiscoverer.h"

PiNode::PiNode()
{
    caps        = PiNode::None;
    capsRunning = PiNode::None;
}

bool PiNode::operator ==(const PiNode &node) const
{
    return (this->addressString == node.addressString);
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
    PiNode node;
    if (datagram.startsWith("raspberry")) {
        if (datagram.contains("picam")) {
            node.caps |= PiNode::PiCam;
        }

        if (datagram.contains("thermal")) {
            node.caps |= PiNode::Thermal;
        }

        if (datagram.contains("mavproxy")) {
            node.caps |= PiNode::MAVProxy;
        }
        node.address = addr;
        node.addressString = addr.toString().split(":").last();
        onNodeDiscovered(node);
        Q_EMIT nodeDiscovered(node);
    }
}

PiNodeList PiDiscoverer::discoveredNodes() const
{
    return m_discoveredNodes;
}

void PiDiscoverer::onNodeDiscovered(const PiNode &node)
{
    qDebug() << "node discovered " << node.addressString;
    if (!m_discoveredNodes.contains(node)) {
        qDebug() << "new unique node adding";
        m_discoveredNodes << node;
    }
}
