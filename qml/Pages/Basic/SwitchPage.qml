
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

        Fluid.Switch {
            checked: true
            text: checked ? qsTr("On") : qsTr("Off")
        }

        Fluid.Switch {
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

        Fluid.Switch {
            enabled: false
            checked: true
            text: checked ? qsTr("On") : qsTr("Off")
        }

        Fluid.Switch {
            enabled: false
            checked: false
            checkable: false
            text: qsTr("Off")
        }
    }
}
