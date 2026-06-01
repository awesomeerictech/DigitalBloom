import Qaterial as Qaterial
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ComponentLibrary
import "qrc:/"
import "qrc:/qml"
import "qrc:/js/UtilsNumber.js" as UtilsNumber

Page {

    id: ericme
    property string pagename : "About Us"
    anchors.fill: parent
    anchors.topMargin: 20
    anchors.leftMargin: screenLeftPadding
    anchors.rightMargin: screenRightPadding
    focus: true

    ScrollView {

    id: scrollView
    contentWidth: -1
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    ScrollBar.vertical.interactive: true
    ScrollBar.horizontal.interactive : true
    property bool isLoading: false
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    Column {

     id: column
     anchors.fill: parent
     anchors.leftMargin: 16
     anchors.rightMargin: 16
     topPadding: 8
     bottomPadding: 8
     spacing: 8

     Item {

      id: logo
      height: 80
      anchors.right: parent.right
      anchors.rightMargin: 0
      anchors.left: parent.left
      anchors.leftMargin: 0

      Image {

        id: imageLogo
        width: 96
        height: 96
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        source: "qrc:/myassets/gfx/logos/DigitalBloom.svg"
        sourceSize: Qt.size(width, height)

                }

       Text {

       id: textVersion
       anchors.left: imageLogo.right
       anchors.leftMargin: 18
       anchors.bottom: parent.bottom
       anchors.bottomMargin: 8
       color: Qaterial.Style.textColor
       font.family: "FuturaPTBook"
       font.pixelSize: 20
       text: qsTr("version %1").arg(utilsApp.appVersion())


                }

         Text {

         id: textName
         anchors.top: parent.top
         anchors.topMargin: 18
         anchors.left: imageLogo.right
         anchors.leftMargin: 16
         text: utilsApp.appName()
         color: EricTheme.colorText
         font.family: "FuturaPTBook"
         font.pixelSize: 28
                              }
                          }



     Row {

      id: buttonsRow
      height: 56
      anchors.left: parent.left
      anchors.leftMargin: 0
      anchors.right: parent.right
      anchors.rightMargin: 0
      visible: true
      spacing: 16

       onWidthChanged: {

         var ww = (scrollView.width - 48 - screenLeftPadding - screenRightPadding) / 2;
         if (ww > 0) { websiteBtn.width = ww; githubBtn.width = ww; }

                  }

      ButtonWireframeIconCentered {

       id: websiteBtn
       width: 180
       anchors.verticalCenter: parent.verticalCenter
       sourceSize: 22
       fullColor: true
       primaryColor: Qaterial.Style.accentColor
       text: qsTr("WEBSITE")
       font.family: "FuturaPTBook"
       font.pixelSize: 18
       source: "qrc:/assets/icons_material/baseline-insert_link-24px.svg"
       onClicked: {

       if(String(ericsettings.read("web"))!=="null") {

        Qt.openUrlExternally(String(ericsettings.read("web")))


        }

        else {

      Qaterial.SnackbarManager.show("coming soon")


            }


         }
   }

ButtonWireframeIconCentered {

id: githubBtn
width: 180
anchors.verticalCenter: parent.verticalCenter
sourceSize: 22
font.family: "FuturaPTBook"
font.pixelSize: 18
fullColor: true
primaryColor:  Qaterial.Style.accentColor

text: qsTr("SUPPORT")
source: "qrc:/assets/icons_material/baseline-support-24px.svg"
onClicked: {

 if(String(ericsettings.read("help"))!=="null"){

 Qt.openUrlExternally(String(ericsettings.read("help")))


    }

      else {


    Qaterial.SnackbarManager.show("coming soon")

         }




                }
            }
      }

////////

 Item { height: 1; width: 1; visible: isDesktop; } // spacer

  Item {
  id: desc
  height: Math.max(UtilsNumber.alignTo(description.contentHeight, 8), 48)
  anchors.left: parent.left
  anchors.leftMargin: 0
  anchors.right: parent.right
  anchors.rightMargin: 0

  ImageSvg {

   id: descImg
    width: 32
    height: 32
    anchors.left: parent.left
    anchors.leftMargin: 0
    anchors.verticalCenter: desc.verticalCenter
    source: "qrc:/assets/icons_material/outline-info-24px.svg"
    color: EricTheme.colorText

     }

  Text {

     id: description
     font.family: "FuturaPTBook"
     anchors.left: parent.left
      anchors.leftMargin: 48
      anchors.right: parent.right
      anchors.rightMargin: 0
      anchors.verticalCenter: desc.verticalCenter
      text: qsTr("An application for uploading menu items and their respective categories for a particular business. ")
      wrapMode: Text.WordWrap
      color: EricTheme.colorText
      font.pixelSize: 20
             }

         }










    Item {


     id: tuto
     height: Math.max(UtilsNumber.alignTo(description.contentHeight, 8), 48)
     anchors.left: parent.left
     anchors.leftMargin: 0
     anchors.right: parent.right
     anchors.rightMargin: 0


      IconSvg {
       width: 27
       height: 27
       anchors.left: parent.left
       anchors.leftMargin: 2
       anchors.verticalCenter: parent.verticalCenter
       source: "qrc:/assets/icons_material/baseline-import_contacts-24px.svg"
       color: EricTheme.colorIcon

           }

      Text {
      id: tutoTxt
      anchors.left: parent.left
      anchors.leftMargin: 48
      anchors.verticalCenter: parent.verticalCenter
      text: qsTr("Open the guide again")
      textFormat: Text.PlainText
      font.pixelSize: EricTheme.fontSizeContent
      color: EricTheme.colorText
                   }

      MouseArea {
      anchors.fill: parent
       onClicked: screenTutorial.reopen()
                   }
               }


       }


     }


}
