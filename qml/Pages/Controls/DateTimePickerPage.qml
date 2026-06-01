

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
                onClicked: dateTimePickerDialogLandscape.open()
            }

            Fluid.Button {
                text: qsTr("Portrait")
                onClicked: dateTimePickerDialogPortrait.open()
            }

            Fluid.DisplayLabel {
                id: dateTimeLabel
                level: 2
                text: qsTr("n.a.")
            }

            Fluid.Switch {
                id: prefer24HourSwitch
                text: qsTr("24 hour clock")
            }

            Fluid.DateTimePicker {
                orientation: Fluid.DateTimePicker.Landscape
                selectedDateTime: new Date(2012, 11, 21, 21, 12, 42)
                onSelectedDateTimeChanged: console.log("Selected date time:", selectedDateTime.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm ap"))
            }

            Fluid.DateTimePicker {
                orientation: Fluid.DateTimePicker.Portrait
                selectedDateTime: new Date(2012, 11, 21, 21, 12, 42)
                onSelectedDateTimeChanged: console.log("Selected date time:", selectedDateTime.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm ap"))
            }
        }

        Fluid.DateTimePickerDialog {
            id: dateTimePickerDialogLandscape
            orientation: Fluid.DateTimePicker.Landscape
            selectedDateTime: new Date(2012, 11, 21, 21, 12, 42)
            prefer24Hour: prefer24HourSwitch.checked
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            standardButtonsContainer: Button {
                height: parent.height - 5
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Now")
                flat: true
                onClicked: dateTimePickerDialogLandscape.selectedDateTime = new Date()
            }
            onAccepted: dateTimeLabel.text = selectedDateTime.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm ap")
            onSelectedDateTimeChanged: console.log("Selected date time:", selectedDateTime.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm ap"))

            Material.theme: page.Material.theme
        }

        Fluid.DateTimePickerDialog {
            id: dateTimePickerDialogPortrait
            orientation: Fluid.DateTimePicker.Portrait
            selectedDateTime: new Date(2012, 11, 21, 21, 12, 42)
            prefer24Hour: prefer24HourSwitch.checked
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            standardButtonsContainer: Button {
                height: parent.height - 5
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Now")
                flat: true
                onClicked: dateTimePickerDialogPortrait.selectedDateTime = new Date()
            }
            onAccepted: dateTimeLabel.text = selectedDateTime.toLocaleString(Qt.locale(), "yyyy-MM-dd  hh:mm ap")
            onSelectedDateTimeChanged: console.log("Selected date time:", selectedDateTime.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm ap"))

            Material.theme: page.Material.theme
        }
    }
}
