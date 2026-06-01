

import QtQuick
import Fluid as Fluid
import "../.."

Item {
    Fluid.NavigationListView {
        id: navListView

        autoHighlight: autoHighlightCheckBox.checked

        topContent: Image {
            width: parent.width
            height: 200
            source: "qrc:/images/materialbg.png"
        }

        actions: [
            Fluid.Action {
                text: "Action 1"
            },
            Fluid.Action {
                text: "Action 2"
            }
        ]
    }

    Column {
        anchors.centerIn: parent

        Fluid.CheckBox {
            id: autoHighlightCheckBox
            text: qsTr("Auto highlight")
        }

        Fluid.Button {
            text: navListView.opened ? qsTr("Close") : qsTr("Open")
            onClicked: navListView.opened ? navListView.close() : navListView.open()
        }
    }
}
