import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtGStreamer 1.0


Rectangle {
    width: 1280
    height: 720
    visible: true

    VideoItem {
        id: video1
        width: parent.width
        height: parent.height
        surface: videoSurface1 //bound on the context from main()
    }

    Button {
        id : right
        iconName : "ArrowForward"
        anchors.verticalCenter: parent.verticalCenter
        x: parent.width - 150
        onClicked: nodeSelector.selectNext()
    }

    Button {
        id: left
        iconName: "ArrowBackward"
        anchors.verticalCenter: parent.verticalCenter
        x: 50
        onClicked: nodeSelector.selectPrevious()
    }

    Image {
        id: thermalImage
        source: "image://theramlprovider/pixmap"
        cache: false
        anchors.centerIn: parent
        width: 80
        height: 60
    }

    Connections {
        target: thermal
        onRefresh: { thermalImage.source = " "; thermalImage.source = "image://theramlprovider/pixmap" }
    }


}
