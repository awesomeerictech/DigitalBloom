import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates  as T

T.PageIndicator   {
    id: control

    property string mainColor;
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 6
    spacing: 6

    delegate: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        y: 1
        radius: width / 2
        color: "#8B8789"
    }

    contentItem: Item {
        id: _item

        implicitWidth: row.width
        implicitHeight: row.height

        property Item currentItem: _repeater.itemAt(control.currentIndex)
        property real currentX: currentItem ? currentItem.x : 0

        Row {
            id: row
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            spacing: control.spacing

            Repeater {
                id: _repeater
                model: control.count
                delegate: control.delegate
            }
        }

        Rectangle {
            width: 8
            height: 8
            radius: width / 2
            color: control.mainColor

            x: _item.currentX

            Behavior on x {
                NumberAnimation { duration: 200 }
            }
        }
    }
}
