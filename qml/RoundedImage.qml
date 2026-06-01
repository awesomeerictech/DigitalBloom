import QtQuick
import Qt5Compat.GraphicalEffects
import QtQuick.Controls
import ComponentLibrary
import Qaterial as Qaterial
import "qrc:/"
import "qrc:/qml"

Item {
    id: _item

    property alias source: _image.source
    property alias asynchronous: _image.asynchronous
    property alias fillMode: _image.fillMode
    property alias borderWidth: _border.border.width
    property int borderRadius: 10
 /*   property int myh
    property int myw
    width: myw
    height: myh */
    implicitWidth: _image.implicitWidth
    implicitHeight: _image.implicitHeight

    Image {

        id: _image
        cache: false
        width: _item.width
        height: _item.height
        visible: false
        fillMode: Image.PreserveAspectFit
        smooth: true

    }

    OpacityMask {
        anchors.fill: _image
        source: _image
        maskSource: Rectangle {
            width: _image.width
            height: _image.height
            radius: _item.borderRadius
        }
    }

    Rectangle {
        id: _border
        anchors.fill: parent
        color: _image.status === Image.Ready ? "transparent" : "dimgray"
        opacity: _image.status === Image.Ready ? 1 : 0.5
        radius: _item.borderRadius
        border.width: 2
        border.color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorHeader : "#5483EF"
    }
}
