import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import Qaterial as Qaterial
import ComponentLibrary
import "qrc:/"
import "qrc:/qml"




Popup {
    id: updatePopup
    background: Rectangle {

        anchors.fill: parent
        color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker(EricTheme.colorHeader,"2.5") : Qt.darker("#5483EF","2.5")

    }

    modal: true
    closePolicy: Popup.NoAutoClose
    property alias text: popupLabel.text
    property bool isUpdate: true
    property bool buttonsVisible: true
    property bool showUpdateButton: true
    Material.elevation: 8
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    implicitHeight: 160
    implicitWidth: parent.width * .9
    ColumnLayout {
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 20
        RowLayout {
            Qaterial.Label {
                id: popupLabel
                font.family: "FuturaPTBook"
                Layout.fillWidth: true
                font.pixelSize: fontSizeTitle
                opacity: opacityTitle
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
            visible: updatePopup.buttonsVisible
            spacing: 20
            Item {
                Layout.preferredWidth: 1
                Layout.fillWidth: true
            }
            Qaterial.FlatButton {
                focusPolicy: Qt.NoFocus
                Layout.fillWidth: true
                font.family: "FuturaPTBook"
                font.pixelSize: 20
                text: updatePopup.showUpdateButton? qsTr("Cancel") : qsTr("OK")
                backgroundColor: Qt.darker(accentColor,"1.5")
                onClicked: {
                    updatePopup.isUpdate = false
                    updatePopup.close()
                }
            }
            Qaterial.FlatButton {
                focusPolicy: Qt.NoFocus
                Layout.fillWidth: true
                font.family: "FuturaPTBook"
                font.pixelSize: 20
                visible: updatePopup.showUpdateButton
                text: qsTr("Update")
                backgroundColor: primaryColor
                onClicked: {
                    updatePopup.isUpdate = true
                    updatePopup.close()
                }
            }

        } // row layout

    }
}
