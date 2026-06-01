import QtQuick
import QtQuick.Controls

import ComponentLibrary

Popup {
    id: popupBarcodeFullscreen
    x: 0
    y: 0

    width: appWindow.width
    height: appWindow.height
    margins: 0
    padding: 0

    dim: false
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    property string barcode_string
    property string full_url

    ////////

    background: Rectangle {
        anchors.fill: parent
        color: "white"
    }

    ////////

    RoundButtonIcon {
        anchors.top: parent.top
        anchors.topMargin: screenPaddingStatusbar
        anchors.left: parent.left
        anchors.leftMargin: 4

        width: appHeader.headerHeight
        height: appHeader.headerHeight

        source: "qrc:/IconLibrary/material-symbols/arrow_back.svg"
        sourceSize: 28
        iconColor: EricTheme.colorHeaderContent

        onClicked: popupBarcodeFullscreen.close()
    }

    ////////

    Image {
        anchors.centerIn: parent

        width: parent.width - 64 - screenPaddingLeft - screenPaddingRight
        height: width

        cache: false
        sourceSize.width: width
        sourceSize.height: width
        fillMode: Image.PreserveAspectFit

        source: popupBarcodeFullscreen.barcode_string

        MouseArea {
            id: zzz
            anchors.fill: parent
            anchors.margins: 0

            clip: true
            enabled: true
            visible: true
            hoverEnabled: false
            acceptedButtons: Qt.LeftButton



            onClicked: {

                Qt.openUrlExternally(popupBarcodeFullscreen.full_url)
            }

            onPressed: {

            }
            onReleased: {

            }
            onCanceled: {

            }

            Rectangle {
                id: mouseBackground
                width: 0; height: width; radius: width;
                x: zzz.mouseX + 4 - (mouseBackground.width / 2)
                y: zzz.mouseY + 4 - (mouseBackground.width / 2)
                color: "#333"
                opacity: 0
                Behavior on opacity { NumberAnimation { duration: 333 } }
                Behavior on width { NumberAnimation { duration: 333 } }
            }

            layer.enabled: true

        }


    }





    ////////
}
