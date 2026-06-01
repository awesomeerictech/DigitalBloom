import QtQuick
import QtQuick.Controls
import QtCore
import QtQuick.Dialogs

import ComponentLibrary
import DigitalBloom

Loader {
    id: screenAbout
    anchors.fill: parent

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // load screen
        screenAbout.active = true

        // change screen
        appContent.state = "ScreenAbout"
    }

    function backAction() {
        if (screenAbout.status === Loader.Ready)
            screenAbout.item.backAction()
    }

    function openEmailClient() {
            var recipient = "stratetacticallimited@gmail.com"
            var subject = "Application Support"
            var body = "Hello,\n\nI have some feedback to share.\n\nRegards,"

            // Use encodeURIComponent() for proper URL encoding, especially for spaces and line breaks.
            var mailtoUrl = "mailto:" + recipient +
                            "?subject=" + encodeURIComponent(subject) +
                            "&body=" + encodeURIComponent(body)

            // Qt.openUrlExternally() is the correct function to launch external applications.
            Qt.openUrlExternally(mailtoUrl)
        }


    ////////////////////////////////////////////////////////////////////////////

    active: false
    asynchronous: true

    sourceComponent: Item {
        anchors.fill: parent

        ////////////////

        function backAction() {
            if (appContent.state === "ScreenAboutPermissions") {
                screenAboutPermissions.backAction()
                return
            }
            if (appContent.state === "ScreenAboutFormats") {
                screenAboutFormats.backAction()
                return
            }

            appWindow.backAction_default()
        }

        Rectangle { // hide the space between the top of the screen and the top of scanWidget
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: contentHeader.height - contentFlickable.contentY
            color: contentHeader.color
            visible: singleColumn
        }

        ////////////////

        Flickable {
            id: contentFlickable
            anchors.fill: parent

            contentWidth: -1
            contentHeight: columnContent.height

            boundsBehavior: isDesktop ? Flickable.OvershootBounds : Flickable.DragAndOvershootBounds
            ScrollBar.vertical: ScrollBar { visible: false }

            Column {
                id: columnContent

                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + ((singleColumn || isPhone) ? 0 : parent.width * 0.12)
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight + ((singleColumn || isPhone) ? 0 : parent.width * 0.12)

                ////////

                Item { width: 16; height: 16; visible: !(singleColumn || isPhone); }

                Rectangle { // header area
                    id: contentHeader
                    anchors.left: parent.left
                    anchors.leftMargin: -screenPaddingLeft
                    anchors.right: parent.right
                    anchors.rightMargin: -screenPaddingRight

                    height: 112
                    radius: (singleColumn || isPhone) ? 0 : 8
                    color: EricTheme.colorForeground

                    border.width: (singleColumn || isPhone) ? 0 : EricTheme.componentBorderWidth
                    border.color: EricTheme.colorSeparator

                    property int availableWidth: (contentHeader.width - rowTitle.width)

                    Row {
                        id: rowTitle
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + EricTheme.componentMargin
                        anchors.verticalCenter: parent.verticalCenter

                        z: 2
                        height: 112
                        spacing: EricTheme.componentMargin

                        Image { // logo
                            anchors.verticalCenter: parent.verticalCenter
                            width: 100
                            height: 100

                            source: "qrc:/myassets/gfx/logos/logo.svg"
                            //sourceSize: Qt.size(width, height)
                        }

                        Column { // title
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.verticalCenterOffset: -1

                            Text {
                                text: utilsApp.appName()
                                color: EricTheme.colorText
                                font.pixelSize: EricTheme.fontSizeTitle
                            }
                            Text {
                                text: qsTr("version %1 %2").arg(utilsApp.appVersion()).arg(utilsApp.appBuildMode())
                                textFormat: Text.PlainText
                                color: EricTheme.colorSubText
                                font.pixelSize: EricTheme.fontSizeContentBig
                            }
                        }
                    }

                    ////////

                    Row { // desktop buttons row
                        anchors.right: parent.right
                        anchors.rightMargin: screenPaddingRight + EricTheme.componentMarginL
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: EricTheme.componentMargin

                        ButtonSolid {
                            visible: (width*3.3 < contentHeader.availableWidth)
                            width: isPhone ? 150 : 160
                            height: 40

                            text: qsTr("WEBSITE")
                            source: "qrc:/IconLibrary/material-symbols/link.svg"
                            sourceSize: 28
                            color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorHeader : "#5483EF"

                            onClicked: Qt.openUrlExternally("https://web.facebook.com/profile.php?id=61576898255121")
                        }

                        ButtonSolid {
                            visible: (width*2.2 < contentHeader.availableWidth)
                            width: isPhone ? 150 : 160
                            height: 40

                            text: qsTr("SUPPORT")
                            source: "qrc:/IconLibrary/material-symbols/support.svg"
                            sourceSize: 22
                            color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorHeader : "#5483EF"

                            onClicked: screenAbout.openEmailClient()
                        }

                        ButtonSolid {
                            visible: (width*1.1 < contentHeader.availableWidth)
                            width: isPhone ? 150 : 160
                            height: 40

                            text: qsTr("GitHub")
                            source: "qrc:/myassets/gfx/logos/github.svg"
                            sourceSize: 22
                            color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorHeader : "#5483EF"

                            onClicked: Qt.openUrlExternally("https://github.com/awesomeerictech")
                        }
                    }

                    Rectangle { // bottom separator
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 2
                        visible: singleColumn
                        border.color: EricTheme.colorSeparator
                    }
                }

                Item { width: 16; height: 16; visible: !(singleColumn || isPhone); }

                ////////

                Row { // buttons row
                    height: 72

                    anchors.left: parent.left
                    anchors.leftMargin: EricTheme.componentMargin
                    anchors.right: parent.right
                    anchors.rightMargin: EricTheme.componentMargin

                    visible: !wideMode
                    spacing: EricTheme.componentMargin

                    ButtonSolid {
                        anchors.verticalCenter: parent.verticalCenter
                        width: ((parent.width - parent.spacing) / 2)

                        text: qsTr("WEBSITE")
                        source: "qrc:/IconLibrary/material-symbols/link.svg"
                        sourceSize: 28
                        color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorHeader : "#5483EF"

                        onClicked: Qt.openUrlExternally("https://web.facebook.com/profile.php?id=61576898255121")
                        //onClicked: Qt.openUrlExternally("https://web.facebook.com/profile.php?id=61576898255121")
                    }
                    ButtonSolid {
                        anchors.verticalCenter: parent.verticalCenter
                        width: ((parent.width - parent.spacing) / 2)

                        text: qsTr("SUPPORT")
                        source: "qrc:/IconLibrary/material-symbols/support.svg"
                        sourceSize: 22
                        color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? EricTheme.colorHeader : "#5483EF"

                        onClicked: screenAbout.openEmailClient()
                        //onClicked: Qt.openUrlExternally("stratetacticallimited@gmail.com")
                    }
                }

                Item { width: 12; height: 12; visible: wideMode; } // spacer

                ////////

                ListItem { // description
                    anchors.left: parent.left
                    anchors.right: parent.right

                   // text: qsTr("The Digital Bloom Program is a versatile and widely accessible initiative. In this case, we introduce a business marketing and communication module that enhances brand visibility, enabling businesses to effectively showcase their products and services to the market. This module integrates barcode technology and advanced content creation techniques to deliver a cutting-edge marketing solution. To get started just type the username for your business in the text box in 'My Qr Code' Tab and then click the Qr Code to access your marketing and communication site. For local orders connect the users to your wifi hotspot and let them scan your qr code to access your site so that they can order products")
                    text: qsTr("<strong>The Digital Bloom Program:</strong><br/><br/>This versatile and accessible program introduces a business marketing and communication module designed to enhance brand visibility and effectively showcase your products and services.<br/><br/>The module integrates <strong>barcode (Qr code) technology</strong> and advanced content-creation techniques to deliver a cutting-edge solution.<br/><br/><strong>Getting Started:</strong><br/><p>1. Enter your business username in the text box under the 'My QR Code' tab.</p><p>2. Click the QR Code to access your marketing site.</p><p>3. For local orders, connect users to your Wi-Fi hotspot, then have them scan your QR Code to access your site and place orders.</p>")

                    source: "qrc:/IconLibrary/material-symbols/info.svg"
                }

                ListItemClickable { // authors
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("Application by <a href=\"https://web.facebook.com/profile.php?id=61576898255121\">STL Limited</a>")
                    source: "qrc:/IconLibrary/material-symbols/supervised_user_circle.svg"
                    indicatorSource: "qrc:/IconLibrary/material-icons/duotone/launch.svg"

                    onClicked: Qt.openUrlExternally("https://web.facebook.com/profile.php?id=61576898255121")
                }

                ListItemClickable { // rate
                    anchors.left: parent.left
                    anchors.right: parent.right

                    //visible: (Qt.platform.os === "android" || Qt.platform.os === "ios")
                    visible: false

                    text: qsTr("Rate the application")
                    source: "qrc:/IconLibrary/material-symbols/stars-fill.svg"
                    indicatorSource: "qrc:/IconLibrary/material-icons/duotone/launch.svg"

                    onClicked: {
                        if (Qt.platform.os === "android")
                            Qt.openUrlExternally("")
                        else if (Qt.platform.os === "ios")
                            Qt.openUrlExternally("")
                        else
                            Qt.openUrlExternally("")
                    }
                }

                ListItemClickable { // release notes
                    visible: false
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("Release notes")
                    source: "qrc:/IconLibrary/material-symbols/new_releases.svg"
                    sourceSize: 28
                    indicatorSource: "qrc:/IconLibrary/material-icons/duotone/launch.svg"

                    onClicked: Qt.openUrlExternally("https://web.facebook.com/profile.php?id=61576898255121")
                }

                ////////

                ListSeparator { }

                ListItemClickable { // tutorial
                    visible: false
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("Open our Help Page")
                    source: "qrc:/IconLibrary/material-symbols/import_contacts-fill.svg"
                    sourceSize: 24
                    indicatorSource: "qrc:/IconLibrary/material-symbols/chevron_right.svg"

                    onClicked: screenTutorial.loadScreenFrom("ScreenAbout")
                }

                ////////

                ListSeparator { visible: (Qt.platform.os === "android" || Qt.platform.os === "ios") }

                ListItemClickable { // permissions
                    anchors.left: parent.left
                    anchors.right: parent.right

                    visible: (Qt.platform.os === "android" || Qt.platform.os === "ios")

                    text: qsTr("About app permissions")
                    source: "qrc:/IconLibrary/material-symbols/flaky.svg"
                    sourceSize: 24
                    indicatorSource: "qrc:/IconLibrary/material-symbols/chevron_right.svg"

                    onClicked: screenAboutPermissions.loadScreenFrom("ScreenAbout")
                }


                ListSeparator { }

                ListItemClickable { // supported formats
                    visible: false
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("Supported barcode formats")
                    source: "qrc:/IconLibrary/material-symbols/check_circle.svg"
                    sourceSize: 24
                    indicatorSource: "qrc:/IconLibrary/material-symbols/chevron_right.svg"

                    onClicked: screenAboutFormats.loadScreenFrom("ScreenAbout")
                }

                ListSeparator { }

                ////////

                Item { // list dependencies
                    visible: false
                    anchors.left: parent.left
                    anchors.leftMargin: EricTheme.componentMargin
                    anchors.right: parent.right
                    anchors.rightMargin: EricTheme.componentMargin

                    height: 40 + dependenciesText.height + dependenciesColumn.height

                    IconSvg {
                        visible: false
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.verticalCenter: dependenciesText.verticalCenter
                        width: 24
                        height: 24

                        source: "qrc:/IconLibrary/material-symbols/settings.svg"
                        color: EricTheme.colorSubText
                    }

                    Text {
                        id: dependenciesText
                        visible: false
                        enabled: false
                        anchors.top: parent.top
                        anchors.topMargin: 16
                        anchors.left: parent.left
                        anchors.leftMargin: appHeader.headerPosition - parent.anchors.leftMargin
                        anchors.right: parent.right
                        anchors.rightMargin: 8

                        text: qsTr("This application is made possible thanks to a couple of third party open source projects:")
                        textFormat: Text.PlainText
                        color: EricTheme.colorSubText
                        font.pixelSize: EricTheme.fontSizeContent
                        wrapMode: Text.WordWrap
                    }

                    Column {
                        id: dependenciesColumn
                        visible: false
                        anchors.top: dependenciesText.bottom
                        anchors.topMargin: 8
                        anchors.left: parent.left
                        anchors.leftMargin: appHeader.headerPosition - parent.anchors.leftMargin
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        spacing: 4

                        Repeater {
                            visible: false
                            model: [
                                "Qt6 (LGPL v3)",
                                "MobileUI (MIT)",
                                "MobileSharing (MIT)",
                                "qzxing (Apache v2)",
                                "zxing-cpp (Apache v2)",
                                "zint (BSD 3 clause)",
                                "Google Material Icons (Apache v2)",
                            ]
                            delegate: Text {
                                anchors.left: parent.left
                                anchors.right: parent.right

                                text: "- " + modelData
                                textFormat: Text.PlainText
                                color: EricTheme.colorSubText
                                font.pixelSize: EricTheme.fontSizeContent
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }

                ////////

                ListSeparatorPadded { visible: settingsManager.showDebug }

                Item { // debug infos
                    anchors.left: parent.left
                    anchors.leftMargin: EricTheme.componentMargin
                    anchors.right: parent.right
                    anchors.rightMargin: EricTheme.componentMargin

                    height: 16 + debugColumn.height
                    visible: settingsManager.showDebug

                    IconSvg {
                        anchors.top: debugColumn.top
                        anchors.topMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        width: 24
                        height: 24

                        source: "qrc:/IconLibrary/material-icons/duotone/info.svg"
                        color: EricTheme.colorSubText
                    }

                    Column {
                        id: debugColumn
                        anchors.left: parent.left
                        anchors.leftMargin: appHeader.headerPosition - parent.anchors.leftMargin
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.verticalCenter: parent.verticalCenter

                        spacing: EricTheme.componentMargin * 0.33

                        Text {
                            color: EricTheme.colorSubText
                            text: "App name: %1".arg(utilsApp.appName())
                            textFormat: Text.PlainText
                            font.pixelSize: EricTheme.fontSizeContent
                        }
                        Text {
                            color: EricTheme.colorSubText
                            text: "App version: %1".arg(utilsApp.appVersion())
                            textFormat: Text.PlainText
                            font.pixelSize: EricTheme.fontSizeContent
                        }
                        Text {
                            visible: false
                            color: EricTheme.colorSubText
                            text: "Backend(s): %1".arg(settingsManager.backend_reader) + (settingsManager.backend_zint ? " + zint" : "")
                            textFormat: Text.PlainText
                            font.pixelSize: EricTheme.fontSizeContent
                        }
                        Text {
                            visible: false
                            color: EricTheme.colorSubText
                            text: "Build mode: %1".arg(utilsApp.appBuildModeFull())
                            textFormat: Text.PlainText
                            font.pixelSize: EricTheme.fontSizeContent
                        }
                        Text {
                            visible: false
                            color: EricTheme.colorSubText
                            text: "Build architecture: %1".arg(utilsApp.qtArchitecture())
                            textFormat: Text.PlainText
                            font.pixelSize: EricTheme.fontSizeContent
                        }
                        Text {
                            color: EricTheme.colorSubText
                            text: "Build date: %1".arg(utilsApp.appBuildDateTime())
                            textFormat: Text.PlainText
                            font.pixelSize: EricTheme.fontSizeContent
                        }
                        Text {
                            visible: false
                            color: EricTheme.colorSubText
                            text: "Qt version: %1".arg(utilsApp.qtVersion())
                            textFormat: Text.PlainText
                            font.pixelSize: EricTheme.fontSizeContent
                        }
                    }
                }

                ListSeparatorPadded { visible: settingsManager.showDebug }


                ////////
            }
        }

        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
