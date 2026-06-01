import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ComponentLibrary

SwipeDelegate {
    id: widgetBarcodeHistory

    implicitWidth: 256
    implicitHeight: 96

    clip: true
    padding: EricTheme.componentMargin

    onClicked: {
        //console.log("WidgetBarcodeHistory::onClicked()")
        //screenBarcodeDetails.loadScreenFrom("ScreenBarcodeHistory", modelData)
    }

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        color: widgetBarcodeHistory.pressed ? Qt.darker(EricTheme.colorLowContrast, 1.05) : EricTheme.colorLowContrast

        Rectangle {
            anchors.right: parent.right
            anchors.rightMargin: -12
            anchors.verticalCenter: parent.verticalCenter

            width: parent.height*0.33
            height: parent.height*1.33
            rotation: 10
            antialiasing: true
            color: parent.color
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: EricTheme.colorSeparator
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Item {
        RowLayout {
            anchors.left: parent.left
            anchors.leftMargin: screenPaddingLeft
            anchors.right: parent.right
            anchors.rightMargin: screenPaddingRight
            anchors.verticalCenter: parent.verticalCenter
            spacing: EricTheme.componentMargin

            ////////

            Item {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignVCenter

                IconSvg {
                    anchors.centerIn: parent
                    width: modelData.isMatrix ? 48 : 32
                    height: modelData.isMatrix ? 48 : 32
                    color: EricTheme.colorText
                    source: modelData.isMatrix ? "qrc:/IconLibrary/material-symbols/qr_code_2.svg" :
                                                 "qrc:/IconLibrary/material-symbols/barcode.svg"
                }
            }

            ////////

            Column {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: EricTheme.componentMargin / 3

                ////

                RowLayout { // content
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 8

                    TagClear {
                        visible: modelData.content
                        text: modelData.content
                    }
                    Text {
                        text: modelData.data
                        Layout.fillWidth: true
                        font.pixelSize: EricTheme.fontSizeContentBig
                        color: EricTheme.colorText
                        elide: Text.ElideRight
                    }
                }

                ////

                Row { // info
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 8

                    ////

                    Row { // date
                        visible: modelData.date
                        height: 16
                        spacing: 6

                        IconSvg {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 16
                            height: 16
                            source: "qrc:/IconLibrary/material-icons/duotone/date_range.svg"
                            color: EricTheme.colorSubText
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.date.toLocaleString(Qt.locale(), "dddd, MMMM d, yyyy hh:mm")
                            font.pixelSize: EricTheme.fontSizeContentSmall
                            color: EricTheme.colorSubText
                        }
                    }

                    ////

                    Row { // location
                        visible: (modelData.latitude != 0 && modelData.longitude != 0)
                        height: 16
                        spacing: 6

                        IconSvg {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 16
                            height: 16
                            source: "qrc:/IconLibrary/material-icons/duotone/pin_drop.svg"
                            color: EricTheme.colorSubText
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.latitude + "°N " + modelData.longitude + "°E"
                            font.pixelSize: EricTheme.fontSizeContentSmall
                            color: EricTheme.colorSubText
                        }
                    }

                    ////
                }

                ////
            }

            ////////

            Item {
                Layout.preferredWidth: modelData.isStarred ? 48 : 0
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignVCenter

                IconSvg {
                    anchors.centerIn: parent
                    width: 32
                    height: 32
                    visible: modelData.isStarred
                    color: EricTheme.colorSubText
                    source: "qrc:/IconLibrary/material-symbols/stars-fill.svg"
                }
            }

            ////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    swipe.right: Row {
        anchors.right: parent.right
        height: parent.height

        Item {
            id: deleteLabel
            width: parent.height*1.33
            height: parent.height

            Rectangle {
                anchors.fill: parent
                color: widgetBarcodeHistory.background.color
            }

            Rectangle {
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: 10

                width: parent.height*1.33
                height: parent.height*2
                rotation: 10
                antialiasing: true

                color: deleteLabel.SwipeDelegate.pressed ? Qt.darker(EricTheme.colorMaterialOrange, 1.1) : EricTheme.colorMaterialOrange
                border.width: 2
                border.color: Qt.darker(color, 1.1)
            }

            Column {
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: 8

                IconSvg {
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "qrc:/IconLibrary/material-symbols/delete.svg"
                    color: "white"
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Remove")
                    textFormat: Text.PlainText
                    font.bold: true
                    color: "white"
                }
            }

            SwipeDelegate.onClicked: {
                utilsApp.vibrate(33)
                barcodeManager.removeHistory(modelData.data)
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
