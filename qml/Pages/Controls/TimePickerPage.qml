

import QtQuick
import QtQuick.Controls.Material
import Fluid as Fluid
import "../.." as Components

Components.StyledPage {
    id: page

    Fluid.ScrollView {
        anchors.fill: parent
        clip: true

        Column {
            spacing: 16

            Fluid.Button {
                text: qsTr("Landscape")
                onClicked: timePickerDialogLandscape.open()
            }

            Fluid.Button {
                text: qsTr("Portrait")
                onClicked: timePickerDialogPortrait.open()
            }

            Fluid.DisplayLabel {
                id: timeLabel
                level: 2
                text: qsTr("n.a.")
            }

            Fluid.Switch {
                id: prefer24HourSwitch
                text: qsTr("24 hour clock")
            }

            Fluid.TimePicker {
                orientation: Fluid.TimePicker.Landscape
                selectedTime: new Date(1, 0, 1, 21, 12, 42)
            }

            Fluid.TimePicker {
                orientation: Fluid.TimePicker.Portrait
                selectedTime: new Date(1, 0, 1, 21, 12, 42)
            }
        }

        Fluid.TimePickerDialog {
            id: timePickerDialogLandscape
            orientation: Fluid.TimePicker.Landscape
            selectedTime: new Date(1, 0, 1, 21, 12, 42)
            prefer24Hour: prefer24HourSwitch.checked
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            standardButtonsContainer: Button {
                height: parent.height - 5
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Now")
                flat: true
                onClicked: timePickerDialogLandscape.selectedTime = new Date()
            }
            onAccepted: timeLabel.text = selectedTime.toLocaleTimeString(Qt.locale(), "hh:mm ap")

            Material.theme: page.Material.theme
        }

        Fluid.TimePickerDialog {
            id: timePickerDialogPortrait
            orientation: Fluid.TimePicker.Portrait
            selectedTime: new Date(1, 0, 1, 21, 12, 42)
            prefer24Hour: prefer24HourSwitch.checked
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            standardButtonsContainer: Button {
                height: parent.height - 5
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Now")
                flat: true
                onClicked: timePickerDialogPortrait.selectedTime = new Date()
            }
            onAccepted: timeLabel.text = selectedTime.toLocaleTimeString(Qt.locale(), "hh:mm ap")

            Material.theme: page.Material.theme
        }
    }
}
