import QtQuick
import QtQuick.Controls
import QtCore
import QtQuick.Dialogs


import ComponentLibrary
import DigitalBloom

Item {
    id: screenAboutPermissions
    anchors.fill: parent

    property string entryPoint: "ScreenAbout"

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // Refresh permissions
        refreshPermissions()

        // Change screen
        appContent.state = "ScreenAboutPermissions"
    }

    function loadScreenFrom(screenname) {
        entryPoint = screenname
        loadScreen()
    }

    function backAction() {
        screenAbout.loadScreen()
    }

    function refreshPermissions() {
        // Refresh permissions
        button_network_test.validperm = true
        button_location_test.validperm = utilsApp.checkMobileBleLocationPermission()
        button_camera_test.validperm = utilsApp.checkMobileCameraPermission()
        button_storage_test.validperm = utilsApp.getMobileStoragePermissions()

    }

    Timer {
        id: retryPermissions
        interval: 333
        repeat: false
        onTriggered: refreshPermissions()
    }

    ////////////////////////////////////////////////////////////////////////////

    Flickable {
        anchors.fill: parent

        contentWidth: -1
        contentHeight: contentColumn.height

        anchors.leftMargin: screenPaddingLeft
        anchors.rightMargin: screenPaddingRight

        Column {
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right

            topPadding: 24
            bottomPadding: 24
            spacing: 16

            ////////

            Item { // element_network
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20

                RoundButtonIcon {
                    id: button_network_test
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    width: 32
                    height: 32
                    z: 1

                    property bool validperm: true

                    source: (validperm) ? "qrc:/IconLibrary/material-symbols/check.svg" : "qrc:/IconLibrary/material-symbols/close.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? EricTheme.colorSuccess : EricTheme.colorSubText
                    backgroundVisible: true

                    onClicked: {
                        refreshPermissions.start()
                    }
                }

                Text { // text_network
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    height: 16

                    text: qsTr("Network access")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: EricTheme.fontSizeContentBig
                    color: EricTheme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text { // legend_network
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 16

                text: qsTr("Network access is used to provide connection needed for device communication, Qr codescanning and product listing")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: EricTheme.colorSubText
                font.pixelSize: EricTheme.fontSizeContentSmall
            }

            ////////

            Item { // element_camera
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20

                RoundButtonIcon {
                    id: button_camera_test
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    width: 32
                    height: 32
                    z: 1

                    property bool validperm: true

                    source: (validperm) ? "qrc:/IconLibrary/material-symbols/check.svg" : "qrc:/IconLibrary/material-symbols/close.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? EricTheme.colorSuccess : EricTheme.colorSubText
                    backgroundVisible: true

                    onClicked: {
                        utilsApp.vibrate(25)
                        utilsApp.getMobileCameraPermission()
                        retryPermissions.start()
                    }
                }

                Text { // text_camera
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    height: 16

                    text: qsTr("Camera")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: EricTheme.fontSizeContentBig
                    color: EricTheme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text { // legend_camera
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 16

                text: qsTr("Camera is used to scan Qr codes.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: EricTheme.colorSubText
                font.pixelSize: EricTheme.fontSizeContentSmall
            }

            ///// Storage

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20

                RoundButtonIcon {
                    id: button_storage_test
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    width: 32
                    height: 32
                    z: 1

                    property bool validperm: true

                    source: (validperm) ? "qrc:/IconLibrary/material-symbols/check.svg" : "qrc:/IconLibrary/material-symbols/close.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? EricTheme.colorSuccess : EricTheme.colorSubText
                    backgroundVisible: true

                    onClicked: {
                        utilsApp.vibrate(25)
                        fileDialog.open()
                        utilsApp.getMobileStoragePermissions()
                        retryPermissions.start()
                    }
                }

                Text { // text_storage
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    height: 16

                    text: qsTr("Storage")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: EricTheme.fontSizeContentBig
                    color: EricTheme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 16

                text: qsTr("We need Storage permissions to read and write files to the device")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: EricTheme.colorSubText
                font.pixelSize: EricTheme.fontSizeContentSmall
            }

            Image {
                    id: image
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    fillMode: Image.PreserveAspectFit
                    // Capture the built-in signal
                      onStatusChanged: {
                        if (image.status === Image.Ready) {
                            button_storage_test.source = "qrc:/IconLibrary/material-symbols/check.svg"
                            button_storage_test.validperm = true
                            image.enabled = false
                            image.visible = false
                         }
                         else if (image.status === Image.Error) {
                            button_storage_test.source = "qrc:/IconLibrary/material-symbols/check.svg"
                            button_storage_test.validperm = false
                            image.enabled = true
                            image.visible = true

                         } else if (image.status === Image.Null) {

                            button_storage_test.validperm = false
                            image.enabled = true
                            image.visible = true
                          }
                        }

                }

            FileDialog {
                    id: fileDialog
                    fileMode: FileDialog.OpenFile // FileDialog.SaveFile FileDialog.OpenFiles
                    currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
                    nameFilters: [
                            "Text files (*.txt)",
                            "HTML files (*.html *.htm)",
                            "Images (*.png *.jpg *.jpeg)",
                            "All Files (*.*)"
                        ]
                   onAccepted: {
                       image.source = selectedFile
                       console.log("selected file is:"+selectedFile)

                       // console.log("Files selected:")
                       //            // Iterate over the list of selected files (URLs)
                       //            for (var i = 0; i < selectedFiles.length; i++) {
                       //                console.log("- " + selectedFiles[i])
                       //            }
                       //            // Optional: Access the paths directly
                       //            // for (var i = 0; i < selectedFiles.length; i++) {
                       //            //     console.log("- Path: " + selectedFiles[i].replace("file://", ""))

                       // // Use the vanilla JavaScript forEach method on the selectedFiles list
                       //             selectedFiles.forEach(function(fileURL, index) {
                       //                 console.log("File " + (index + 1) + ": " + fileURL)

                       //                 // Example: Get local file path without the 'file://' prefix
                       //                 let localPath = fileURL.replace(/^file:\/\//, "");

                       //                 // On Windows, paths might need an extra slash handled if they start with file:///C:/
                       //                 // QML often handles this automatically, but you can process the string as needed.
                       //                 console.log("Local Path: " + localPath);
                       //             });

                       //             // Alternatively, using a modern 'for...of' loop
                       //             // for (const fileURL of selectedFiles) {
                       //             //     console.log("Path (for...of): " + fileURL);
                       //             // }

                   }


                }


            ////////

            Item { // element_position
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20

                RoundButtonIcon {
                    id: button_location_test
                    width: 32
                    height: 32
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    property bool validperm: true

                    source: (validperm) ? "qrc:/IconLibrary/material-symbols/check.svg" : "qrc:/IconLibrary/material-symbols/close.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? EricTheme.colorSuccess : EricTheme.colorSubText
                    backgroundVisible: true

                    onClicked: {
                        utilsApp.vibrate(25)
                        utilsApp.getMobileLocationPermission()
                        retryPermissions.start()
                    }
                }

                Text { // text_position
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    height: 16

                    text: qsTr("Position (GPS)")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: EricTheme.fontSizeContentBig
                    color: EricTheme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text { // legend_position
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 16

                text: qsTr("You can save GPS position for support")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: EricTheme.colorSubText
                font.pixelSize: EricTheme.fontSizeContentSmall
            }

            ////////

            ListSeparatorPadded { height: 16+1 }

            ////////

            Item { // element_infos
                anchors.left: parent.left
                anchors.right: parent.right
                height: 32

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: EricTheme.componentMargin + 4
                    anchors.verticalCenter: parent.verticalCenter
                    width: 32
                    height: 32

                    opacity: 0.66
                    color: EricTheme.colorSubText
                    source: "qrc:/IconLibrary/material-icons/duotone/info.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: appHeader.headerPosition
                    anchors.right: parent.right
                    anchors.rightMargin: EricTheme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Click on the checkmarks to request missing permissions.")
                    textFormat: Text.StyledText
                    lineHeight : 0.8
                    wrapMode: Text.WordWrap
                    color: EricTheme.colorText
                    font.pixelSize: EricTheme.fontSizeContent
                }
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: appHeader.headerPosition
                anchors.right: parent.right
                anchors.rightMargin: EricTheme.componentMargin

                visible: (Qt.platform.os === "android")

                // text: qsTr("If it has no effect, you may have previously refused a permission and clicked on \"don't ask again\".") + "<br>" +
                //       qsTr("You can go to the Android \"application info\" panel to change a permission manually.")
                text: qsTr("You can always go to your phone settings and grant these permissions manually if this does not work. Click the application info button to do that.")
                textFormat: Text.StyledText
                wrapMode: Text.WordWrap
                color: EricTheme.colorSubText
                font.pixelSize: EricTheme.fontSizeContentSmall
            }

            ButtonWireframe {
                anchors.left: parent.left
                anchors.leftMargin: appHeader.headerPosition
                height: 38

                visible: (Qt.platform.os === "android")
                colorBackground: EricTheme.colorForeground

                text: qsTr("Application info")
                source: "qrc:/IconLibrary/material-icons/duotone/tune.svg"
                sourceSize: 20

                onClicked: utilsApp.openAndroidAppInfo("org.stlltd.digitalbloom")
            }

            ////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
