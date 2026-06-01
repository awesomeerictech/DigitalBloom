import QtQuick

import ComponentLibrary

Item {
    id: control
    implicitWidth: 128
    implicitHeight: 32

    width: contentRow.width + EricTheme.componentBorderWidth*2

    opacity: enabled ? 1 : 0.66

    // colors
    property color colorBackground: EricTheme.colorComponent
    property color colorForeground: EricTheme.colorComponentBackground

    // states
    signal menuSelected(var index)
    property int currentSelection: 1

    // model
    property var model: null

    ////////////////

    Rectangle { // background
        anchors.fill: parent
        radius: EricTheme.componentRadius
        color: control.colorBackground
    }

    ////////////////

    Row {
        id: contentRow
        anchors.centerIn: parent
        height: parent.height - EricTheme.componentBorderWidth*2
        spacing: EricTheme.componentBorderWidth

        Repeater {
            model: control.model
            delegate: SelectorMenuItem {
                colorContent: EricTheme.colorComponentText
                colorContentHighlight: EricTheme.colorComponentText
                colorBackgroundHighlight: control.colorForeground
                height: parent.height
                highlighted: (control.currentSelection === idx)
                index: idx ?? 0
                text: txt ?? ""
                source: src ?? ""
                sourceSize: sz ?? 32
                onClicked: control.menuSelected(idx)
            }
        }
    }

    ////////////////
}
