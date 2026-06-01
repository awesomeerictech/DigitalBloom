import QtQuick
import QtQuick.Controls

import ComponentLibrary
import DigitalBloom

DrawerThemed {
    contentItem: Item {

        Column {
            id: rectangleHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            z: 5

            ////////

            Rectangle {
                id: rectangleStatusbar
                anchors.left: parent.left
                anchors.right: parent.right

                height: Math.max(screenPaddingTop, screenPaddingStatusbar)
                color: EricTheme.colorBackground // to hide flickable content

                Rectangle {
                    anchors.fill: parent
                    color: EricTheme.colorStatusbar // so we can read the statusbar
                    opacity: 0.85
                }
            }

            ////////

            Rectangle {
                id: rectangleLogo
                anchors.left: parent.left
                anchors.right: parent.right

                height: 80
                color: EricTheme.colorBackground

                Image {
                    id: imageHeader
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter

                    width: 40
                    height: 40
                    source: "qrc:/myassets/gfx/logos/logo.svg"
                    //sourceSize: Qt.size(width, height)
                }
                Text {
                    id: textHeader
                    anchors.left: imageHeader.right
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: 0

                    text: utilsApp.appName()
                    color: EricTheme.colorText
                    font.bold: false
                    font.pixelSize: EricTheme.fontSizeTitle
                }
            }

            ////////
        }

        ////////////////////////////////////////////////////////////////////////

        Flickable {
            anchors.top: rectangleHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            contentWidth: -1
            contentHeight: contentColumn.height

            Column {
                id: contentColumn
                anchors.left: parent.left
                anchors.right: parent.right

                ////////

                ListSeparatorPadded { }

                ////////

                DrawerItem {
                    highlighted: (appContent.state === "ScreenBarcodeReader")
                    text: qsTr("Qr code reader")
                    source: "qrc:/IconLibrary/material-icons/duotone/qr_code_scanner.svg"

                    onClicked: {
                        screenBarcodeReader.loadScreen()
                        appDrawer.close()
                    }
                }

                DrawerItem {
                    visible: true
                    highlighted: (appContent.state === "ScreenBarcodeWriter")
                    text: qsTr("My Qr Code")
                    source: "qrc:/IconLibrary/material-symbols/qr_code_2.svg"

                    onClicked: {
                        screenBarcodeWriter.loadScreen()
                        appDrawer.close()
                    }
                }

                ////////

                ListSeparatorPadded { }

                DrawerItem {
                    highlighted: (appContent.state === "ScreenBarcodeHistory")
                    text: qsTr("Qr Menu")
                    source: "qrc:/IconLibrary/material-icons/duotone/list.svg"

                    onClicked: {
                        screenBarcodeHistory.loadScreen()
                        appDrawer.close()
                    }
                }

                ////////

                ListSeparatorPadded { }

                ////////

                DrawerItem {
                    text: qsTr("Settings")
                    source: "qrc:/IconLibrary/material-symbols/settings.svg"
                    highlighted: (appContent.state === "ScreenSettings")

                    onClicked: {
                        screenSettings.loadScreen()
                        appDrawer.close()
                    }
                }

                DrawerItem {
                    text: qsTr("About")
                    source: "qrc:/IconLibrary/material-symbols/info.svg"
                    highlighted: (appContent.state === "ScreenAbout" ||
                                  appContent.state === "ScreenAboutFormats" ||
                                  appContent.state === "ScreenAboutPermissions")

                    onClicked: {
                        screenAbout.loadScreen()
                        appDrawer.close()
                    }
                }

                ////////

                ListSeparatorPadded { }

                ////////
            }
        }

        ////////////////

        Rectangle {
            id: rectangleNavigationbar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            height: Math.max(screenPaddingBottom, screenPaddingNavbar)
            color: EricTheme.colorForeground // so we can read the navigation bar
            opacity: 0.85
        }

        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
