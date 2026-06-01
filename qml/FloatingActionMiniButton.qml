
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial


Button {
    id: button
    // image should be 24x24
    property alias imageSource: contentImage.source
    // default: primaryColor
    property alias backgroundColor: buttonBackground.color
    property bool showShadow: false
    focusPolicy: Qt.NoFocus
    contentItem:
        Item {
        implicitHeight: 24
        implicitWidth: 24

        Qaterial.ColorIcon
             {

                 id: contentImage
                 anchors.centerIn: parent
                 color:
                 {
                   "white"
                 }

             }



    }
    background:
        Rectangle {
        id: buttonBackground
        implicitWidth: 48
        implicitHeight: 48
        color: "purple"
        radius: width / 2
        opacity: button.pressed ? 0.75 : 1.0
        layer.enabled: button.showShadow
        layer.effect: DropShadow {
            verticalOffset: 3
            horizontalOffset: 1
            color: "grey"
            spread: 0.5
        }
    }
}
