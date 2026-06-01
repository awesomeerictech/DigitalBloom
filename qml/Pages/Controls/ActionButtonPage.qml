

import QtQuick
import Fluid as Fluid
import "../.." as Components

Components.StyledPageTwoColumns {
    leftColumn: Column {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Enabled")
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("device/airplanemode_active")
            mini: false
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("navigation/check")
            highlighted: true
            mini: false
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("device/airplanemode_active")
            mini: true
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("navigation/check")
            highlighted: true
            mini: true
        }
    }

    rightColumn: Column {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Disabled")
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("device/airplanemode_active")
            mini: false
            enabled: false
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("navigation/check")
            highlighted: true
            mini: false
            enabled: false
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("device/airplanemode_active")
            mini: true
            enabled: false
        }

        Fluid.FloatingActionButton {
            icon.source: Utils.iconUrl("navigation/check")
            highlighted: true
            mini: true
            enabled: false
        }
    }
}
