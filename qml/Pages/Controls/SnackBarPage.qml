

import QtQuick
import Fluid as Fluid

Item {
    Column {
        anchors.centerIn: parent

        Fluid.TextField {
            id: textField
            placeholderText: qsTr("Text")
            text: qsTr("Marked as read")
            width: 300
        }

        Fluid.TextField {
            id: buttonTextField
            placeholderText: qsTr("Button Text")
            text: qsTr("Undo")
            width: 300
        }

        Fluid.Switch {
            text: qsTr("Full Width")
            checked: snackBar.fullWidth
            onCheckedChanged: snackBar.fullWidth = checked
        }

        Row {
            spacing: 8

            Fluid.Button {
                text: qsTr("Open")
                onClicked: snackBar.open(textField.text, buttonTextField.text)
            }

            Fluid.Button {
                text: qsTr("Close")
                onClicked: snackBar.close()
            }
        }
    }

    Fluid.SnackBar {
        id: snackBar
        onClicked: console.log("Snack bar button clicked")
    }
}
