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
    implicitWidth: isLandscape? parent.width * 0.50 : parent.width * 0.80

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    background: Rectangle{
        color: Material.color(Material.Red, isDarkTheme? Material.Shade500 : Material.Shade800)
        radius: 24
        opacity: toastOpacity
    }
    Timer {
        id: errorTimer
        interval: 5000
        repeat: false
        onTriggered: {
            popup.close()
        }
    } // toastTimer
    ColumnLayout {
        width: parent.width - 32
        Layout.fillWidth: true
        Layout.leftMargin: 16
        Layout.rightMargin: 16
        RowLayout {
            Layout.fillWidth: true
            spacing: 16



            Qaterial.ColorIcon
              {


                color:
                  {
                Qaterial.Style.accentColor
                }
            source: Qaterial.Icons.alertCircle

              } // ColorIcon

            Label {
                id: errorLabel
                font.family: "FuturaPTBook"
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                rightPadding: 1
                font.pixelSize: 18
                color: "white"
                wrapMode: Label.WordWrap
            }
        }
    }




    onAboutToShow: {
        errorTimer.start()
    }
    function start(errorText) {
        errorLabel.text = errorText
        if(!errorTimer.running) {
            open()
        } else {
            errorTimer.restart()
        }
    } // function start
} // popup errorPopup
