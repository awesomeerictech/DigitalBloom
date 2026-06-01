import QtQuick
import QtQuick.Effects

import ComponentLibrary

Rectangle {
    id: control

    anchors.left: parent.left
    anchors.leftMargin: singleColumn ? 0 : EricTheme.componentMargin
    anchors.right: parent.right
    anchors.rightMargin: singleColumn ? 0 : EricTheme.componentMargin

    height: EricTheme.componentHeightXL
    radius: singleColumn ? 0 : EricTheme.componentRadius
    z: 2

    color: EricTheme.colorForeground
    border.width: singleColumn ? 0 : EricTheme.componentBorderWidth
    border.color: EricTheme.colorSeparator

    property string source
    property int sourceSize: 24
    property int sourceRotation: 0
    property color sourceColor: EricTheme.colorIcon

    property string text: "title"
    property color textColor: EricTheme.colorText
    property int textSize: source ? EricTheme.fontSizeContentBig : EricTheme.fontSizeContentVeryBig

    property bool shadow: !singleColumn

    ////////////////

    IconSvg {
        anchors.left: control.left
        anchors.leftMargin: EricTheme.componentMarginL
        anchors.verticalCenter: control.verticalCenter

        visible: control.source
        width: control.sourceSize
        height: control.sourceSize
        rotation: control.sourceRotation

        color: control.sourceColor
        source: control.source
    }

    Text {
        anchors.left: control.left
        anchors.leftMargin: control.source ? (singleColumn ? appHeader.headerPosition : EricTheme.componentMarginL*2 + sourceSize)
                                           : EricTheme.componentMarginL
        anchors.right: control.right
        anchors.rightMargin: EricTheme.componentMarginL
        anchors.verticalCenter: control.verticalCenter

        text: control.text
        textFormat: Text.PlainText
        font.pixelSize: control.textSize
        font.bold: false
        color: control.textColor
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignVCenter
    }

    ////////////////

    layer.enabled: control.shadow
    layer.effect: MultiEffect {
        autoPaddingEnabled: true
        shadowEnabled: true
        shadowOpacity: 0.12
        shadowColor: control.border.color
    }

    ////////////////
}
