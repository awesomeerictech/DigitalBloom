import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtCore
import QtQuick.Dialogs

import ComponentLibrary
import DigitalBloom

Loader {
    id: screenSettings
    anchors.fill: parent

    function loadScreen() {
        // load screen
        screenSettings.active = true

        // change screen
        appContent.state = "ScreenSettings"
    }

    function backAction() {
        if (screenSettings.status === Loader.Ready)
            screenSettings.item.backAction()
    }

    ////////////////////////////////////////////////////////////////////////////

    active: false
    asynchronous: true

    sourceComponent: Item {
        anchors.fill: parent

        function backAction() {
            appWindow.backAction_default()
        }

        Flickable {
            anchors.fill: parent

            contentWidth: -1
            contentHeight: contentColumn.height

            boundsBehavior: isDesktop ? Flickable.OvershootBounds : Flickable.DragAndOvershootBounds
            ScrollBar.vertical: ScrollBar { visible: false }

            Column {
                id: contentColumn
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + ((singleColumn || isPhone) ? 0 : parent.width * 0.12)
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight + ((singleColumn || isPhone) ? 0 : parent.width * 0.12)

                topPadding: 20
                bottomPadding: 20
                spacing: 8

                property int padIcon: singleColumn ? EricTheme.componentMarginL : EricTheme.componentMarginL
                property int padText: appHeader.headerPosition
                property int padMargin: singleColumn ? 0 : EricTheme.componentMargin

                ////////////////

                ListTitle {
                    text: qsTr("User interface")
                    source: "qrc:/IconLibrary/material-symbols/settings.svg"
                }

                ////////////////

                Item {
                    id: element_appTheme
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    IconSvg {
                        id: image_appTheme
                        width: 24
                        height: 24
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        color: EricTheme.colorIcon
                        source: "qrc:/IconLibrary/material-icons/duotone/style.svg"
                    }

                    Text {
                        id: text_appTheme
                        anchors.left: image_appTheme.right
                        anchors.leftMargin: 24
                        anchors.right: appTheme_selector.left
                        anchors.rightMargin: 16
                        anchors.verticalCenter: parent.verticalCenter
                        height: 40

                        text: qsTr("Theme")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    Row {
                        id: appTheme_selector
                        anchors.right: parent.right
                        anchors.rightMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        z: 1
                        spacing: 10

                        Rectangle {
                            id: rectangleLight
                            width: 64
                            height: 32
                            anchors.verticalCenter: parent.verticalCenter

                            radius: 2
                            color: (EricTheme.currentTheme === EricTheme.THEME_RAIN) ? EricTheme.colorForeground : "#dddddd"
                            border.color: EricTheme.colorSecondary
                            border.width: (settingsManager.appTheme === "THEME_RAIN") ? 2 : 0

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    settingsManager.appTheme = "THEME_RAIN"
                                }
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter

                                text: qsTr("light")
                                textFormat: Text.PlainText
                                color: "#313236"
                                font.pixelSize: EricTheme.fontSizeContentSmall
                            }
                        }

                        Rectangle {
                            id: rectangleDark
                            width: 64
                            height: 32
                            anchors.verticalCenter: parent.verticalCenter

                            radius: 2
                            color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorForeground : "#313236"
                            border.color: EricTheme.colorSecondary
                            border.width: (settingsManager.appTheme === "THEME_NIGHT") ? 2 : 0

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    settingsManager.appTheme = "THEME_NIGHT"
                                }
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter

                                text: qsTr("dark")
                                textFormat: Text.PlainText
                                color: "#ddd"
                                font.pixelSize: EricTheme.fontSizeContentSmall
                            }
                        }
                    }
                }

                ////////

                Item {
                    id: element_appThemeAuto
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    IconSvg {
                        id: image_appThemeAuto
                        width: 24
                        height: 24
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        color: EricTheme.colorIcon
                        source: "qrc:/IconLibrary/material-icons/duotone/brightness_4.svg"
                    }

                    Text {
                        id: text_appThemeAuto
                        height: 40
                        anchors.left: image_appThemeAuto.right
                        anchors.leftMargin: 24
                        anchors.right: switch_appThemeAuto.left
                        anchors.rightMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Automatic dark mode")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    SwitchThemed {
                        id: switch_appThemeAuto
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        z: 1

                        checked: settingsManager.appThemeAuto
                        onClicked: {
                            settingsManager.appThemeAuto = checked
                            EricTheme.loadTheme(settingsManager.appTheme)
                            settingsManager.saveSettings()
                        }
                    }
                }
                Text {
                    id: legend_appThemeAuto
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin + 64
                    anchors.right: parent.right
                    anchors.rightMargin: 12

                    topPadding: -12
                    bottomPadding: 0
                    visible: element_appThemeAuto.visible

                    text: settingsManager.appThemeAuto ?
                              qsTr("Dark mode will switch on automatically between 9 PM and 9 AM.") :
                              qsTr("Dark mode schedule is disabled.")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    color: EricTheme.colorSubText
                    font.pixelSize: EricTheme.fontSizeContentSmall
                }

                ////////

                Item {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    Item {
                        visible: false
                        width: 56
                        height: 48

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-symbols/stars-fill.svg"
                        }
                    }

                    Text {
                        visible: false
                        anchors.left: parent.left
                        anchors.leftMargin: 64
                        anchors.right: menuDefTab.left
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Default tab")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SelectorMenuColorful {
                        id: menuDefTab
                        visible: false
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        z: 1

                        model: ListModel {
                            ListElement { idx: 1; txt: "Reader"; src: ""; sz: 0; }
                            ListElement { idx: 2; txt: "Writer"; src: ""; sz: 0; }
                        }
                        currentSelection: {
                            if (settingsManager.defaultTab === "writer") return 2
                            return 1
                        }

                        onMenuSelected: (index) => {
                            console.log("SelectorMenu clicked #" + index)
                            currentSelection = index

                            if (index === 1) settingsManager.defaultTab = "reader"
                            else if (index === 2) settingsManager.defaultTab = "writer"
                        }
                    }
                }

                Item {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    Item {
                        visible: false
                        width: 56
                        height: 48

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-symbols/stars-fill.svg"
                        }
                    }

                    Text {
                        visible: false
                        anchors.left: parent.left
                        anchors.leftMargin: 64
                        anchors.right: menuDefReader.left
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Default reader")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SelectorMenuColorful {
                        id: menuDefReader
                        visible: false
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        z: 1
                        enabled: false

                        model: ListModel {
                            ListElement { idx: 1; txt: "qzxing"; src: ""; sz: 0; }
                            ListElement { idx: 2; txt: "zxingcpp"; src: ""; sz: 0; }
                        }
                        currentSelection: {
                            if (settingsManager.backend_reader === "qzxing") return 1
                            if (settingsManager.backend_reader === "zxingcpp") return 2
                            return 0
                        }
                    }
                }

                Item {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    Item {
                        width: 56
                        height: 48

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-symbols/stars-fill.svg"
                        }
                    }

                    Text {
                        visible: false
                        anchors.left: parent.left
                        anchors.leftMargin: 64
                        anchors.right: menuDefWriter.left
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Default writer")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SelectorMenuColorful {
                        id: menuDefWriter
                        visible: false
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        z: 1

                        model: ListModel {
                            ListElement { idx: 1; txt: "qzxing"; src: ""; sz: 0; }
                            ListElement { idx: 2; txt: "zxingcpp"; src: ""; sz: 0; }
                            ListElement { idx: 3; txt: "zint"; src: ""; sz: 0; }
                        }
                        currentSelection: {
                            if (settingsManager.backend_writer === "qzxing") return 1
                            if (settingsManager.backend_writer === "zxingcpp") return 2
                            if (settingsManager.backend_writer === "zint") return 3
                            return 2
                        }

                        onMenuSelected: (index) => {
                            console.log("SelectorMenu clicked #" + index)
                            currentSelection = index

                            if (index === 1) settingsManager.backend_writer = "qzxing"
                            else if (index === 2) settingsManager.backend_writer = "zxingcpp"
                            else if (index === 3) settingsManager.backend_writer = "zint"
                        }
                    }
                }

                ////////////////

                ListTitle {
                    text: qsTr("Scanner")
                    source: "qrc:/IconLibrary/material-icons/duotone/qr_code_2.svg"
                }

                ////////////////

                RowLayout { // save barcodes
                    visible: false
                    enabled: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    Item {
                        Layout.preferredWidth: 56

                        IconSvg {
                            visible: false
                            enabled: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-symbols/save.svg"
                        }
                    }

                    Text {
                        visible: false
                        enabled: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Save barcode automatically")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    SwitchThemed {
                        visible: false
                        enabled: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.save_barcodes
                        onClicked: settingsManager.save_barcodes = checked
                    }
                }

                // Metadata archive

                RowLayout {
                    id: element_metadataarchive
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    Item {
                        Layout.preferredWidth: 56

                        IconSvg {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/lucide/upload.svg"
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Mass upload items using metadata archive")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }


                    Text {
                        id: legend_metadataarchive
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        Layout.topMargin: -5
                        Layout.bottomMargin: 0
                        visible: element_metadataarchive.visible

                        text: qsTr("Click the button on the left to upload a metadata archive. New items will be added automatically, and existing ones will be updated.")
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                        color: EricTheme.colorSubText
                        font.pixelSize: EricTheme.fontSizeContentSmall
                    }

                    ButtonSolid {

                        id: btnmetadataarchive
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        visible: true
                        width: isPhone ? 150 : 170
                        height: 40

                        text: qsTr("UPLOAD")
                        source: "qrc:/IconLibrary/lucide/upload.svg"
                        sourceSize: 28
                        color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorHeader : "#5483EF"

                        onClicked: {

                            fileDialog.open()
                        }
                    }



                }



                // Assuming 'archiveBridge' is registered as a context property

                  FileDialog {
                          id: fileDialog
                          title: "Select Archive with product media files"
                          fileMode: FileDialog.OpenFile
                          // Use standard paths for cross-platform compatibility
                          currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
                          nameFilters: [ "XZ Compressed Tar Archives (*.tar.xz)" ]

                          onAccepted: {

                              console.log("file dialog accepted")
                              if (fileDialog.selectedFile) {
                                  // 1. Get the URL string (file:///...) from the FileDialog
                                  const selectedFileUrlString = fileDialog.selectedFile.toString();
                                  const baseName = "item_import"; // Identifier kept external
                                  var f= ""
                                  if(Qt.platform.os==="android") {
                                      f = UtilsPath.makeUrl(fshelpereric.getfilenameandroid(fileDialog.selectedFile))
                                  }

                                  else {
                                      f = selectedFileUrlString
                                  }

                                   console.log("Cleanfile is: "+f)

                                  // 2. Call the C++ bridge function with only file path and base name.
                                  // C++ will handle all extraction and metadata reading internally.
                                  console.log("We are showing the selected file: "+fileDialog.selectedFile)
                                  console.log("We are showing the selectedFileUrlString: "+selectedFileUrlString)
                                  const result = archiveBridge.processExtractedArchive(
                                      f,
                                      baseName
                                  );

                                   console.log("We are showing the result of processing archive: "+result)

                                  // Update the image source (just for visual feedback)
                                 // image.source = fileDialog.selectedFile.toLocalFile();

                                  if (result && result.extracted_data) {
                                      const fileCount = result.extracted_data.mediafile.length;
                                      const categoryCount = result.categories_data.name.length;
                                      appWindow.showInfo(String(`Extraction success! Found ${fileCount} media files and ${categoryCount} categories.`)+String("Kindly wait for a few seconds before accessing the new menu!"))
                                      console.log("Generated JSON Payload:", JSON.stringify(result, null, 2));
                                      //netuploader.uploadtoserver(result)
                                      timeduploader.uploadExtractedItems(result)
                                  } else {
                                   //  appWindow.showToast("Extraction failed. Check logs for details.")
                                  }
                              } else {
                                  appWindow.showError("File selection cancelled or failed")
                              }
                          }
                          onRejected: {
                              appWindow.showInfo("File selection cancelled")

                          }
                      }


                // Metadata archive

                RowLayout { // save camera
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    enabled: false

                    Item {
                        visible: false
                        Layout.preferredWidth: 56

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/camera.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Save camera picture")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    SwitchThemed {
                        visible: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.save_camera
                        onClicked: settingsManager.save_camera = checked
                    }
                }

                RowLayout { // save GPS position
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    Item {
                        Layout.preferredWidth: 56

                        IconSvg {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/pin_drop.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Save GPS position")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    SwitchThemed {
                        visible: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: {
                            if (!utilsApp.checkMobileLocationPermission()) return false
                            if (settingsManager.save_gps) return true
                            return false
                        }
                        onClicked: {
                            utilsApp.getMobileLocationPermission()
                            settingsManager.save_gps = checked
                        }
                    }
                }

                ////////////////

                ListTitle {
                    visible: false
                    text: qsTr("Debug")
                    source: "qrc:/IconLibrary/material-icons/duotone/bug_report.svg"
                }

                ////////////////

                Item {
                    id: element_showDebug
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 48

                    IconSvg {
                        id: image_showDebug
                        visible: false
                        width: 24
                        height: 24
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        color: EricTheme.colorIcon
                        source: "qrc:/IconLibrary/material-icons/duotone/bug_report.svg"
                    }

                    Text {
                        id: text_showDebug
                        visible: false
                        height: 40
                        anchors.left: image_showDebug.right
                        anchors.leftMargin: 24
                        anchors.right: switch_showDebug.left
                        anchors.rightMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Show debug info")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                    }

                    SwitchThemed {
                        id: switch_showDebug
                        visible: false
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        z: 1

                        checked: settingsManager.showDebug
                        onClicked: settingsManager.showDebug = checked
                    }
                }

                ////////

                RowLayout {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 36

                   // visible: settingsManager.showDebug

                    Item {
                        Layout.preferredWidth: 56

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/qr_code_2.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Full resolution scan")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SwitchThemed {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.scan_fullres
                        onClicked: settingsManager.scan_fullres = checked
                    }
                }
                RowLayout {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 36

                   // visible: settingsManager.showDebug

                    Item {
                        visible: false
                        Layout.preferredWidth: 56

                        IconSvg {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/qr_code_2.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Full screen scan")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SwitchThemed {
                        visible: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.scan_fullscreen
                        onClicked: settingsManager.scan_fullscreen = checked
                    }
                }
                RowLayout {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 36

                   // visible: settingsManager.showDebug

                    Item {
                        Layout.preferredWidth: 56

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/qr_code_2.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Try harder")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SwitchThemed {
                        visible: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.scan_tryHarder
                        onClicked: settingsManager.scan_tryHarder = checked
                    }
                }
                RowLayout {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 36

                   // visible: settingsManager.showDebug

                    Item {
                        visible: false
                        Layout.preferredWidth: 56

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/qr_code_2.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Try rotate")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SwitchThemed {
                        visible: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.scan_tryRotate
                        onClicked: settingsManager.scan_tryRotate = checked
                    }
                }
                RowLayout {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 36

                  //  visible: settingsManager.showDebug

                    Item {
                        visible: false
                        Layout.preferredWidth: 56

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/qr_code_2.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Try invert")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SwitchThemed {
                        visible: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.scan_tryInvert
                        onClicked: settingsManager.scan_tryInvert = checked
                    }
                }
                RowLayout {
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padMargin
                    anchors.right: parent.right
                    anchors.rightMargin: contentColumn.padMargin
                    height: 36

                  //  visible: settingsManager.showDebug

                    Item {
                        visible: false
                        Layout.preferredWidth: 56

                        IconSvg {
                            visible: false
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: EricTheme.colorIcon
                            source: "qrc:/IconLibrary/material-icons/duotone/qr_code_2.svg"
                        }
                    }

                    Text {
                        visible: false
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTr("Try downscale")
                        textFormat: Text.PlainText
                        font.pixelSize: EricTheme.fontSizeContent
                        color: EricTheme.colorText
                        wrapMode: Text.WordWrap
                    }

                    SwitchThemed {
                        visible: false
                        Layout.alignment: Qt.AlignVCenter
                        Layout.rightMargin: 12
                        z: 1

                        checked: settingsManager.scan_tryDownscale
                        onClicked: settingsManager.scan_tryDownscale = checked
                    }
                }

                ////////
            }

            ////////////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////

}
