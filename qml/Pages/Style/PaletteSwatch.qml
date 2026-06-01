

import QtQuick
import QtQuick.Controls.Material
import Fluid as Fluid
import "../.."

Column {
    id: mainLayout

    property int paletteIndex
    property string paletteName
    property color paletteColor

    spacing: 0

    Rectangle {
        width: 300
        height: 80
        color: paletteColor

        Fluid.Label {
            anchors {
                top: parent.top
                left: parent.left
                margins: Fluid.Units.smallSpacing * 2
            }
            font.bold: true
            color: Fluid.Color.lightDark(parent.color, blackColor, whiteColor)
            text: paletteName
        }
    }

    Column {
        spacing: 0

        Repeater {
            model: ListModel {
                ListElement {
                    shadeIndex: Material.Shade100
                    name: "100"
                }
                ListElement {
                    shadeIndex: Material.Shade200
                    name: "200"
                }
                ListElement {
                    shadeIndex: Material.Shade300
                    name: "300"
                }
                ListElement {
                    shadeIndex: Material.Shade400
                    name: "400"
                }
                ListElement {
                    shadeIndex: Material.Shade500
                    name: "500"
                }
                ListElement {
                    shadeIndex: Material.Shade600
                    name: "600"
                }
                ListElement {
                    shadeIndex: Material.Shade700
                    name: "700"
                }
                ListElement {
                    shadeIndex: Material.Shade800
                    name: "800"
                }
                ListElement {
                    shadeIndex: Material.Shade900
                    name: "900"
                }
            }

            Rectangle {
                width: 300
                height: 40
                color: Material.color(paletteIndex, model.shadeIndex)

                Fluid.Label {
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                        margins: Fluid.Units.smallSpacing * 2
                    }
                    font.bold: true
                    color: Fluid.Color.lightDark(parent.color, blackColor, whiteColor)
                    text: model.name
                }

                Fluid.Label {
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        margins: Fluid.Units.smallSpacing * 2
                    }
                    font.bold: true
                    color: Fluid.Color.lightDark(parent.color, blackColor, whiteColor)
                    text: parent.color
                }
            }
        }
    }

    Column {
        spacing: 0

        Repeater {
            model: ListModel {
                ListElement {
                    shadeIndex: Material.ShadeA100
                    name: "A100"
                }
                ListElement {
                    shadeIndex: Material.ShadeA200
                    name: "A200"
                }
                ListElement {
                    shadeIndex: Material.ShadeA400
                    name: "A400"
                }
                ListElement {
                    shadeIndex: Material.ShadeA700
                    name: "A700"
                }
            }

            Rectangle {
                width: 300
                height: 40
                color: Material.color(paletteIndex, model.shadeIndex)

                Fluid.Label {
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                        margins: Fluid.Units.smallSpacing * 2
                    }
                    font.bold: true
                    color: Fluid.Color.lightDark(parent.color, blackColor, whiteColor)
                    text: model.name
                }

                Fluid.Label {
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        margins: Fluid.Units.smallSpacing * 2
                    }
                    font.bold: true
                    color: Fluid.Color.lightDark(parent.color, blackColor, whiteColor)
                    text: parent.color
                }
            }
        }
    }
}
