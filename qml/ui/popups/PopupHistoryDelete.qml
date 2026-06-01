import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ComponentLibrary

Popup {
    id: popupChoice

    x: singleColumn ? 0 : (appWindow.width / 2) - (width / 2)
    y: singleColumn ? (appWindow.height - height)
                    : ((appWindow.height / 2) - (height / 2))

    width: singleColumn ? appWindow.width : 720
    height: columnContent.height + padding*2 + screenPaddingNavbar + screenPaddingBottom
    padding: EricTheme.componentMarginXL
    margins: 0

    dim: true
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    signal confirmed()

    ////////////////////////////////////////////////////////////////////////////

    enter: Transition { NumberAnimation { property: "opacity"; from: 0.5; to: 1.0; duration: 133; } }
    //exit: Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; } }

    Overlay.modal: Rectangle {
        color: "#000"
        opacity: EricTheme.isLight ? 0.24 : 0.48
    }

    background: Rectangle {
        color: EricTheme.colorBackground
        border.color: EricTheme.colorSeparator
        border.width: singleColumn ? 0 : EricTheme.componentBorderWidth
        radius: singleColumn ? 0 : EricTheme.componentRadius

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: EricTheme.componentBorderWidth
            visible: singleColumn
            color: EricTheme.colorSeparator
        }

        layer.enabled: !singleColumn
        layer.effect: MultiEffect {
            autoPaddingEnabled: true
            shadowEnabled: true
            shadowColor: EricTheme.isLight ? "#aa000000" : "#aaffffff"
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Item {
        Column {
            id: columnContent
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: EricTheme.componentMarginXL

            ////////

            Text {
                anchors.left: parent.left
                anchors.right: parent.right

                text: qsTr("Are you sure you want to delete this entry?")
                textFormat: Text.PlainText
                font.pixelSize: EricTheme.fontSizeContentVeryBig
                color: EricTheme.colorText
                wrapMode: Text.WordWrap
            }

            ////////

            Text {
                anchors.left: parent.left
                anchors.right: parent.right

                text: qsTr("This entry will be deleted from the barcode history.")
                textFormat: Text.PlainText
                font.pixelSize: EricTheme.fontSizeContent
                color: EricTheme.colorSubText
                wrapMode: Text.WordWrap
            }

            ////////

            Flow {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: EricTheme.componentMargin

                property int btnSize: singleColumn ? width : ((width-spacing) / 2)

                ButtonClear {
                    width: parent.btnSize
                    color: EricTheme.colorGrey

                    text: qsTr("Cancel")
                    onClicked: popupChoice.close()
                }

                ButtonFlat {
                    width: parent.btnSize
                    color: EricTheme.colorError

                    text: qsTr("Delete entry")
                    onClicked: {
                        popupChoice.confirmed()
                        popupChoice.close()
                    }
                }
            }

            ////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
