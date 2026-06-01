

import QtQuick
import QtQuick.Controls.Material
import Fluid as Fluid

Item {
    Image {
        anchors.centerIn: parent
        source: Qt.resolvedUrl("qrc:/images/balloon.jpg")
        width: 400
        height: 250

        Fluid.Ripple {
            anchors.fill: parent
            onClicked: overlayView.open()
        }
    }

    Fluid.OverlayView {
        id: overlayView

        width: 800
        height: 500

        Image {
            anchors.fill: parent
            source: Qt.resolvedUrl("qrc:/images/balloon.jpg")

            Fluid.Ripple {
                anchors.fill: parent
                onClicked: overlayView.close()
            }
        }

        Row {
            anchors { top: parent.top; right: parent.right; rightMargin: Fluid.Units.largeSpacing }
            height: 48
            spacing: 24

            Repeater {
                model: ["content/add", "image/edit", "action/delete"]
                delegate: Fluid.ToolButton {
                    anchors.verticalCenter: parent.verticalCenter

                    icon.source: Fluid.Utils.iconUrl(modelData)
                    icon.color: Material.iconColor

                    Material.theme: Material.Dark
                }
            }
        }
    }
}
