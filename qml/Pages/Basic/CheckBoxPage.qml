
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

        Fluid.CheckBox {
            checked: true
            text: checked ? qsTr("On") : qsTr("Off")
        }

        Fluid.CheckBox {
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

        Fluid.CheckBox {
            checked: true
            enabled: false
            text: qsTr("On")
        }

        Fluid.CheckBox {
            checked: false
            enabled: false
            text: qsTr("Off")
        }
    }
}
