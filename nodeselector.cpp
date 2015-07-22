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
    m_mavProxyCommand(mavproxyCommand),
    m_nam(nam),
    QObject(parent)
{
    m_ports << "5003" << "5004" << "5005";
//    auto playcmd = "udpsrc port=9000 ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! h264parse ! avdec_h264";
//    m_player->setPipelineString(playcmd);
//    m_player->play();
}

NodeSelector::~NodeSelector()
{
    PiNodeList nodes = m_discoverer->discoveredNodes();

    //terminate all nodes
    Q_FOREACH(const PiNode &node, nodes) {
            QUrl terminateUrl("http://" + node.addressString + ":8080/picam/?command=terminate");
            sendRequest(terminateUrl);
            if (node.caps & PiNode::Thermal) {
                qDebug() << "termiante thermal node as well";
                terminateUrl = QUrl("http://" + node.addressString + ":8080/thermalcam/?command=terminate");
                sendRequest(terminateUrl);
            }
            if (node.caps & PiNode::MAVProxy) {
                qDebug() << "terminate mavproxy node as well";
                terminateUrl = QUrl("http://" + node.addressString + ":8080/mavproxy/?command=terminate");
                sendRequest(terminateUrl);
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

    const PiNode &node = nodes.at(m_currentIndex);

    if (node.caps & PiNode::PiCam) { // has picam capability
        if (!(node.capsRunning & PiNode::PiCam)) { //picam is not running
            QString servercmd = "http://$SERVER_IP:8080/picam/?command=" + m_serverCommand;
            servercmd = servercmd.replace("$SERVER_IP", node.addressString);
            servercmd = servercmd.replace("$CLIENT_IP", deviceAddress());
            servercmd = servercmd.replace("$UDP_PORT", m_ports.at(m_currentIndex));
            servercmd = servercmd.replace("$RESw", "1280");
            servercmd = servercmd.replace("$RESh", "720");

            qDebug() << "SERVER CMD: " << servercmd;
            QVariantMap map;
            map.insert("requestFor", PiNode::PiCam);
            map.insert("nodeIndex", m_currentIndex);
            sendRequest(servercmd, map);
        }

        // play picam
        QString playcmd = m_playCommand;
        playcmd = playcmd.replace("$PORT", m_ports.at(m_currentIndex));
        qDebug() << "CLIENT CMD: " << playcmd;
        m_player->setPipelineString(playcmd);
        m_player->play();
    }

    if (node.caps & PiNode::MAVProxy) { // has mav capability
        if (!(node.capsRunning & PiNode::MAVProxy)) { // mav proxy is not running
            QString mavcmd = "http://" + node.addressString + ":8080/mavproxy/?command=" + m_mavProxyCommand;
            mavcmd.replace("$CLIENT_IP", deviceAddress());
            qDebug() << "mav command " << mavcmd;
            QVariantMap map;
            map.insert("requestFor", PiNode::MAVProxy);
            map.insert("nodeIndex", m_currentIndex);
            sendRequest(mavcmd, map);
        }
    }

    // check if its thermal module, if so start thermal
    if (node.caps & PiNode::Thermal) {
        QUrl mjpegUrl("http://"+ node.addressString + ":5002/cam.mjpg");
        if (!(node.capsRunning & PiNode::Thermal)) {
            QUrl startUrl("http://" + node.addressString + ":8080/thermalcam/?command=start");
            qDebug() << "thermal server start url " << startUrl;
            qDebug() << "mjpeg server start url " << mjpegUrl;
            QVariantMap map;
            map.insert("requestFor", PiNode::Thermal);
            map.insert("nodeIndex", m_currentIndex);
            map.insert("camUrl", mjpegUrl);
            sendRequest(startUrl, map);
        } else {
            Q_EMIT thermalUrl(mjpegUrl);
        }
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
    reply->deleteLater();
    PiNodeList nodes = m_discoverer->discoveredNodes();
    if (!nodes.size()) {
        return;
    }
    Q_ASSERT(reply);
    if (reply->error() == QNetworkReply::NoError)
    {
        bool ok = false;
        int capibility = reply->property("requestFor").toInt(&ok);
        if (!ok) {
            qDebug() << "generic request return";
            return;
        }

        int index;
        QUrl url;
        switch (capibility) {
        case PiNode::PiCam:
            index = reply->property("nodeIndex").toInt();
            nodes[index].capsRunning |= PiNode::PiCam;
            qDebug() << "picam started without any error";
            break;
        case PiNode::Thermal:
            index = reply->property("nodeIndex").toInt();
            nodes[index].capsRunning |= PiNode::Thermal;
            url = reply->property("camUrl").toUrl();
            Q_EMIT thermalUrl(url);
            qDebug() << "thermal camera started without any error";
            break;
        case PiNode::MAVProxy:
            index = reply->property("nodeIndex").toInt();
            nodes[index].capsRunning |= PiNode::MAVProxy;
            qDebug() << "mavproxy started without any error";
        default:
            break;
        }
    }
}

void NodeSelector::sendRequest(const QUrl &url, const QVariantMap &properties)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = m_nam->get(request);
    Q_FOREACH(const QString &key, properties.keys()) {
        reply->setProperty(key.toStdString().c_str(), properties.value(key));
    }
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}
