#include "nodeselector.h"
#include <QNetworkInterface>
#include <QNetworkRequest>

NodeSelector::NodeSelector(PiDiscoverer *discoverer, QNetworkAccessManager *nam, GStreamerPlayer *player, QObject *parent) :
    m_discoverer(discoverer),
    m_player(player),
    m_currentIndex(0),
    m_playCommand(playCommand),
    m_serverCommand(serverCommand),
    m_nam(nam),
    QObject(parent)
{
    m_ports << "5003" << "5004" << "5005";
}

NodeSelector::~NodeSelector()
{
    PiNodeList nodes = m_discoverer->discoveredNodes();

    //terminate all nodes
    Q_FOREACH(QSharedPointer<PiNode> node, nodes) {
            QString addr = node->address.toString().split(":").last();
            QUrl terminateUrl("http://" + addr + ":8080/picam/?command=terminate");
            m_nam->get(QNetworkRequest(terminateUrl));
            if (node->caps.testFlag(PiNode::Thermal)) {
                qDebug() << "termiante thermal node as well";
                terminateUrl = QUrl("http://" + addr + ":8080/thermal/?command=terminate");
                m_nam->get(QNetworkRequest(terminateUrl));
            }
        }
}

void NodeSelector::selectNext()
{
    PiNodeList nodes = m_discoverer->discoveredNodes();

    if (!nodes.size()) {
        qDebug() << __FUNCTION__ << "nodes are empty";
        return;
    }

    m_currentIndex = ++m_currentIndex%nodes.size();
    play();
}

void NodeSelector::selectPrevious()
{
    PiNodeList nodes = m_discoverer->discoveredNodes();

    if (!nodes.size()) {
        return;
    }

    m_currentIndex = --m_currentIndex%nodes.size();
    play();
}

void NodeSelector::checkPlayback()
{
    PiNodeList nodes = m_discoverer->discoveredNodes();
    if(nodes.size() && m_player->pipelineString().isEmpty()) {
        play();
    }
    if (nodes.size()) {
        Q_FOREACH(QSharedPointer<PiNode> node, nodes) {
            if (node->caps.testFlag(PiNode::Thermal)) {
                QString addr = node->address.toString().split(":").last();
                QUrl startUrl("http://" + addr + ":8080/thermal/?command=start");
                qDebug() << "thermal server start url " << startUrl;
                QUrl mjpegUrl("http://"+ addr + ":5002/cam.mjpeg");
                qDebug() << "mjpeg server start url " << mjpegUrl;
                m_nam->get(QNetworkRequest(startUrl));
                Q_EMIT thermalUrl(mjpegUrl);
            }
        }
    }
}

void NodeSelector::play()
{
    PiNodeList nodes = m_discoverer->discoveredNodes();
    if (!nodes.size()) {
        return;
    }
    qDebug() << "currentIndex is " << m_currentIndex << "among a total of " << nodes.size();

    QString servercmd = "http://$SERVER_IP:8080/picam/?command=" + m_serverCommand;
    PiNodeShared node = nodes.at(m_currentIndex);
    servercmd = servercmd.replace("$SERVER_IP", node->address.toString().split(":").last());
    servercmd = servercmd.replace("$CLIENT_IP", deviceAddress());
    servercmd = servercmd.replace("$UDP_PORT", m_ports.at(m_currentIndex));
    servercmd = servercmd.replace("$RESw", "1280");
    servercmd = servercmd.replace("$RESh", "720");

    qDebug() << "SERVER CMD: " << servercmd;
    m_nam->get(QNetworkRequest(servercmd));
    QString playcmd = m_playCommand;
    playcmd = playcmd.replace("$PORT", m_ports.at(m_currentIndex));
    qDebug() << "CLIENT CMD: " << playcmd;
    m_player->setPipelineString(playcmd);
    m_player->play();
}

QString NodeSelector::deviceAddress() const
{
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    QHostAddress addr;
    Q_FOREACH(QHostAddress address, addresses) {
        if (address.isLoopback()) {
            continue;
        }
        addr = address;
        break;
    }

    QString address = addr.toString();
    qDebug() << "got the following address for the host device" << address;
    return address;
}
