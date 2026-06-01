

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Fluid as Fluid
import "../.."

Flickable {
    clip: true
    contentHeight: Math.max(grid.implicitHeight, height)

    Fluid.ScrollBar.vertical: Fluid.ScrollBar {}

    property color whiteColor: Qt.rgba(255, 255, 255, 1)
    property color blackColor: Qt.rgba(0, 0, 0, 1)

    GridLayout {
        id: grid

        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            topMargin: Fluid.Units.largeSpacing
        }

        width: parent.width * 0.8

        columns: width / 300
        rowSpacing: Fluid.Units.smallSpacing
        columnSpacing: Fluid.Units.smallSpacing

        Repeater {
            model: ListModel {
                ListElement {
                    paletteIndex: Material.Red
                    name: "Red"
                }
                ListElement {
                    paletteIndex: Material.Pink
                    name: "Pink"
                }
                ListElement {
                    paletteIndex: Material.Purple
                    name: "Purple"
                }
                ListElement {
                    paletteIndex: Material.DeepPurple
                    name: "DeepPurple"
                }
                ListElement {
                    paletteIndex: Material.Indigo
                    name: "Indigo"
                }
                ListElement {
                    paletteIndex: Material.Blue
                    name: "Blue"
                }
                ListElement {
                    paletteIndex: Material.LightBlue
                    name: "LightBlue"
                }
                ListElement {
                    paletteIndex: Material.Cyan
                    name: "Cyan"
                }
                ListElement {
                    paletteIndex: Material.Teal
                    name: "Teal"
                }
                ListElement {
                    paletteIndex: Material.Green
                    name: "Green"
                }
                ListElement {
                    paletteIndex: Material.LightGreen
                    name: "LightGreen"
                }
                ListElement {
                    paletteIndex: Material.Lime
                    name: "Lime"
                }
                ListElement {
                    paletteIndex: Material.Yellow
                    name: "Yellow"
                }
                ListElement {
                    paletteIndex: Material.Amber
                    name: "Amber"
                }
                ListElement {
                    paletteIndex: Material.Orange
                    name: "Orange"
                }
                ListElement {
                    paletteIndex: Material.DeepOrange
                    name: "DeepOrange"
                }
                ListElement {
                    paletteIndex: Material.Grey
                    name: "Grey"
                }
                ListElement {
                    paletteIndex: Material.BlueGrey
                    name: "BlueGrey"
                }
                ListElement {
                    paletteIndex: Material.Brown
                    name: "Brown"
                }
            }

            PaletteSwatch {
                paletteIndex: model.paletteIndex
                paletteName: model.name
                paletteColor: Material.color(model.paletteIndex, Material.Shade500)
            }
        }
    }
}
