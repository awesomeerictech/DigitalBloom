import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Item {
    implicitWidth: 32
    implicitHeight: 32

    property string source
    property string color
    property int fillMode: Image.PreserveAspectFit

    Image {
        id: sourceImg
        anchors.fill: parent
        visible: (parent.visible && parent.color) ? false : true

        source: parent.source
        sourceSize: Qt.size(width, height)
        fillMode: parent.fillMode
    }
    ColorOverlay {
        source: sourceImg
        anchors.fill: parent
        visible: (parent.visible && parent.color) ? true : false

        color: parent.color
        cached: visible
    }
}
