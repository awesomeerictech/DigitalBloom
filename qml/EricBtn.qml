import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Button {
    id: control
    property alias label: textLabel.text
    property alias backgroundImageSource: _backgroundImage.source

    contentItem: Label {
        id: textLabel
        color: "red"
        font.pixelSize: 32
        font.letterSpacing: 0.5
        font.family: "FuturaPTBold"
        topPadding: 0
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors.fill: parent
    }

    background: Item {
        Image {
            id: _backgroundImage
            anchors.fill: parent
            anchors.top: parent.top
            fillMode: Image.PreserveAspectCrop
            visible: false
        }

        Desaturate {
            id: _desaturate
            anchors.fill: _backgroundImage
            source: _backgroundImage
            desaturation: enabled ? 0.0 : 1.0
            visible: false
        }

        ColorOverlay {
            id: _colorOverlay
            anchors.fill: _desaturate
            source: _desaturate
            color: enabled ? "#80000000" : "#AA000000"
            visible: false
        }
        OpacityMask {
            anchors.fill: _colorOverlay
            source: _colorOverlay
            maskSource: Rectangle {
                width: _backgroundImage.width
                height: _backgroundImage.height
                radius: 10
            }
        }
        Rectangle {
            anchors.fill: parent
            color: "blue"
            radius: 10
            border.width: 2
            border.color: "white"
        }
    }
}
