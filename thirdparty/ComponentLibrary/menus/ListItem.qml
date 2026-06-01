import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Controls.impl

import ComponentLibrary

T.ItemDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: EricTheme.componentMargin
    spacing: EricTheme.componentMargin
    verticalPadding: 0

    property string source
    property string sourceColor: EricTheme.colorIcon
    property int sourceSize: 32

    //property string text
    property color textColor: EricTheme.colorText
    property int textSize: EricTheme.fontSizeContent

    ////////////////

    background: Rectangle {
        implicitHeight: EricTheme.componentHeightXL

        color: EricTheme.colorBackground
    }

    ////////////////

    contentItem: RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: EricTheme.componentMargin
        anchors.right: parent.right
        anchors.rightMargin: EricTheme.componentMargin

        spacing: 0
        opacity: control.enabled ? 1 : 0.4

        Item {
            Layout.preferredWidth: appHeader.headerPosition - parent.anchors.leftMargin
            Layout.preferredHeight: EricTheme.componentHeightXL

            Layout.alignment: Qt.AlignTop

            IconSvg {
                anchors.left: parent.left
                anchors.leftMargin: (32 - control.sourceSize) / 2
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: (control.height !== EricTheme.componentHeightXL) ? -(EricTheme.componentMargin / 2) : 0

                width: control.sourceSize
                height: control.sourceSize
                color: control.sourceColor
                source: control.source
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Layout.topMargin: 0
            Layout.bottomMargin: 4

            text: control.text
            color: control.textColor
            wrapMode: Text.WordWrap
            font.pixelSize: control.textSize
            horizontalAlignment: Text.AlignJustify
        }
    }

    ////////////////
}
