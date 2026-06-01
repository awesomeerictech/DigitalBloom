

import QtQuick
import QtQuick.Layouts
import Fluid as Fluid
import "../.." as Components

Components.StyledPageTwoColumns {
    leftColumn: ColumnLayout {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Determinate")

            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            rows: 2
            columns: 2

            Fluid.Label {
                text: qsTr("Static")
            }

            Fluid.ProgressBar {
                from: 0.0
                to: 1.0
                value: 0.5
                indeterminate: false
            }

            Fluid.Label {
                text: qsTr("Animated")
            }

            Fluid.ProgressBar {
                from: 0.0
                to: 1.0
                indeterminate: false

                SequentialAnimation on value {
                    running: true
                    loops: NumberAnimation.Infinite

                    NumberAnimation {
                        from: 0.0
                        to: 1.0
                        duration: 3000
                    }
                }
            }
        }
    }

    rightColumn: ColumnLayout {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Indeterminate")

            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            rows: 2
            columns: 2

            Fluid.Label {
                text: qsTr("Static")
            }

            Fluid.ProgressBar {
                from: 0.0
                to: 1.0
                value: 0.5
                indeterminate: true
            }

            Fluid.Label {
                text: qsTr("Animated")
            }

            Fluid.ProgressBar {
                from: 0.0
                to: 1.0
                indeterminate: true

                SequentialAnimation on value {
                    running: true
                    loops: NumberAnimation.Infinite

                    NumberAnimation {
                        from: 0.0
                        to: 1.0
                        duration: 3000
                    }
                }
            }
        }
    }
}
