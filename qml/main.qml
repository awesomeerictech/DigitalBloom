import Qaterial as Qaterial
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QSyncable 1.0
import SortFilterProxyModel 0.2
import Quickeric 1.0
import "customs" as Custom
import Fluid as FluidControls
import Qt.labs.settings as QLab
import "qrc:/"
import "qrc:/qml"
import ComponentLibrary
import System 1.0
import StatusBar 0.1
import QtQuick.Controls.Material




App

{


  id: app
  width: 400
  height: 800
  visible: true
  api: Custom.Api{}
  urls: Custom.MyUrls{}


  StatusBar {

       id: theStatusBar
       theme: StatusBar.Dark
       color:  (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker("orange",1.5) :  Qt.darker("teal", 1.5)

       }



 property int myappTheme: Qaterial.Style.theme
 QLab.Settings { property alias myappTheme: app.myappTheme }
 readonly property bool lightTheme: app.myappTheme === Qaterial.Style.Theme.Light
 property bool isTabletInLandscape: isTablet && isLandscape
 property int safeWidth: width - unsafeArea.unsafeLeftMargin - unsafeArea.unsafeRightMargin - (isTabletInLandscape? drawerWidth : 0)
 property int safeHeight: height - unsafeArea.unsafeTopMargin - unsafeArea.unsafeBottomMargin
 property bool isLandscape: width > height
 property bool isHdpi: (utilsScreen.screenDpi > 128)
 property bool isDesktop: (Qt.platform.os !== "ios" && Qt.platform.os !== "android")
 property bool isMobile: (Qt.platform.os === "ios" || Qt.platform.os === "android")
 property bool isPhone: ((Qt.platform.os === "ios" || Qt.platform.os === "android") && (utilsApp.screenSize < 7.0))
 property bool isTablet: ((Qt.platform.os === "ios" || Qt.platform.os === "android") && (utilsApp.screenSize >= 7.0))
 property string datapathout : jsontools.datapath()
 // 1 = Qt.PortraitOrientation, 2 = Qt.LandscapeOrientation
  property int screenOrientation: Screen.primaryOrientation
  onScreenOrientationChanged: handleNotches()
  property int screenStatusbarPadding: 0
  property int screenNotchPadding: 0
  property int screenLeftPadding: 0
  property int screenRightPadding: 0
  // A top margin added to various components so that they're not covered
   // by the iPhone top notch
   property real safeTopMargin: 0
   property real safeBottomMargin: 0
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
 property int isDarkTheme:  app.myappTheme
 property real iconInactiveOpacity: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? 0.3 : 0.26
 property real iconActiveOpacity: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? 1.0 : 0.54
 property real opacitySubheading: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? 0.3 : 0.26
 property bool playing : false
 property int screenPaddingStatusbar: 0
 property int screenPaddingNotch: 0
 property int screenPaddingLeft: 0
 property int screenPaddingRight: 0
 property int screenPaddingBottom: 0


  function navi() {


       var ret =0

       if(navDrawer.visible && navDrawer.mobileAspect) {

           ret =1
       }

       else if(navDrawer.visible && !navDrawer.mobileAspect) {

           ret =2
       }

       return ret
   }


 title: qsTr("Digital Bloom")



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







 Component.onCompleted:  {

//  wintimermax.start()
   // if (Qt.platform.os !== "ios") return
   firstHandleNotches.restart()
   secondHandleNotches.restart()
   thirdHandleNotches.restart()
   // settingsManager.appTheme = EricTheme.THEME_DAY
    // settingsManager.appTheme = EricTheme.THEME_NIGHT
    EricSettings.wWidth = Qt.binding(function() {return width})
    EricSettings.wHeight = Qt.binding(function() {return height})
    Qaterial.Style.primaryColor = (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ?  "teal" : EricSettings.maroonFlush
    Qaterial.Style.accentColor= (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)
    settingsManager.appTheme =  app.lightTheme ? "THEME_DAY" : "THEME_NIGHT"
    EricTheme.loadTheme(settingsManager.appTheme)
    Qaterial.Style.theme = app.myappTheme
    let margins = System.getSafeAreaMargins(app)
    app.safeTopMargin = margins.top
    app.safeBottomMargin = margins.bottom
    console.log("top", app.safeTopMargin)
    console.log("bottom", app.safeBottomMargin)



 }



 Timer {

 id: wintimermax
 repeat: false
 interval: 1
 onTriggered: {

 // app.width=quickWindow.width
//   app.height=quickWindow.height
//   quickWindow.showFullScreen()
      }

  }


 Timer {
           id: firstHandleNotches
           interval: 100
           repeat: false
           onTriggered: handleNotches()
       }
       Timer {
           id: secondHandleNotches
           interval: 250
           repeat: false
           onTriggered: handleNotches()
       }
       Timer {
           id: thirdHandleNotches
           interval: 1000
           repeat: false
           onTriggered: handleNotches()
       }

       function handleNotches() {
           if (Qt.platform.os !== "ios") return
           if (typeof quickWindow === "undefined" || !quickWindow) return

           var screenPadding = (Screen.height - Screen.desktopAvailableHeight)
           console.log("screen width : " + Screen.width)
           console.log("screen height : " + Screen.height)
           console.log("screen avail  : " + Screen.desktopAvailableHeight)
           console.log("screen padding: " + screenPadding)

           var safeMargins = utilsScreen.getSafeAreaMargins(quickWindow)
           console.log("top:" + safeMargins["top"])
           console.log("right:" + safeMargins["right"])
           console.log("bottom:" + safeMargins["bottom"])
           console.log("left:" + safeMargins["left"])

           if (safeMargins["total"] !== safeMargins["top"]) {
               if (Screen.primaryOrientation === Qt.PortraitOrientation) {
                   screenStatusbarPadding = 20
                   screenNotchPadding = 12
               } else {
                   screenStatusbarPadding = 0
                   screenNotchPadding = 0
               }

               if (Screen.primaryOrientation === Qt.LandscapeOrientation) {
                   // TODO left or right ???
                   screenLeftPadding = 32
                   screenRightPadding = 0
               } else {
                   screenLeftPadding = 0
                   screenRightPadding = 0
               }
           } else {
               screenStatusbarPadding = 20
               screenNotchPadding = 0
           }
   /*
           console.log("RECAP screenStatusbarPadding:" + screenStatusbarPadding)
           console.log("RECAP screenNotchPadding:" + screenNotchPadding)
           console.log("RECAP screenLeftPadding:" + screenLeftPadding)
           console.log("RECAP screenRightPadding:" + screenRightPadding)

   */

           if (safeMargins["total"] === safeMargins["top"]) {
                       screenPaddingStatusbar = safeMargins["top"]
                       screenPaddingNotch = 0
                       screenPaddingLeft = 0
                       screenPaddingRight = 0
                       screenPaddingBottom = 0
                   } else if (safeMargins["total"] > 0) {
                       if (Screen.orientation === Qt.PortraitOrientation) {
                           screenPaddingStatusbar = 20
                           screenPaddingNotch = 12
                           screenPaddingLeft = 0
                           screenPaddingRight = 0
                           screenPaddingBottom = 6
                       } else if (Screen.orientation === Qt.InvertedPortraitOrientation) {
                           screenPaddingStatusbar = 12
                           screenPaddingNotch = 20
                           screenPaddingLeft = 0
                           screenPaddingRight = 0
                           screenPaddingBottom = 6
                       } else if (Screen.orientation === Qt.LandscapeOrientation) {
                           screenPaddingStatusbar = 0
                           screenPaddingNotch = 0
                           screenPaddingLeft = 32
                           screenPaddingRight = 0
                           screenPaddingBottom = 0
                       } else if (Screen.orientation === Qt.InvertedLandscapeOrientation) {
                           screenPaddingStatusbar = 0
                           screenPaddingNotch = 0
                           screenPaddingLeft = 0
                           screenPaddingRight = 32
                           screenPaddingBottom = 0
                       } else {
                           screenPaddingStatusbar = 0
                           screenPaddingNotch = 0
                           screenPaddingLeft = 0
                           screenPaddingRight = 0
                           screenPaddingBottom = 0
                       }
                   } else {
                       screenPaddingStatusbar = 0
                       screenPaddingNotch = 0
                       screenPaddingLeft = 0
                       screenPaddingRight = 0
                       screenPaddingBottom = 0
                   }
           /*
                   console.log("total:" + safeMargins["total"])
                   console.log("top:" + safeMargins["top"])
                   console.log("left:" + safeMargins["left"])
                   console.log("right:" + safeMargins["right"])
                   console.log("bottom:" + safeMargins["bottom"])

                   console.log("RECAP screenPaddingStatusbar:" + screenPaddingStatusbar)
                   console.log("RECAP screenPaddingNotch:" + screenPaddingNotch)
                   console.log("RECAP screenPaddingLeft:" + screenPaddingLeft)
                   console.log("RECAP screenPaddingRight:" + screenPaddingRight)
                   console.log("RECAP screenPaddingBottom:" + screenPaddingBottom)
           */


       }










  FluidControls.NavigationDrawer {
      id: navDrawer

      readonly property bool mobileAspect: app.width < 500

      modal: mobileAspect
      interactive: !(appContent.state==="accountsetuppage")
      position: mobileAspect ? 0.0 : 1.0
      visible: !mobileAspect && ( appContent.state ==="itemgridpage" || appContent.state ==="mepage" ||
     appContent.state ==="aboutpage" ||   appContent.state ==="qrpage" ||   appContent.state ==="literaturepage" ||
                                                      appContent.state ==="mediapage" )

      topContent: Image {
          source: navDrawer.mobileAspect ? "qrc:/images/materialbg.png" : ""
          width: parent.width
          height: navDrawer.mobileAspect ? 200 : 0
      }

      ScrollView {


          anchors.fill: parent

          clip: true

          ListView {
              id: navListView


              model: ListModel {



                  ListElement { category: qsTr("Account Info"); title: qsTr("Me") }
                  ListElement { category: qsTr("Your Qr Code"); title: qsTr("Qr") }
                  ListElement { category: qsTr("Video Clips"); title: qsTr("Clips") }
                  ListElement { category: qsTr("TV"); title: qsTr("Live Stream") }
                  ListElement { category: qsTr("Literature"); title: qsTr("Books") }
                  ListElement { category: qsTr("About Us"); title: qsTr("About") }


              }
              currentIndex: -1
              section.property: "category"
              section.criteria: ViewSection.FullString
              section.delegate: FluidControls.Subheader {
                  text: section
                  width: parent.width

                  FluidControls.ThinDivider {
                      width: parent.width
                      color: "teal"
                  }
              }
              delegate: FluidControls.ListItem {
                  mainText: model.title
                  maintextcolor: "teal"
                  highlighted: ListView.isCurrentItem
                  onClicked: {
                      navListView.currentIndex = index;
                      if(model.title==="Me") {

                          appContent.state ="mepage"

                      }

                      else if(model.title==="Qr") {

                          appContent.state ="qrpage"

                      }

                      else if(model.title==="Live Stream") {

                          appContent.state ="mediapage"

                      }

                      else if(model.title==="Books") {

                            appContent.state ="literaturepage"

                      }

                      else if(model.title==="About") {

                            appContent.state ="aboutpage"

                      }

                      else if(model.title==="Clips") {


                           appContent.state ="clipspage"

                      }



                      if (navDrawer.modal)
                          navDrawer.close();
                  }
              }
          }
      }
  }




  initialPage: FluidControls.Page {


         id: myinitialpage
         title: "awaitdev"
         font.family: "FuturaPTBook"
         font.pixelSize: 25
         x: (appContent.state==="accountsetuppage") ? 0 : navDrawer.position * navDrawer.width
         width: app.width - x



         leftAction: FluidControls.Action {


            icon.source: {

                if((appContent.state ==="mepage" || appContent.state ==="aboutpage" ||
                    appContent.state ==="qrpage" ||   appContent.state ==="literaturepage"
                    || appContent.state ==="mediapage" ||  appContent.state ==="clipspage"  ) ) {

                    FluidControls.Utils.iconUrl("navigation/arrow_back")


                }

                else if(appContent.state ==="Tutorial") {


                    FluidControls.Utils.iconUrl("navigation/close")

                }

                else if(appContent.state ==="itemgridpage") {

                     FluidControls.Utils.iconUrl("navigation/menu")

                }
            }



             visible: !(appContent.state ==="accountsetuppage")
             onTriggered: {

                 if((appContent.state ==="mepage" || appContent.state ==="aboutpage" ||
                     appContent.state ==="qrpage" ||   appContent.state ==="literaturepage" ||
                     appContent.state ==="mediapage" ||  appContent.state ==="clipspage"  )) {


                     appContent.state ="itemgridpage"
                     navDrawer.visible ? navDrawer.close() : navDrawer.open()





                 }

               if(appContent.state ==="itemgridpage") {

                     navDrawer.visible ? navDrawer.close() : navDrawer.open()


                 }


                  if(appContent.state ==="Tutorial") {

                      appContent.state= screenTutorial.entryPoint

                 }






             }
         }



         actions: [
             FluidControls.Action {

                 visible: ( appContent.state ==="itemgridpage" || appContent.state ==="mepage" ||
    appContent.state ==="aboutpage" ||   appContent.state ==="qrpage" ||   appContent.state ==="literaturepage" ||
                         appContent.state ==="mediapage" ||  appContent.state ==="clipspage" )
                 text: qsTr("Theme")
                 icon.source: app.lightTheme ? Qaterial.Icons.weatherSunny : Qaterial.Icons.moonWaningCrescent
                 toolTip: qsTr("Theme")
                 onTriggered: {

                    app.myappTheme = app.lightTheme ? Qaterial.Style.Theme.Dark : Qaterial.Style.Theme.Light
                    Qaterial.Style.theme = app.myappTheme
                    settingsManager.appTheme =  app.lightTheme ? "THEME_DAY" : "THEME_NIGHT"
                     EricTheme.loadTheme(settingsManager.appTheme)
                    Qaterial.Style.primaryColor = (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ?  "teal" : EricSettings.maroonFlush
                    Qaterial.Style.accentColor= (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)
                   // myappbar.Material.background = Qaterial.Style.accentColor
                  //myappbar.Material.theme = Qaterial.Style.primaryColor

                 }
             },
             FluidControls.Action {

                 visible: ( appContent.state ==="itemgridpage" || appContent.state ==="mepage" ||
    appContent.state ==="aboutpage" ||   appContent.state ==="qrpage" ||   appContent.state ==="literaturepage" ||
                         appContent.state ==="mediapage" ||  appContent.state ==="clipspage"  )
                 text: qsTr("Log Out")
                 icon.source: Qaterial.Icons.logout
                 toolTip: qsTr("Log Out")
                 onTriggered: {

                     appContent.state= "accountsetuppage"


                 }
             }
         ]


       FocusScope {

       id: appContent
       anchors.fill: parent

       focus: true

        Keys.onEscapePressed: {


         if (appContent.state === "Tutorial") return // do nothing

            if (appContent.state === "accountsetuppage") {

                 if (exitTimer.running)
                       mymedia.doclosewin()
                  else
                      exitTimer.start()

                   }

             else if (( appContent.state ==="itemgridpage" || appContent.state ==="mepage" ||
                       appContent.state ==="aboutpage" ||   appContent.state ==="qrpage" ||
                       appContent.state ==="literaturepage" || appContent.state ==="mediapage"
                       ||  appContent.state ==="clipspage" )) {


                       appContent.state = "accountsetuppage"

                 }


        }

       Keys.onBackPressed: {
          if (appContent.state === "Tutorial") return // do nothing

          if (appContent.state === "accountsetuppage") {

               if (exitTimer.running)
                     mymedia.doclosewin()
                else
                    exitTimer.start()

                 }

           else if (( appContent.state ==="itemgridpage" || appContent.state ==="mepage" ||
                     appContent.state ==="aboutpage" ||   appContent.state ==="qrpage" ||
                     appContent.state ==="literaturepage" || appContent.state ==="mediapage" ||
                     appContent.state ==="clipspage" )) {


                     appContent.state = "accountsetuppage"

               }




       }


       Accounts {

          id: myaccounts
          anchors.fill: parent

               }


        ItemGrid {

        id: myitemgrid
        anchors.fill: parent




         }

       Me {

          id: myme
          anchors.fill: parent


          }


       Qr {

         id: myqr
         anchors.fill: parent


          }



   /*   Literature {

         id: myliterature
         anchors.fill: parent


         } */

      Media {

        id: mymedia
        anchors.fill: parent

        } 


      About {

        id: myabout
        anchors.fill: parent


         }

      Tutorial {

        anchors.fill: parent
        id: screenTutorial

             }

      Clips {

      anchors.fill: parent
      id: myclips

      }






   Component.onCompleted: {


        screenTutorial.open()


         }


   state: "accountsetuppage"

   onStateChanged: {


       console.log("state changed to: "+ appContent.state)
       if(appContent.state==="mediapage") {

           mymedia.playmedia()
           app.playing=true

         }

       else {

          if(app.playing) {

          mymedia.stopmedia()
          app.playing=false

           }


       } 

        }

  states: [

  State {
   name: "Tutorial"
   PropertyChanges { target: screenTutorial; visible: true; enabled: true; }
   PropertyChanges { target: myinitialpage; title:  screenTutorial.pagename; }
   PropertyChanges { target: myaccounts; visible: true; enabled: true }
   PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
   PropertyChanges { target: myme; visible: false; enabled: false; }
   PropertyChanges { target: myqr; visible: false; enabled: false; }
 //  PropertyChanges { target: myliterature; visible: false; enabled: false; }
  PropertyChanges { target: mymedia; visible: false; enabled: false; }
   PropertyChanges { target: myabout; visible: false; enabled: false; }
   PropertyChanges { target: myclips; visible: false; enabled: false; }

                   },
  State {

  name: "accountsetuppage"
  PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
  PropertyChanges { target: myinitialpage; title:  myaccounts.pagename; }
  PropertyChanges { target: myaccounts; visible: true; enabled: true }
  PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
  PropertyChanges { target: myme; visible: false; enabled: false; }
  PropertyChanges { target: myqr; visible: false; enabled: false; }
//  PropertyChanges { target: myliterature; visible: false; enabled: false; }
  PropertyChanges { target: mymedia; visible: false; enabled: false; }
  PropertyChanges { target: myabout; visible: false; enabled: false; }
  PropertyChanges { target: myclips; visible: false; enabled: false; }

       },
  State {

   name: "itemgridpage"
   PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
   PropertyChanges { target: myinitialpage; title:  myitemgrid.pagename; }
   PropertyChanges { target: myaccounts; visible: false; enabled: false }
   PropertyChanges { target: myitemgrid; visible: true; enabled: true; }
   PropertyChanges { target: myme; visible: false; enabled: false; }
   PropertyChanges { target: myqr; visible: false; enabled: false; }
 //  PropertyChanges { target: myliterature; visible: false; enabled: false; }
   PropertyChanges { target: mymedia; visible: false; enabled: false; }
   PropertyChanges { target: myabout; visible: false; enabled: false; }
   PropertyChanges { target: myclips; visible: false; enabled: false; }

        },

 State {

 name: "mepage"
 PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
 PropertyChanges { target: myinitialpage; title:  myme.pagename; }
 PropertyChanges { target: myaccounts; visible: false; enabled: false }
 PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
 PropertyChanges { target: myme; visible: true; enabled: true; }
 PropertyChanges { target: myqr; visible: false; enabled: false; }
// PropertyChanges { target: myliterature; visible: false; enabled: false; }
 PropertyChanges { target: mymedia; visible: false; enabled: false; }
 PropertyChanges { target: myabout; visible: false; enabled: false; }
 PropertyChanges { target: myclips; visible: false; enabled: false; }

            },

 State {

 name: "qrpage"
 PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
 PropertyChanges { target: myinitialpage; title:  myqr.pagename; }
 PropertyChanges { target: myaccounts; visible: false; enabled: false }
 PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
 PropertyChanges { target: myme; visible: false; enabled: false; }
 PropertyChanges { target: myqr; visible: true; enabled: true; }
// PropertyChanges { target: myliterature; visible: false; enabled: false; }
 PropertyChanges { target: mymedia; visible: false; enabled: false; }
 PropertyChanges { target: myabout; visible: false; enabled: false; }
 PropertyChanges { target: myclips; visible: false; enabled: false; }

                 },
/*State {

name: "literaturepage"
PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
PropertyChanges { target: myinitialpage; title:  myliterature.pagename; }
PropertyChanges { target: myaccounts; visible: false; enabled: false }
PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
PropertyChanges { target: myme; visible: false; enabled: false; }
PropertyChanges { target: myqr; visible: false; enabled: false; }
PropertyChanges { target: myliterature; visible: true; enabled: true; }
//PropertyChanges { target: mymedia; visible: false; enabled: false; }
PropertyChanges { target: myabout; visible: false; enabled: false; }
PropertyChanges { target: myclips; visible: false; enabled: false; }

                      }, */

 State {

 name: "mediapage"
 PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
 PropertyChanges { target: myinitialpage; title:  mymedia.pagename; }
 PropertyChanges { target: myaccounts; visible: false; enabled: false }
 PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
 PropertyChanges { target: myme; visible: false; enabled: false; }
 PropertyChanges { target: myqr; visible: false; enabled: false; }
 //PropertyChanges { target: myliterature; visible: false; enabled: false; }
 PropertyChanges { target: mymedia; visible: true; enabled: true; }
 PropertyChanges { target: myabout; visible: false; enabled: false; }
 PropertyChanges { target: myclips; visible: false; enabled: false; }

                      }, 
State {

name: "aboutpage"
PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
PropertyChanges { target: myinitialpage; title:  myabout.pagename; }
PropertyChanges { target: myaccounts; visible: false; enabled: false }
PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
PropertyChanges { target: myme; visible: false; enabled: false; }
PropertyChanges { target: myqr; visible: false; enabled: false; }
//PropertyChanges { target: myliterature; visible: false; enabled: false; }
PropertyChanges { target: mymedia; visible: false; enabled: false; }
PropertyChanges { target: myabout; visible: true; enabled: true; }
PropertyChanges { target: myclips; visible: false; enabled: false; }

                      },

State {

name: "clipspage"
PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
PropertyChanges { target: myinitialpage; title:  myclips.pagename; }
PropertyChanges { target: myaccounts; visible: false; enabled: false }
PropertyChanges { target: myitemgrid; visible: false; enabled: false; }
PropertyChanges { target: myme; visible: false; enabled: false; }
PropertyChanges { target: myqr; visible: false; enabled: false; }
//PropertyChanges { target: myliterature; visible: false; enabled: false; }
PropertyChanges { target: mymedia; visible: false; enabled: false; }
PropertyChanges { target: myabout; visible: false; enabled: false; }
PropertyChanges { target: myclips; visible: true; enabled: true; }

                            }

   ]


    }


  }






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

        Connections {
               target: Qt.application
               function onStateChanged() {
                   switch (Qt.application.state) {
                       case Qt.ApplicationSuspended:
                           console.log("QML says Qt.ApplicationSuspended")
                           break
                       case Qt.ApplicationHidden:
                           console.log("QML says Qt.ApplicationHidden")
                           break
                       case Qt.ApplicationInactive:
                           console.log("QML says Qt.ApplicationInactive")
                           break
                       case Qt.ApplicationActive:
                           console.log("QML says Qt.ApplicationActive")

                           // Check if we need an 'automatic' theme change
                           EricTheme.loadTheme(settingsManager.appTheme)

                           break
                   }
               }
           }


        Timer {
               id: exitTimer
               interval: 3000
               running: false
               repeat: false
               onRunningChanged: exitWarning.opacity = running
           }




        Text {
                id: exitWarning
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 32
                anchors.horizontalCenter: parent.horizontalCenter

                visible: opacity
                opacity: 0
                Behavior on opacity { OpacityAnimator { duration: 333 } }

                text: qsTr("Press one more time to exit...")
                textFormat: Text.PlainText
                font.pixelSize: EricTheme.fontSizeContent
                color: EricTheme.colorForeground

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -8
                    z: -1
                    radius: 4
                    color: EricTheme.colorSubText
                }
        }


        appBar {

         id: myappbar
         maxActionCount: 3
         appBarHeight: 50

         //Material.background: Qaterial.Style.primaryColor
         //Material.theme: Qaterial.Style.accentColor


        }



  }





