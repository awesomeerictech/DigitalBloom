
import QtQuick
import QtQuick.Layouts
import Fluid as Fluid
import "../.." as Components

Components.StyledPageTwoColumns {
    leftColumn: ColumnLayout {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Enabled")

            Layout.alignment: Qt.AlignHCenter
        }

        Fluid.RadioButton {
            checked: true
            text: checked ? qsTr("On") : qsTr("Off")
        }

        Fluid.RadioButton {
            checked: false
            checkable: false
            text: qsTr("Off")
        }
    }

    rightColumn: ColumnLayout {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Disabled")

            Layout.alignment: Qt.AlignHCenter
        }

        Fluid.RadioButton {
            enabled: false
            checked: true
            text: checked ? qsTr("On") : qsTr("Off")
        }

        Fluid.RadioButton {
            enabled: false
            checked: false
            checkable: false
            text: qsTr("Off")
        }
    }
}
