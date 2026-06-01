import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qaterial as Qaterial
import ComponentLibrary
import "qrc:/"
import "qrc:/qml"



Popup {
    id: popup
    closePolicy: Popup.CloseOnPressOutside
    property alias text: popupLabel.text
    property alias buttonText: okButton.text
    background: Rectangle {

    anchors.fill: parent
    color:  Qt.lighter( Qaterial.Style.accentColor, 0.2)

    }
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    implicitHeight: 250
    implicitWidth: parent.width * .9
    ColumnLayout {
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 10
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
               // color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker("teal", 6.5)  :  Qt.darker("orange",6.5)
                color:  Qt.darker( Qaterial.Style.primaryColor, 0.1)
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            } // popupLabel
        } // row label
        RowLayout {

            Qaterial.FlatButton {
             focusPolicy: Qt.NoFocus
             Layout.fillWidth: true
             font.family: "FuturaPTBook"
             font.pixelSize: 20
             Layout.preferredWidth : 1
             leftPadding: 6
             rightPadding: 6
                id: okButton
                text: ""
                backgroundColor: Qaterial.Style.primaryColor
                onClicked: {
                    popup.close()
                }
            } // okButton
        } // row button
    } // col layout

    onAboutToHide: {
        stopTimer()
    }
    onAboutToShow: {
        closeTimer.start()
    }

    Timer {
        id: closeTimer
        interval: 20000
        repeat: false
        onTriggered: {
            popup.close()
        }
    }
    function stopTimer() {
        closeTimer.stop()
    }
} // popup
