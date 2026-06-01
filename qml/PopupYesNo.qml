import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import Qaterial as Qaterial
import ComponentLibrary
import "qrc:/"
import "qrc:/qml"


Popup {
    id: popup
    modal: true
    background: Rectangle {

        anchors.fill: parent
        color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker(EricTheme.colorHeader,"2.5") : Qt.darker("#5483EF","2.5")

    }

    closePolicy: Popup.NoAutoClose
    property alias text: popupLabel.text
    property bool isYes: false
    signal isyesclicked(var x)
    Material.elevation: 8
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    implicitHeight: 200
    implicitWidth: parent.width * .9
    ColumnLayout {
        width: parent.width - 32
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 20
        RowLayout {
            Qaterial.Label {
                id: popupLabel
                font.family: "FuturaPTBook"
                Layout.fillWidth: true
                font.pixelSize: fontSizeSubheading
                opacity: opacitySubheading
                topPadding: 20
                leftPadding: 8
                rightPadding: 8
                text: ""
                color:  "white"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            } // popupLabel
        } // row label
        RowLayout {
            spacing: 20
            Item {
                Layout.preferredWidth: 1
                Layout.fillWidth: true
            }
            Qaterial.FlatButton {
                id: noButton
                focusPolicy: Qt.NoFocus
                Layout.fillWidth: true
                font.family: "FuturaPTBook"
                font.pixelSize: 20
                text: qsTr("No")
                backgroundColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricSettings.maroonFlush :  "teal"
                onClicked: {
                    popup.isYes = false
                    popup.isyesclicked(0)
                    popup.close()
                }
            }
            Qaterial.FlatButton {
                id: yesButton
                focusPolicy: Qt.NoFocus
                Layout.fillWidth: true
                font.family: "FuturaPTBook"
                font.pixelSize: 20
                text: qsTr("Yes")
                backgroundColor: Qaterial.Style.primaryColor
                onClicked: {
                    popup.isYes = true
                    popup.isyesclicked(1)
                    popup.close()
                }
            }
        } // row layout
    }


    onOpened: {
        appWindow.modalPopupActive = true

    }
    onClosed: {
        appWindow.modalPopupActive = false
    }
}
