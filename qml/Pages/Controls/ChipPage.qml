

import QtQuick
import QtQuick.Controls.Material
import Fluid as Fluid
import "../.." as Components

Components.StyledPage {
    Column {
        anchors.centerIn: parent
        spacing: 16

        Column {
            spacing: 16

            Fluid.TitleLabel {
                text: qsTr("Input Chips")
            }

            ListModel {
                id: contactModel

                ListElement {
                    label: "Contact Name 1"
                    value: "primaryemail@email.com"
                    imageSource: "qrc:/images/balloon.jpg"
                }
                ListElement {
                    label: "Contact Name 2"
                    value: "email2@email.com"
                    imageSource: "qrc:/images/balloon.jpg"
                }
                ListElement {
                    label: "Contact Name 3"
                    value: "email3@email.com"
                    imageSource: "qrc:/images/balloon.jpg"
                }
            }

            Row {
                spacing: 8

                Fluid.Chip {
                    id: inputChip1
                    iconItem: Fluid.CircleImage {
                        source: inputChip1.selectedItem.imageSource
                        width: 24
                        height: 24
                    }
                    text: selectedItem.label
                    expandable: true
                    model: contactModel
                    onSelectedItemChanged: console.info(selectedItem.value)
                }

                Fluid.Chip {
                    id: inputChip2
                    iconItem: Fluid.CircleImage {
                        source: inputChip2.selectedItem.imageSource
                        width: 24
                        height: 24
                    }
                    text: selectedItem.label
                    expandable: true
                    deletable: true
                    model: contactModel
                    onDeleted: console.info(qsTr("Deleted"))
                    onSelectedItemChanged: console.info(selectedItem.value)
                }
            }
        }

        Column {
            spacing: 16

            Fluid.TitleLabel {
                text: qsTr("Color Chips")
            }

            ButtonGroup {
                id: colorChipsGroup
            }

            Row {
                spacing: 8

                Fluid.Chip {
                    id: colorChip1
                    iconItem: Rectangle {
                        width: 24
                        height: 24
                        radius: 12
                        color: Material.color(Material.Blue)

                        Fluid.Icon {
                            anchors.centerIn: parent
                            source: colorChip1.checked ? Utils.iconUrl("navigation/check") : ""
                            size: 20
                        }
                    }
                    text: qsTr("Blue 500")
                    checkable: true
                    ButtonGroup.group: colorChipsGroup
                }

                Fluid.Chip {
                    id: colorChip2
                    iconItem: Rectangle {
                        width: 24
                        height: 24
                        radius: 12
                        color: Material.color(Material.Green)

                        Fluid.Icon {
                            anchors.centerIn: parent
                            source: colorChip2.checked ? Utils.iconUrl("navigation/check") : ""
                            size: 20
                        }
                    }
                    text: qsTr("Green 500")
                    checkable: true
                    ButtonGroup.group: colorChipsGroup
                }

                Fluid.Chip {
                    id: colorChip3
                    iconItem: Rectangle {
                        width: 24
                        height: 24
                        radius: 12
                        color: Material.color(Material.Red)

                        Fluid.Icon {
                            anchors.centerIn: parent
                            source: colorChip3.checked ? Utils.iconUrl("navigation/check") : ""
                            size: 20
                        }
                    }
                    text: qsTr("Red 500")
                    checkable: true
                    ButtonGroup.group: colorChipsGroup
                }
            }
        }

        Column {
            spacing: 16

            Fluid.TitleLabel {
                text: qsTr("Choice Chips")
            }

            ButtonGroup {
                id: choiceChipsGroup
            }

            Row {
                id: choiceChips
                spacing: 8

                Fluid.Chip {
                    checkable: true
                    checked: true
                    text: qsTr("Extra Soft")
                    ButtonGroup.group: choiceChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Soft")
                    ButtonGroup.group: choiceChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Medium")
                    ButtonGroup.group: choiceChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Hard")
                    ButtonGroup.group: choiceChipsGroup
                }
            }
        }

        Column {
            spacing: 16

            Fluid.TitleLabel {
                text: qsTr("Filter Chips")
            }

            ButtonGroup {
                id: filterChipsGroup
                exclusive: false
            }

            Row {
                id: filterChips
                spacing: 8

                Fluid.Chip {
                    checkable: true
                    checked: true
                    text: qsTr("Elevator")
                    icon.source: checked ? Utils.iconUrl("navigation/check") : ""
                    ButtonGroup.group: filterChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Washer / Dryer")
                    icon.source: checked ? Utils.iconUrl("navigation/check") : ""
                    ButtonGroup.group: filterChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Fireplace")
                    icon.source: checked ? Utils.iconUrl("navigation/check") : ""
                    ButtonGroup.group: filterChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Wheelchair access")
                    icon.source: checked ? Utils.iconUrl("navigation/check") : ""
                    ButtonGroup.group: filterChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Dogs ok")
                    icon.source: checked ? Utils.iconUrl("navigation/check") : ""
                    ButtonGroup.group: filterChipsGroup
                }

                Fluid.Chip {
                    checkable: true
                    text: qsTr("Cats ok")
                    icon.source: checked ? Utils.iconUrl("navigation/check") : ""
                    ButtonGroup.group: filterChipsGroup
                }
            }
        }

        Column {
            spacing: 16

            Fluid.TitleLabel {
                text: qsTr("Chips")
            }

            Row {
                spacing: 8

                Fluid.Chip {
                    text: qsTr("Chip")
                }
                Fluid.Chip {
                    text: qsTr("Deletable chip")
                    deletable: true
                    onDeleted: console.info(qsTr("Deleted"))
                }
            }

            Row {
                spacing: 8

                Fluid.Chip {
                    text: qsTr("Chip with icon")
                    icon.source: Utils.iconUrl("action/face")
                }
                Fluid.Chip {
                    text: qsTr("Deletable chip with icon")
                    icon.source: Utils.iconUrl("social/person")
                    deletable: true
                    onDeleted: console.info(qsTr("Deleted"))
                }
            }

            Row {
                spacing: 8

                Fluid.Chip {
                    iconItem: Fluid.CircleImage {
                        source: "qrc:/images/balloon.jpg"
                        width: 24
                        height: 24
                    }
                    text: qsTr("Chip with image")
                }
                Fluid.Chip {
                    iconItem: Fluid.CircleImage {
                        source: "qrc:/images/balloon.jpg"
                        width: 24
                        height: 24
                    }
                    text: qsTr("Deletable chip with image")
                    deletable: true
                    onDeleted: console.info(qsTr("Deleted"))
                }
            }
        }
    }
}
