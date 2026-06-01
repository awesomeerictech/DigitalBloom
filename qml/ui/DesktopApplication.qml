import QtQuick
import QtQuick.Controls
import QtQuick.Window

import ComponentLibrary
import DigitalBloom
import Local.LibArchive 1.0
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

ApplicationWindow {
    id: appWindow
    minimumWidth: 560
    minimumHeight: 800

    flags: Qt.Window | Qt.MaximizeUsingFullscreenGeometryHint
    color: EricTheme.colorBackground
    visible: true

    property bool isHdpi: (utilsScreen.screenDpi >= 128 || utilsScreen.screenPar >= 2.0)
    property bool isDesktop: true
    property bool isMobile: false
    property bool isPhone: false
    property bool isTablet: false

    property bool singleColumn: (width < height)
    property bool wideMode: (isDesktop && width >= 640) || (isTablet && width >= 480)

    property bool isLandscape: width > height


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



    // Desktop stuff ///////////////////////////////////////////////////////////

    width: {
        if (settingsManager.initialSize.width > 0)
            return settingsManager.initialSize.width
        else
            return isHdpi ? 960 : 1280
    }
    height: {
        if (settingsManager.initialSize.height > 0)
            return settingsManager.initialSize.height
        else
            return isHdpi ? 640 : 720
    }
    x: settingsManager.initialPosition.width
    y: settingsManager.initialPosition.height
    visibility: settingsManager.initialVisibility

    WindowGeometrySaver {
        windowInstance: appWindow
    }

    // LibArchiveBridge {
    //     Component.onCompleted: {
    //         // compress
    //         var ok = compressFolder("C:/Users/digital_bloom_ke/Downloads/Orders_files/App", "C:/Users/digital_bloom_ke/Downloads/Orders_files", "mybackup");
    //         console.log("compress ok:", ok);

    //         // // extract (call into your original extractor)
    //         // var ok2 = extractTarXzFromQrc(":/myserverassets/AppArchive", "C:/Users/digital_bloom_ke/Downloads/extract");
    //         // console.log("extract ok:", ok2);
    //     }
    // }

    // Mobile stuff ////////////////////////////////////////////////////////////

    property int screenOrientation: Screen.primaryOrientation
    property int screenOrientationFull: Screen.orientation

    property int screenPaddingStatusbar: 0
    property int screenPaddingNavbar: 0
    property int screenPaddingTop: 0
    property int screenPaddingLeft: 0
    property int screenPaddingRight: 0
    property int screenPaddingBottom: 0

    Item { // compat
        id: appHeader
        property int headerPosition: 64
    }
    Item { // compat
        id: mobileUI
        function setScreenAlwaysOn() {}
    }
    Item { // compat
        id: appDrawer
        visible: false
    }
    Item { // compat
        id: exitTimer
        property bool running: false
    }

    // Events handling /////////////////////////////////////////////////////////

    Component.onCompleted: {
        //screenBarcodeHistory.loadScreen(); return; // DEBUG
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


    Timer {
        id: orderserver
        interval: 2000
        running: false;
        repeat: false;
        onTriggered: { }
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
            screenBarcodeReader.backAction()
        } else if (appContent.state === "ScreenBarcodeWriter") {
            screenBarcodeWriter.backAction()
        } else if (appContent.state === "ScreenBarcodeHistory") {
            screenBarcodeHistory.backAction()
        } else if (appContent.state === "ScreenSettings") {
            screenSettings.backAction()
        } else if (appContent.state === "ScreenAbout" ||
                   appContent.state === "ScreenAboutFormats" ||
                   appContent.state === "ScreenAboutPermissions") {
            screenAbout.backAction()
        } else {
            backAction_default()
        }
    }
    function backAction_default() {
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

    Shortcut {
        sequence: StandardKey.FullScreen
        onActivated: {
            if (appWindow.visibility === Window.Windowed)
                appWindow.visibility = Window.FullScreen
            else
                appWindow.visibility = Window.Windowed
        }
    }
    Shortcut {
        sequence: StandardKey.Preferences
        onActivated: appContent.state = "settings"
    }
    Shortcut {
        sequences: [StandardKey.Close]
        onActivated: appWindow.close()
    }
    Shortcut {
        sequences: [StandardKey.Quit]
        onActivated: appWindow.close()
    }

    MouseArea {
        anchors.fill: parent

        enabled: isDesktop
        acceptedButtons: Qt.BackButton | Qt.ForwardButton
        onClicked: (mouse) => {
            if (mouse.button === Qt.BackButton) {
                backAction()
            } else if (mouse.button === Qt.ForwardButton) {
                forwardAction()
            }
        }
    }

    // QML /////////////////////////////////////////////////////////////////////

    DesktopSidebar {
        id: appSidebar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }

    FocusScope {
        id: appContent

        anchors.top: parent.top
        anchors.left: appSidebar.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom

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

        // Initial state
        state: "ScreenBarcodeReader"

        onStateChanged: {
            if (state === "ScreenBarcodeReader") {
                //
            } else {
                screenBarcodeReader.close()
            }
        }

        states: [
            State {
                name: "ScreenTutorial"
                PropertyChanges { target: screenTutorial; visible: true; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
            },

            State {
                name: "ScreenMainMenu"
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: true; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
            },
            State {
                name: "ScreenBarcodeReader"
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: true; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
            },
            State {
                name: "ScreenBarcodeWriter"
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: true; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
            },
            State {
                name: "ScreenBarcodeHistory"
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: true; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
            },

            State {
                name: "ScreenSettings"
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: true; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
            },
            State {
                name: "ScreenAbout"
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: true; }
                PropertyChanges { target: screenAboutFormats; visible: false; }
            },
            State {
                name: "ScreenAboutFormats"
                PropertyChanges { target: screenTutorial; visible: false; }
                PropertyChanges { target: screenMainMenu; visible: false; }
                PropertyChanges { target: screenBarcodeReader; visible: false; }
                PropertyChanges { target: screenBarcodeWriter; visible: false; }
                PropertyChanges { target: screenBarcodeHistory; visible: false; }
                PropertyChanges { target: screenSettings; visible: false; }
                PropertyChanges { target: screenAbout; visible: false; }
                PropertyChanges { target: screenAboutFormats; visible: true; }
            }
        ]
    }

    ////////////////////////////////////////////////////////////////////////////

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
          console.log("onShowDialogRequestedInfo signal in desktop application");
          showInfo(messageinfo);
      }
  }

  // Connection for the Error dialog
  Connections {
      target: qmlbackend
      onShowDialogRequestedError: (messageerror) => {
          console.log("onShowDialogRequestedError signal in desktop application");
          showError(messageerror);
      }
  }

  // Connection for the Toast message
  Connections {
      target: qmlbackend
      onShowDialogRequestedToast: (messagetoast) => {
          console.log("onShowDialogRequestedToast signal in desktop application");
          showToast(messagetoast);
      }
  }

}
