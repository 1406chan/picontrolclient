#include "nodeselector.h"
#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QNetworkReply>

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
    Q_FOREACH(PiNode node, nodes) {
            QUrl terminateUrl("http://" + node.addressString + ":8080/picam/?command=terminate");
            m_nam->get(QNetworkRequest(terminateUrl));
            if (node.caps & PiNode::Thermal) {
                qDebug() << "termiante thermal node as well";
                terminateUrl = QUrl("http://" + node.addressString + ":8080/thermalcam/?command=terminate");
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

    m_currentIndex = --m_currentIndex < 0 ? nodes.size()-1 : m_currentIndex;
    play();
}

void NodeSelector::checkPlayback()
{
    PiNodeList nodes = m_discoverer->discoveredNodes();
    if(nodes.size() && m_player->pipelineString().isEmpty()) {
        play();
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
    PiNode node = nodes.at(m_currentIndex);
    servercmd = servercmd.replace("$SERVER_IP", node.addressString);
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

    // check if its thermal module, if so start thermal
    if (node.caps & PiNode::Thermal) {
        QUrl startUrl("http://" + node.addressString + ":8080/thermalcam/?command=start");
        qDebug() << "thermal server start url " << startUrl;
        QUrl mjpegUrl("http://"+ node.addressString + ":5002/cam.mjpg");
        qDebug() << "mjpeg server start url " << mjpegUrl;
        QNetworkReply *reply = m_nam->get(QNetworkRequest(startUrl));
        reply->setProperty("camUrl", mjpegUrl);
        connect(reply, SIGNAL(finished()), SLOT(replyFinished()));
    } else {
        // hide thermal
        Q_EMIT thermalUrl(QUrl());
    }
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

void NodeSelector::replyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "thermal camera started without any error";
        QUrl mjpegUrl = reply->property("camUrl").toUrl();
        Q_EMIT thermalUrl(mjpegUrl);
    }
    reply->deleteLater();
}
