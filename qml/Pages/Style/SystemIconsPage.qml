

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import Fluid as Fluid

Page {
    header: Row {
        Fluid.RadioButton {
            id: lightRadio
            text: qsTr("Light")
            checked: true
        }

        Fluid.RadioButton {
            id: darkRadio
            text: qsTr("Dark")
        }
    }

    Material.theme: lightRadio.checked ? Material.Light : Material.Dark

    Fluid.ScrollView {
        id: scrollView
        anchors.fill: parent
        clip: true

        Column {
            anchors.fill: parent
            anchors.margins: 16

            Fluid.Label {
                id: warningLabel
                width: parent.width
                text: qsTr("This feature might not be available on your platform, as it depends on " +
                           "the installation of a freedesktop.org icon theme.")
                wrapMode: Label.WordWrap
                visible: Qt.platform.os !== "linux"
            }

            Item {
                width: parent.width
                height: 16
                visible: warningLabel.visible
            }

            GridLayout {
                columns: (scrollView.width * 0.8) / 48
                columnSpacing: 16
                rowSpacing: 16

                Fluid.Icon {
                    name: "text-editor-symbolic"
                    size: 48
                }

                Fluid.Icon {
                    name: "weather-few-clouds-symbolic"
                    size: 48
                }

                Fluid.Icon {
                    name: "system-software-install-symbolic"
                    size: 48
                }

                Fluid.Icon {
                    name: "system-users-symbolic"
                    size: 48
                }

                Fluid.Icon {
                    name: "accessories-calculator"
                    size: 48
                }

                Fluid.Icon {
                    name: "accessories-character-map"
                    size: 48
                }

                Fluid.Icon {
                    name: "accessories-dictionary"
                    size: 48
                }

                Fluid.Icon {
                    name: "accessories-text-editor"
                    size: 48
                }
            }
        }
    }
}
