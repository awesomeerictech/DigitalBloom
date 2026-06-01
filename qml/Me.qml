import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ComponentLibrary
import Qaterial as Qaterial
import "qrc:/"
import "qrc:/qml"



Qaterial.Card
{
  id: root
  focus: true
  font.family: "FuturaPTBook"
  font.pixelSize: 25
  property string pagename : "My Account"
  media: "qrc:/myassets/gfx/logos/DigitalBloom.svg"
  headerText: String(ericsettings.read("user"))==="undefined" ? "DigitalBloom" : ericsettings.read("user")




  subHeaderText: {

     if(String(ericsettings.read("subscription")) ==="active") {

         "subscribed user"

     }

     else if(String(ericsettings.read("subscription")) ==="inactive") {

         "unsubscribed user"

     }

     else {

         "subscription is trial"

     }



  }
  supportingText: String(ericsettings.read("phone"))==="undefined" ? "DigitalBloom" : ericsettings.read("phone")


  contentItem: ColumnLayout
  {
    width: parent.width
    spacing: 0


    Qaterial.CardMedia {

        source: root.media
        clipTop: true
        Layout.fillHeight: false
        Layout.fillWidth: false
        Layout.preferredHeight: 334
        Layout.preferredWidth: 334
        Layout.alignment: Qt.AlignHCenter


    }

    RowLayout {

    Layout.fillWidth: true
    Layout.leftMargin: Qaterial.Style.card.horizontalPadding
    spacing: 0
    visible: true

    Qaterial.CardTitle
    {


      headerText: root.headerText
      subHeaderText: root.subHeaderText

    }

    }

    RowLayout {

    Layout.fillWidth: true
    Layout.leftMargin: Qaterial.Style.card.horizontalPadding
    spacing: 2
    visible: true

   Qaterial.Label {


       font.family: "FuturaPTBook"
       font.pixelSize: 25
       text: root.supportingText

   }

    Qaterial.FlatButton {

      height: editphone.height
      width: editphone.width
      Qaterial.ColorIcon
           {
             id: editphone
             anchors.fill: parent
             color:
             {
               Qaterial.Style.accentColor
             }
             source: Qaterial.Icons.accountEdit

     } // ColorIcon

      onClicked: {





      }

    }











    }






  }


}
