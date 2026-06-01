

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

        GridLayout {
            rows: 4
            columns: 2

            Fluid.Label {
                text: qsTr("Horizontal / Single")
            }

            Fluid.Slider {
                from: 0.0
                to: 1.0
                value: 0.5
            }

            Fluid.Label {
                text: qsTr("Horizontal / Range")
            }

            Fluid.RangeSlider {
                from: 0.0
                to: 1.0
                first.value: 0.4
                second.value: 0.6
            }

            Fluid.Label {
                text: qsTr("Vertical / Single")
            }

            Fluid.Slider {
                from: 0.0
                to: 1.0
                value: 0.5
                orientation: Qt.Vertical
            }

            Fluid.Label {
                text: qsTr("Vertical / Range")
            }

            Fluid.RangeSlider {
                from: 0.0
                to: 1.0
                first.value: 0.4
                second.value: 0.6
                orientation: Qt.Vertical
            }
        }
    }

    rightColumn: ColumnLayout {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Disabled")

            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            rows: 4
            columns: 2

            Fluid.Label {
                text: qsTr("Horizontal / Single")
            }

            Fluid.Slider {
                enabled: false
                from: 0.0
                to: 1.0
                value: 0.5
            }

            Fluid.Label {
                text: qsTr("Horizontal / Range")
            }

            Fluid.RangeSlider {
                enabled: false
                from: 0.0
                to: 1.0
                first.value: 0.4
                second.value: 0.6
            }

            Fluid.Label {
                text: qsTr("Vertical / Single")
            }

            Fluid.Slider {
                enabled: false
                from: 0.0
                to: 1.0
                value: 0.5
                orientation: Qt.Vertical
            }

            Fluid.Label {
                text: qsTr("Vertical / Range")
            }

            Fluid.RangeSlider {
                enabled: false
                from: 0.0
                to: 1.0
                first.value: 0.4
                second.value: 0.6
                orientation: Qt.Vertical
            }
        }
    }
}
