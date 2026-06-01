import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

import ComponentLibrary

Popup {
    id: actionMenu
    width: 200

    padding: 0
    margins: 0

    parent: Overlay.overlay
    modal: true
    dim: false
    focus: isMobile
    locale: Qt.locale()
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    enter: Transition { NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 133; } }
    exit: Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 133; } }

    property var model: null
    property int layoutDirection: Qt.LeftToRight

    signal menuSelected(var index)

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        color: EricTheme.colorBackground
        radius: EricTheme.componentRadius
        border.color: EricTheme.colorSeparator
        border.width: EricTheme.componentBorderWidth
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Column {
        padding: EricTheme.componentBorderWidth

        topPadding: 8
        bottomPadding: 8
        spacing: 4

        DelegateChooser {
            id: chooser
            role: "t"
            DelegateChoice {
                roleValue: "sep"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: EricTheme.componentBorderWidth
                    color: EricTheme.colorSeparator
                }
            }
            DelegateChoice {
                roleValue: "itm"
                ActionMenuItem {
                    index: idx
                    text: txt
                    source: src
                    layoutDirection: actionMenu.layoutDirection
                    onClicked: {
                        actionMenu.menuSelected(idx)
                        actionMenu.close()
                    }
                }
            }
        }

        Repeater {
            model: actionMenu.model
            delegate: chooser
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
