import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects


Button {
    id: button
    // image should be 24x24
    property alias imageSource: contentImage.source
    property alias backgroundColor: buttonBackground.color
    property bool showShadow: true
    focusPolicy: Qt.NoFocus
    contentItem:
        Item {
        implicitHeight: 24
        implicitWidth: 24
        ImageSvg {
            id: contentImage
            source: "qrc:/assets/icons_material/baseline-refresh-24px.svg"
            anchors.centerIn: parent
        }
    }
    background:
        Rectangle {
        id: buttonBackground
        implicitWidth: 56
        implicitHeight: 56
        radius: width / 2
        opacity: button.pressed ? 0.75 : 1.0
        layer.enabled: button.showShadow
        layer.effect: DropShadow {
            verticalOffset: 3
            horizontalOffset: 1
            color: "grey"
         //   samples: button.pressed ? 20 : 10
            spread: 0.5
        }
    }
}
