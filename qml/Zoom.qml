import QtQuick 2.15
import QtQuick.Controls.Material
import Qt.labs.folderlistmodel
import QtQml
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "qrc:/"
import "qrc:/qml"
import Quickeric 1.0



Page {
    id: imgDetailPage
    focus: true
    property string pagename : "Zoom Page"

    Component.onCompleted: {

        starttimer.start()


    }


    Timer {

        id: starttimer
        interval: 1000
        repeat: false

        onTriggered: {

            fitIntoWindow()

        }


    }



    Flickable {
        id: flickable
        property string pagename: "imgDetail"
        contentHeight: itemImage.sourceSize.height * itemImage.scale + 60
        contentWidth: itemImage.sourceSize.width * itemImage.scale
        anchors.fill: parent

            Image {
                id: itemImage
                anchors.top: parent.top
                anchors.left: parent.left
                width: sourceSize.width
                asynchronous: true
                height: sourceSize.height
                fillMode: Image.PreserveAspectFit
               // source: jsontools.medialink()+ericsettings.read("image")
                source: "qrc:/images/materialbg.png"
                horizontalAlignment: Image.AlignLeft
                verticalAlignment: Image.AlignTop
                transformOrigin: Item.TopLeft
                transform: Translate { y: 6; x:6 }
            } // image


    } // flickable

    FloatingActionMiniButton  {
        visible: itemImage.scale >= 0.2
        z: 1
        anchors.leftMargin: 20
        anchors.bottomMargin: 10
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        imageSource: Qaterial.Icons.minus
        backgroundColor: "#5483EF"
        showShadow: true
        onClicked: {
            itemImage.scale = itemImage.scale - 0.2
        }
    } // FAB



    FloatingActionMiniButton {
        z: 1
        anchors.leftMargin: 80
        anchors.bottomMargin: 10
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        imageSource: Qaterial.Icons.aspectRatio
        backgroundColor: "#5483EF"
        showShadow: true
        onClicked: {
            fitIntoWindow()
        }
    } // FAB
    FloatingActionMiniButton {
        visible: itemImage.scale <= 1.0
        z: 1
        anchors.leftMargin: 140
        anchors.bottomMargin: 10
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        imageSource: Qaterial.Icons.plus
        backgroundColor: "#5483EF"
        showShadow: true
        onClicked: {
            itemImage.scale = itemImage.scale + 0.2
        }
    } // FAB

    function fitIntoWindow() {


        var widthScale = (app.safeWidth-20) / itemImage.sourceSize.width
        var heightScale = (app.safeHeight-20) / itemImage.sourceSize.height
        itemImage.scale = Math.min(widthScale, heightScale)
        flickable.contentX = 0
        flickable.contentY = 0
    }

    Component.onDestruction: {

    }



} // ImgDetailPage
