

import QtQuick
import QtQuick.Controls.Material
import Fluid as Fluid

Item {
    Column {
        anchors.centerIn: parent

        Fluid.Button {
            text: qsTr("List-style BottomSheet")
            onClicked: listBottomSheet.open()
        }

        Fluid.Button {
            text: qsTr("Long List-style BottomSheet")
            onClicked: longListBottomSheet.open()
        }

        Fluid.Button {
            text: qsTr("Grid-style BottomSheet")
            onClicked: gridBottomSheet.open()
        }

        Fluid.Button {
            text: qsTr("Custom BottomSheet")
            onClicked: customBottomSheet.open()
        }
    }

    Fluid.BottomSheetList {
        id: listBottomSheet
        title: qsTr("Save As")
        actions: [
            Fluid.Action {
                text: qsTr("Folder")
                icon.source: Fluid.Utils.iconUrl("file/folder")
            },
            Fluid.Action {
                text: qsTr("New Folder")
                icon.source: Fluid.Utils.iconUrl("file/create_new_folder")
            },
            Fluid.Action {
                text: qsTr("Shared Folder")
                icon.source: Fluid.Utils.iconUrl("file/folder_shared")
            },
            Fluid.Action {
                text: qsTr("Cloud")
                icon.source: Fluid.Utils.iconUrl("file/cloud")
            },
            Fluid.Action {
                text: qsTr("Email Attachment")
                icon.source: Fluid.Utils.iconUrl("file/attachment")
            },
            Fluid.Action {
                text: qsTr("Upload")
                icon.source: Fluid.Utils.iconUrl("file/file_upload")
            },
            Fluid.Action {
                text: qsTr("Warning (Disabled)")
                icon.source: Fluid.Utils.iconUrl("alert/warning")
                enabled: false
            }
        ]
    }

    Fluid.BottomSheetList {
        id: longListBottomSheet
        title: qsTr("Save As")
        actions: [
            Fluid.Action {
                text: qsTr("Folder")
                icon.source: Fluid.Utils.iconUrl("file/folder")
            },
            Fluid.Action {
                text: qsTr("New Folder")
                icon.source: Fluid.Utils.iconUrl("file/create_new_folder")
            },
            Fluid.Action {
                text: qsTr("Shared Folder")
                icon.source: Fluid.Utils.iconUrl("file/folder_shared")
            },
            Fluid.Action {
                text: qsTr("Cloud")
                icon.source: Fluid.Utils.iconUrl("file/cloud")
            },
            Fluid.Action {
                text: qsTr("Email Attachment")
                icon.source: Fluid.Utils.iconUrl("file/attachment")
            },
            Fluid.Action {
                text: qsTr("Upload")
                icon.source: Fluid.Utils.iconUrl("file/file_upload")
            },
            Fluid.Action {
                text: qsTr("Warning (Disabled)")
                icon.source: Fluid.Utils.iconUrl("alert/warning")
                enabled: false
                hasDividerAfter: true
            },
            Fluid.Action {
                text: qsTr("Placeholder 1")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 2")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 3")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 4")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 5")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 6")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 7")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 8")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 9")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 10")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            }
        ]
    }

    Fluid.BottomSheetGrid {
        id: gridBottomSheet
        actions: [
            Fluid.Action {
                text: qsTr("Folder")
                icon.source: Fluid.Utils.iconUrl("file/folder")
            },
            Fluid.Action {
                text: qsTr("New Folder")
                icon.source: Fluid.Utils.iconUrl("file/create_new_folder")
            },
            Fluid.Action {
                text: qsTr("Shared Folder")
                icon.source: Fluid.Utils.iconUrl("file/folder_shared")
            },
            Fluid.Action {
                text: qsTr("Cloud")
                icon.source: Fluid.Utils.iconUrl("file/cloud")
            },
            Fluid.Action {
                text: qsTr("Email Attachment")
                icon.source: Fluid.Utils.iconUrl("file/attachment")
            },
            Fluid.Action {
                text: qsTr("Upload")
                icon.source: Fluid.Utils.iconUrl("file/file_upload")
            },
            Fluid.Action {
                text: qsTr("Warning (Disabled)")
                icon.source: Fluid.Utils.iconUrl("alert/warning")
                enabled: false
            },
            Fluid.Action {
                text: qsTr("Placeholder 1")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 2")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 3")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 4")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 5")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 6")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 7")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 8")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 9")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 10")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 11")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 12")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 13")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 14")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 15")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 16")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 17")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 18")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 19")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            },
            Fluid.Action {
                text: qsTr("Placeholder 20")
                icon.source: Fluid.Utils.iconUrl("file/cloud_done")
            }
        ]
    }

    Fluid.BottomSheet {
        id: customBottomSheet

        Column {
            width: parent.width

            Fluid.Pane {
                width: parent.width
                padding: 16

                Column {
                    spacing: 8

                    Fluid.TitleLabel {
                        text: "freedom"
                    }

                    Fluid.BodyLabel {
                        text: "/ˈfriːdəm/"
                        color: Material.secondaryTextColor
                    }
                }

                Material.background: Material.color(Material.Yellow, Material.Shade800)
            }

            Fluid.Pane {
                width: parent.width
                implicitHeight: 100
                padding: 16

                Column {
                    Fluid.SubheadingLabel {
                        text: "noun"
                        color: Material.secondaryTextColor
                    }

                    Fluid.BodyLabel {
                        text: "the right to live in the way you want without being controlled by anyone else"
                    }
                }
            }
        }
    }
}
