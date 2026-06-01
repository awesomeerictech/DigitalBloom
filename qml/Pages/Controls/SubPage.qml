

import Fluid as Fluid

Fluid.Page {
    title: "Sub page demo"

    actions: [
        Fluid.Action {
            icon.source: Fluid.Utils.iconUrl("action/settings")
            toolTip: qsTr("Settings")
            hoverAnimation: true
        }
    ]

    Fluid.Label {
        anchors.centerIn: parent
        text: "Testing"
    }
}
