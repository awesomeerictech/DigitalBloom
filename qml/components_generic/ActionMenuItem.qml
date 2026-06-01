import QtQuick
import QtQuick.Layouts

import ThemeEngine

Rectangle {
    id: actionMenuItem
    height: 34

    anchors.left: parent.left
    anchors.leftMargin: EricTheme.componentBorderWidth
    anchors.right: parent.right
    anchors.rightMargin: EricTheme.componentBorderWidth

    radius: 0
    color: EricTheme.colorBackground

    // actions
    signal clicked()
    signal pressAndHold()

    // settings
    property int index
    property string text
    property url source
    property int sourceSize: 20
    property int layoutDirection: Qt.RightToLeft

    ////////////////////////////////////////////////////////////////////////////

    MouseArea {
        anchors.fill: parent
        hoverEnabled: isDesktop && visible

        onClicked: actionMenuItem.clicked()
        onPressAndHold: actionMenuItem.pressAndHold()

        onEntered: actionMenuItem.state = "hovered"
        onExited: actionMenuItem.state = "normal"
        onCanceled: actionMenuItem.state = "normal"
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12

        spacing: 6
        layoutDirection: actionMenuItem.layoutDirection

        IconSvg {
            id: iButton
            width: actionMenuItem.sourceSize
            height: actionMenuItem.sourceSize
            Layout.maximumWidth: actionMenuItem.sourceSize
            Layout.maximumHeight: actionMenuItem.sourceSize

            source: actionMenuItem.source
            color: EricTheme.colorIcon
        }

        Text {
            id: tButton

            Layout.fillWidth: true

            text: actionMenuItem.text
            font.bold: false
            font.pixelSize: EricTheme.fontSizeComponent
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: EricTheme.colorText
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    states: [
        State {
            name: "normal";
            PropertyChanges { target: actionMenuItem; color: EricTheme.colorBackground; }
        },
        State {
            name: "hovered";
            PropertyChanges { target: actionMenuItem; color: EricTheme.colorSeparator; }
        }
    ]

    ////////////////////////////////////////////////////////////////////////////
}
