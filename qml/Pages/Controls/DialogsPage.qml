

import QtQuick
import Fluid as Fluid

Item {
    Column {
        anchors.centerIn: parent

        Fluid.Button {
            text: qsTr("Alert")
            onClicked: alert.open()
        }

        Fluid.Button {
            text: qsTr("Input")
            onClicked: input.open()
        }
    }

    Fluid.AlertDialog {
        id: alert

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        text: qsTr("Discard draft?")
        standardButtons: Fluid.Dialog.Discard | Fluid.Dialog.Cancel
    }

    Fluid.InputDialog {
        id: input

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 280

        title: qsTr("In what year were you born?")
        text: qsTr("We need to know in what year you were born in order to verify your age.")
        textField.inputMask: "9999"
        textField.placeholderText: qsTr("Type a 4 digits number")
    }
}
