

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import Fluid as Fluid
import Fluid.Demo as FluidDemo

Fluid.Page {
    Material.theme: lightRadio.checked ? Material.Light : Material.Dark

    Row {
        id: themeRow
        anchors {
            top: parent.top
            left: parent.left
        }

        spacing: 16

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

    Fluid.SearchBar {
        id: searchBar
        anchors.left: themeRow.right
    }

    Fluid.ScrollView {
        id: scrollView
        anchors { left: parent.left; right: parent.right; top: searchBar.bottom; bottom: parent.bottom }

        clip: true

        ColumnLayout {
            Repeater {
                model: FluidDemo.IconCategoryModel {}
                delegate: ColumnLayout {
                    id: entry

                    property string currentCategory: model.category

                    Fluid.Subheader {
                        text: model.category
                    }

                    GridLayout {
                        columns: (scrollView.width * 0.8) / 48
                        columnSpacing: 16
                        rowSpacing: 16

                        Repeater {
                            model: FluidDemo.IconNameModel {
                                category: entry.currentCategory
                            }
                            delegate: Fluid.Icon {
                                source: Fluid.Utils.iconUrl(entry.currentCategory + "/" + model.name)
                                visible: model.name.indexOf(searchBar.searchText) !== -1
                                size: 48

                                Fluid.ToolTip.visible: iconMouseArea.containsMouse
                                Fluid.ToolTip.text: entry.currentCategory + "/" + model.name

                                MouseArea {
                                    id: iconMouseArea
                                    anchors.fill: parent
                                    acceptedButtons: Qt.NoButton
                                    hoverEnabled: true
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
