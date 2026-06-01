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
    closePolicy: Popup.NoAutoClose
    bottomMargin: isLandscape? 24 : 80
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    background: Rectangle{
        color: toastColor
        radius: 24
        opacity: toastOpacity
    }
    Timer {
        id: toastTimer
        interval: 3000
        repeat: false
        onTriggered: {
            popup.close()
        }
    } // toastTimer
    Label {
        id: toastLabel
        font.family: "FuturaPTBook"
        width: parent.width
        leftPadding: 16
        rightPadding: 16
        font.pixelSize: 18
        color: "white"
      //  color: Qaterial.Style.textColor
        wrapMode: Label.WordWrap
    } // toastLabel
    onAboutToShow: {
        toastTimer.start()
    }
    function start(toastText) {
        toastLabel.text = toastText
        if(!toastTimer.running) {
            open()
        } else {
            toastTimer.restart()
        }
    } // function start
} // popup toastPopup
