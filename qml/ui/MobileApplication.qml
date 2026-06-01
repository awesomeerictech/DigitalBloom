import QtQuick
import QtQuick.Controls
import QtQuick.Window

import ComponentLibrary
import DigitalBloom
import MobileUI
import Qaterial as Qaterial
import QtQml
import QtQuick.Layouts
import QSyncable 1.0
import SortFilterProxyModel 0.2
import Quickeric 1.0
//import Fluid as FluidControls
import Qt.labs.settings as QLab
import "qrc:/"
import "qrc:/qml"

Window {
    id: appWindow
    minimumWidth: 480
    minimumHeight: 800

    flags: Qt.Window | Qt.MaximizeUsingFullscreenGeometryHint
    color: EricTheme.colorBackground
    visible: true

    property bool isHdpi: (utilsScreen.screenDpi >= 128 || utilsScreen.screenPar >= 2.0)
    property bool isLandscape: width > height
    property bool isDesktop: (Qt.platform.os !== "android" && Qt.platform.os !== "ios")
    property bool isMobile: (Qt.platform.os === "android" || Qt.platform.os === "ios")
    property bool isPhone: ((Qt.platform.os === "android" || Qt.platform.os === "ios") && (utilsScreen.screenSize < 7.0))
    property bool isTablet: ((Qt.platform.os === "android" || Qt.platform.os === "ios") && (utilsScreen.screenSize >= 7.0))

    property bool wideMode: (isDesktop && width >= 640) || (isTablet && width >= 480)
    property bool singleColumn: (width < height)
    property int myappTheme: Qaterial.Style.theme
    QLab.Settings { property alias myappTheme: appWindow.myappTheme }
    readonly property bool lightTheme: appWindow.myappTheme === Qaterial.Style.Theme.Light
    // Material.primary: Qaterial.Style.primaryColor
    // Material.accent: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)
   property color myforegroundColor: (Qaterial.Style.theme=== Qaterial.Style.Theme.Light) ? Qaterial.Style.textColorLight  : Qaterial.Style.textColorDark
   property color dropShadow: Material.dropShadowColor
   // font sizes - defaults from Google Material Design Guide
   property int fontSizeDisplay4: 112
   property int fontSizeDisplay3: 56
   property int fontSizeDisplay2: 45
   property int fontSizeDisplay1: 34
   property int fontSizeHeadline: 24
   property int fontSizeTitle: 20
   property int fontSizeSubheading: 22
   property int fontSizeBodyAndButton: 14 // is Default
   property int fontSizeCaption: 12
   property int fontSizeActiveNavigationButton: 14
   property int fontSizeInactiveNavigationButton: 12
   property real opacityToggleInactive: 0.2
   property real opacityToggleActive: 1.0
   property color toastColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ?  "Darkgrey" : "#323232"
   property real toastOpacity: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? "0.9" : "0.75"
   property color primaryColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ?  "teal" : EricSettings.maroonFlush
   property color accentColor:  (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)
   property real opacityTitle: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? 1.0 : 0.7
   property bool backKeyfreezed: false
   property bool modalPopupActive: false
   property bool modalMenuOpen: false
   property var myLocale: Qt.locale()
   property int isDarkTheme:  appWindow.myappTheme
   property real iconInactiveOpacity: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? 0.3 : 0.26
   property real iconActiveOpacity: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? 1.0 : 0.54
   property real opacitySubheading: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? 0.3 : 0.26
   property bool playing : false
    // Mobile stuff ////////////////////////////////////////////////////////////

    // 1 = Qt.PortraitOrientation, 2 = Qt.LandscapeOrientation
    // 4 = Qt.InvertedPortraitOrientation, 8 = Qt.InvertedLandscapeOrientation
    property int screenOrientation: Screen.primaryOrientation
    property int screenOrientationFull: Screen.orientation

    property int screenPaddingStatusbar: 0
    property int screenPaddingNavbar: 0

    property int screenPaddingTop: 0
    property int screenPaddingLeft: 0
    property int screenPaddingRight: 0
    property int screenPaddingBottom: 0


    // Loads Material icons font
     FontLoader {
         source: "qrc:/assets/fonts/MaterialIcons-Regular.ttf"
     }
     FontLoader {
         source: "qrc:/assets/fonts/FuturaPTMedium.otf"
     }
     FontLoader {
         source: "qrc:/assets/fonts/FuturaPTBold.otf"
     }
     FontLoader {
         source: "qrc:/assets/fonts/FuturaPTBook.otf"
     }
     FontLoader {
         source: "qrc:/assets/fonts/FuturaPTLightOblique.otf"
     }
     FontLoader {
         source: "qrc:/assets/fonts/EraIcons.ttf"
     }


    Connections {
        target: Screen
        function onOrientationChanged() {
            mobileUI.handleSafeAreas()
            rotateTimer1.start()
            rotateTimer2.start()
            rotateTimer3.start()
        }
    }
    Connections {
        target: Theme
        function onCurrentThemeChanged() { mobileUI.handleSafeAreas() }
    }

    Timer {
        id: rotateTimer1
        interval: 40
        running: false; repeat: false;
        onTriggered: { mobileUI.handleSafeAreas() }
    }
    Timer {
        id: rotateTimer2
        interval: 128
        running: false; repeat: false;
        onTriggered: { mobileUI.handleSafeAreas() }
    }
    Timer {
        id: rotateTimer3
        interval: 256
        running: false; repeat: false;
        onTriggered: { mobileUI.handleSafeAreas() }
    }

    MobileUI {
        id: mobileUI

        statusbarColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ?  "teal" : EricSettings.maroonFlush
        statusbarTheme: EricTheme.THEME_NIGHT

        navbarColor: "transparent"
        navbarTheme: (appContent.state === "ScreenBarcodeReader") ? MobileUI.Dark : EricTheme.themeStatusbar

        Component.onCompleted: {

            // settingsManager.appTheme = EricTheme.THEME_DAY
                // settingsManager.appTheme = EricTheme.THEME_NIGHT
                EricSettings.wWidth = Qt.binding(function() {return width})
                EricSettings.wHeight = Qt.binding(function() {return height})
                Qaterial.Style.primaryColor = (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ?  "teal" : EricSettings.maroonFlush
                Qaterial.Style.accentColor= (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)
                settingsManager.appTheme =  appWindow.lightTheme ? "THEME_DAY" : "THEME_RAIN"
                EricTheme.loadTheme(settingsManager.appTheme)
                Qaterial.Style.theme = appWindow.myappTheme
                settingsManager.saveSettings()
            handleSafeAreas()
            mystorage.start()

        }

        function handleSafeAreas() {
            // safe areas handling is a work in progress /!\
            // safe areas are only taken into account when using maximized geometry / full screen mode

            mobileUI.refreshUI() // hack

            mobileUI.statusbarTheme = EricTheme.themeStatusbar // hack

            if (appWindow.visibility === Window.FullScreen ||
                appWindow.flags & Qt.MaximizeUsingFullscreenGeometryHint) {

                screenPaddingStatusbar = mobileUI.statusbarHeight
                screenPaddingNavbar = mobileUI.navbarHeight

                screenPaddingTop = mobileUI.safeAreaTop
                screenPaddingLeft = mobileUI.safeAreaLeft
                screenPaddingRight = mobileUI.safeAreaRight
                screenPaddingBottom = mobileUI.safeAreaBottom

                // hacks
                if (Qt.platform.os === "android") {
                    if (appWindow.visibility === Window.FullScreen) {
                        screenPaddingStatusbar = 0
                        screenPaddingNavbar = 0
                    }
                    if (appWindow.flags & Qt.MaximizeUsingFullscreenGeometryHint) {
                        if (mobileUI.isPhone) {
                            if (Screen.orientation === Qt.LandscapeOrientation) {
                                screenPaddingLeft = screenPaddingStatusbar
                                screenPaddingRight = screenPaddingNavbar
                                screenPaddingNavbar = 0
                            } else if (Screen.orientation === Qt.InvertedLandscapeOrientation) {
                                screenPaddingLeft = screenPaddingNavbar
                                screenPaddingRight = screenPaddingStatusbar
                                screenPaddingNavbar = 0
                            }
                        }
                    }
                }
                // hacks
                if (Qt.platform.os === "ios") {
                    if (appWindow.visibility === Window.FullScreen) {
                        screenPaddingStatusbar = 0
                    }
                }
            } else {
                screenPaddingStatusbar = 0
                screenPaddingNavbar = 0
                screenPaddingTop = 0
                screenPaddingLeft = 0
                screenPaddingRight = 0
                screenPaddingBottom = 0
            }
/*
            console.log("> handleSafeAreas()")
            console.log("- window mode:         " + appWindow.visibility)
            console.log("- window flags:        " + appWindow.flags)
            console.log("- screen dpi:          " + Screen.devicePixelRatio)
            console.log("- screen width:        " + Screen.width)
            console.log("- screen width avail:  " + Screen.desktopAvailableWidth)
            console.log("- screen height:       " + Screen.height)
            console.log("- screen height avail: " + Screen.desktopAvailableHeight)
            console.log("- screen orientation (full): " + Screen.orientation)
            console.log("- screen orientation (primary): " + Screen.primaryOrientation)
            console.log("- screenSizeStatusbar: " + screenPaddingStatusbar)
            console.log("- screenSizeNavbar:    " + screenPaddingNavbar)
            console.log("- screenPaddingTop:    " + screenPaddingTop)
            console.log("- screenPaddingLeft:   " + screenPaddingLeft)
            console.log("- screenPaddingRight:  " + screenPaddingRight)
            console.log("- screenPaddingBottom: " + screenPaddingBottom)
*/
        }
    }

    MobileHeader {
        id: appHeader

        visible: (appContent.state !== "ScreenBarcodeReader")
    }

    MobileDrawer {
        id: appDrawer

        interactive: (appContent.state !== "Tutorial")
    }

    // Events handling /////////////////////////////////////////////////////////

    Component.onCompleted: {


        if (settingsManager.defaultTab === "writer") {
            screenBarcodeWriter.loadScreen()
        } else {
            screenBarcodeReader.loadScreen()
        }
    }

    Connections {
        target: Qt.application
        function onStateChanged() {
            switch (Qt.application.state) {
                case Qt.ApplicationSuspended:
                    //console.log("Qt.ApplicationSuspended")
                    screenBarcodeReader.close()
                    break
                case Qt.ApplicationHidden:
                    //console.log("Qt.ApplicationHidden")
                    break
                case Qt.ApplicationInactive:
                    //console.log("Qt.ApplicationInactive")
                    break
                case Qt.ApplicationActive:
                    //console.log("Qt.ApplicationActive")

                    if (appContent.state === "ScreenBarcodeReader")
                        screenBarcodeReader.loadScreen()

                    // Check if we need an 'automatic' theme change
                    EricTheme.loadTheme(settingsManager.appTheme)

                    break
            }
        }
    }

    Connections {
        target: appHeader
        function onLeftMenuClicked() {
            if (appHeader.leftMenuMode === "drawer") {
                appDrawer.open()
            } else if (appHeader.leftMenuMode === "close") {
                appContent.state = screenTutorial.entryPoint
            } else {
                backAction()
            }
        }
        function onRightMenuClicked() {
            //
        }
    }

    function backAction() {
        //console.log("backAction() backAction() backAction() backAction()")

        if (appContent.state === "ScreenTutorial") {
            if (screenTutorial.entryPoint === "ScreenBarcodeReader") {
                return // do nothing
            } else {
                appContent.state = screenTutorial.entryPoint
                return
            }
        }

        if (appContent.state === "ScreenBarcodeReader") {
            //screenBarcodeReader.backAction()
            backAction_default()
        } else if (appContent.state === "ScreenBarcodeWriter") {
            screenBarcodeWriter.backAction()
        } else if (appContent.state === "ScreenBarcodeHistory") {
            screenBarcodeHistory.backAction()
        } else if (appContent.state === "ScreenAbout" ||
                   appContent.state === "ScreenAboutFormats" ||
                   appContent.state === "ScreenAboutPermissions") {
            screenAbout.backAction()
        } else {
            backAction_default()
        }
    }
    function backAction_default() {
        if ((appContent.state === "ScreenBarcodeReader" && settingsManager.defaultTab === "reader") ||
            (appContent.state === "ScreenBarcodeWriter" && settingsManager.defaultTab === "writer")) {
            if (exitTimer.running)
                Qt.quit()
            else
                exitTimer.start()
        }

        if (settingsManager.defaultTab === "reader")
            screenBarcodeReader.loadScreen()
        else if (settingsManager.defaultTab === "writer")
            screenBarcodeWriter.loadScreen()
    }
    function forwardAction() {
        //console.log("forwardAction() forwardAction() forwardAction() forwardAction()")
    }

    Shortcut {
        sequences: [StandardKey.Back]
        onActivated: backAction()
    }
    Shortcut {
        sequences: [StandardKey.Forward]
        onActivated: forwardAction()
    }

    Timer {
        id: exitTimer
        interval: 3333
        repeat: false

    }

    Timer {
        id: mystorage
        interval: 2000
        repeat: false
        onTriggered: {
             utilsApp.checkMobileStoragePermissions()
        }
    }



    // QML /////////////////////////////////////////////////////////////////////

    FocusScope {
        id: appContent

        anchors.top: (appContent.state === "ScreenBarcodeReader") ? parent.top : appHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: screenPaddingBottom

        focus: true
        Keys.onBackPressed: backAction()

        ScreenTutorial {
            id: screenTutorial
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }
        ScreenMainMenu {
            id: screenMainMenu
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }

        ScreenBarcodeReader {
            id: screenBarcodeReader
        }
        ScreenBarcodeHistory {
            id: screenBarcodeHistory
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }
        ScreenBarcodeWriter {
            id: screenBarcodeWriter
            enabled: true
            visible: true
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }

        ScreenSettings {
            id: screenSettings
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }
        ScreenAbout {
            id: screenAbout
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }
        ScreenAboutFormats {
            id: screenAboutFormats
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }
        MobilePermissions {
            id: screenAboutPermissions
            anchors.bottomMargin: screenPaddingBottom + screenPaddingNavbar
        }

        // Initial state
        state: "ScreenBarcodeReader"

        onStateChanged: {
            if (state === "ScreenBarcodeReader")
                appHeader.leftMenuMode = "drawer"
            else if (state === "ScreenTutorial")
                appHeader.leftMenuMode = "close"
            else
                appHeader.leftMenuMode = "back"

            if (state === "ScreenBarcodeReader") {
                //
            } else {
                screenBarcodeReader.close()
            }
        }

        states: [
            State {
                name: "ScreenTutorial"
                PropertyChanges { target: appHeader; headerTitle: utilsApp.appName(); }
                PropertyChanges { target: screenTutorial; visible: true; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },

            State {
                name: "ScreenMainMenu"
                PropertyChanges { target: appHeader; headerTitle: utilsApp.appName(); }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: true; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },
            State {
                name: "ScreenBarcodeReader"
                PropertyChanges { target: appHeader; headerTitle: ""; }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: true; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },
            State {
                name: "ScreenBarcodeWriter"
                PropertyChanges { target: appHeader; headerTitle: qsTr("Qr Code Writer"); }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: true; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },
            State {
                name: "ScreenBarcodeHistory"
                PropertyChanges { target: appHeader; headerTitle: qsTr("Qr Menu"); }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: true; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },

            State {
                name: "ScreenSettings"
                PropertyChanges { target: appHeader; headerTitle: qsTr("Settings"); }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: true; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },
            State {
                name: "ScreenAbout"
                PropertyChanges { target: appHeader; headerTitle: qsTr("About"); }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: true; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },
            State {
                name: "ScreenAboutFormats"
                PropertyChanges { target: appHeader; headerTitle: qsTr("About formats"); }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: true; }
                PropertyChanges { target: screenAboutPermissions; visible: false; }
            },
            State {
                name: "ScreenAboutPermissions"
                PropertyChanges { target: appHeader; headerTitle: qsTr("About permissions"); }
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
                PropertyChanges { target: screenAboutPermissions; visible: true; }
            }
        ]
    }

    ////////////////

    Rectangle { // navbar area
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: screenPaddingNavbar

        visible: (appContent.state !== "ScreenBarcodeReader")
        color: {
            if (appContent.state === "ScreenTutorial") return EricTheme.colorHeader
            return EricTheme.colorBackground
        }
    }

    ////////////////



    function showToast(info) {

        popupToast.start(info)

    }

    function showError(info) {

        popupError.start(info)

    }

    function showInfo(info) {

        popupInfo.text = info
        popupInfo.buttonText = qsTr("OK")
        popupInfo.open()

    }


    function showYesNo(info) {

        yesno.text = info
        yesno.open()

    }

    function showUpdate(info) {

        myupdate.text = info
        myupdate.open()

    }


     function resetFocus() {



      }

     PopupYesNo {

        id: yesno
        onAboutToHide: {

            resetFocus()
        }


     }
     PopupUpdate {

         id: myupdate
         onAboutToHide: {

             resetFocus()
         }

     }


  PopupInfo {
      id: popupInfo
      onAboutToHide: {
          popupInfo.stopTimer()
          resetFocus()
      }
  } // popupInfo
  // PopupToast
  PopupToast {
      id: popupToast
      onAboutToHide: {
          resetFocus()
      }
  } // popupToast
  // PopupToast
  PopupError {
      id: popupError
      onAboutToHide: {
          resetFocus()
      }
  } // popupError
  
  
  // Connection for the Info dialog
  Connections {
      target: qmlbackend
      onShowDialogRequestedInfo: (messageinfo) => {
          console.log("onShowDialogRequestedInfo signal in mobile application");
          showInfo(messageinfo);
      }
  }

  // Connection for the Error dialog
  Connections {
      target: qmlbackend
      onShowDialogRequestedError: (messageerror) => {
          console.log("onShowDialogRequestedError signal in mobile application");
          showError(messageerror);
      }
  }

  // Connection for the Toast message
  Connections {
      target: qmlbackend
      onShowDialogRequestedToast: (messagetoast) => {
          console.log("onShowDialogRequestedToast signal in mobile application");
          showToast(messagetoast);
      }
  }
	
}
