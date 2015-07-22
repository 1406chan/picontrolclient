import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtGStreamer 1.0
import QtQuick.Controls.Styles 1.3


Rectangle {
    width: 1280
    height: 720
    visible: true
    anchors.centerIn: parent
    color: "black"

    VideoItem {
        id: video1
        width: parent.width
        height: parent.height
        surface: videoSurface1 //bound on the context from main()
    }

    Button {
        id : right
        iconSource: "images/back.png"
//        opacity: 0
        rotation: 180
        anchors.verticalCenter: parent.verticalCenter
        x: parent.width - 150
        onClicked: nodeSelector.selectNext()
        style: ButtonStyle {
                background: Rectangle {
                         color:"transparent"
                        }
            }
    }

    Button {
        id: left
        iconSource: "images/back.png"
        anchors.verticalCenter: parent.verticalCenter
        x: 50
        onClicked: nodeSelector.selectPrevious()
        style: ButtonStyle {
                background: Rectangle {
                         color:"transparent"
                        }
            }
    }

    Image {
        id: thermalImage
        property real thermalfovfactor: 0.5
        property int imageid: 0
        source: "image://theramlprovider/pixmap"
        cache: false
        anchors.centerIn: parent
        width: parent.width*thermalfovfactor
        height: parent.height*thermalfovfactor
    }

    Connections {
        target: thermal
        onRefresh: { thermalImage.imageid++;
                     thermalImage.source = "image://theramlprovider/"+thermalImage.imageid }
    }




}
