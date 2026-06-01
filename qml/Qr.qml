import QtQuick.Dialogs
import Img 21.1
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/js/UtilsNumber.js" as UtilsNumber
import Qaterial as Qaterial
import ComponentLibrary
import "qrc:/"
import "qrc:/qml"
import "qrc:/qml/components_js/UtilsPath.js" as Myutil
import Quickeric 1.0



Page {

    id: ericqr
    focus: true
    anchors.topMargin: 30
    property string pagename : "My Qr Code"
    property Action leftAction: null
    property bool isLoading: false
    property string username
    property string mainurl


    Component.onCompleted: {



   //  username = String(ericsettings.read("user"))
  //   mainurl = String(ericsettings.read("mainurl"))


    }











   /* property string advancedUrl: "image://QZXing/encode/" + mainurl+"/"+username +
                                     "?correctionLevel=" + "H" +
                                     "&format=" + "qrcode" +
                                     "&border=" + (true) */

    property string advancedUrl: "image://QZXing/encode/" +"www.awesomeerictech.com/mybiz" +
                                     "?correctionLevel=" + "H" +
                                     "&format=" + "qrcode" +
                                     "&border=" + (true)



    ColumnLayout {
        id: mainLayout
        anchors {
            fill: parent
          //  margins: 10
        }
         width: parent.width




         Row
         {

           spacing: 10
           Layout.fillWidth: true

           Label {
               text: "Download Qr"
               font.pixelSize: 18
               color: app.myforegroundColor
               wrapMode: Text.Wrap
           }

           Qaterial.FlatButton {

             height: download.height
             width: download.width
             Qaterial.ColorIcon
                  {
                    id: download
                    anchors.fill: parent
                    color:
                    {
                      Qaterial.Style.accentColor
                    }
                    source: Qaterial.Icons.download

            } // ColorIcon

             onClicked: {




                 // img.save(resultImage,"/root/qtprojects/qrscans/"+"example"+".png")

                 if(Qt.platform.os==="android") {



                            username = String(ericsettings.read("user"))
                            console.log("paths are "+utilsApp.getStandardPath_url("downloads"))
                            console.log("clean url is :"+Myutil.cleanUrl(utilsApp.getStandardPath_url("downloads")))
                           fshelpereric.createanydir("DigitalBloom",Myutil.cleanUrl(utilsApp.getStandardPath_url("downloads"))+"/")
                           img.save(resultImage,Myutil.cleanUrl(utilsApp.getStandardPath_url("downloads"))+"/DigitalBloom/"+username+".png")
                           // img.save(resultImage,Myutil.cleanUrl(utilsApp.getStandardPath_url("home"))+"/"+username+".png")
                            Qaterial.SnackbarManager.show("Qr image has been saved in gallery")
                           Myutil.cleanUrl(utilsApp.getStandardPath_url("downloads"))+"/DigitalBloom/"+username+".png"






                 }

                 else {

                     username = String(ericsettings.read("user"))
                     console.log("paths are "+utilsApp.getStandardPath_url("photo"))
                     console.log("clean url is :"+Myutil.cleanUrl(utilsApp.getStandardPath_url("photo")))
                    fshelpereric.createanydir("DigitalBloom",Myutil.cleanUrl(utilsApp.getStandardPath_url("photo"))+"/")
                    img.save(resultImage,Myutil.cleanUrl(utilsApp.getStandardPath_url("photo"))+"/DigitalBloom/"+username+".png")
                    // img.save(resultImage,Myutil.cleanUrl(utilsApp.getStandardPath_url("home"))+"/"+username+".png")
                     Qaterial.SnackbarManager.show("Qr image has been saved in gallery")


                 }







             }

           }
         }




        Rectangle {
            id: barcodeRectangle
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.width: 1
            border.color: "#bdbebf"
            clip: true
            color: Qaterial.Style.accentColor

            property int imageWidth: Math.min(height, width) * 0.7;

            Image {

                id:resultImage
                anchors.centerIn: parent
                sourceSize.width: barcodeRectangle.imageWidth
                sourceSize.height: barcodeRectangle.imageWidth
                source: mainLayout.getImageRequestString()
                cache: false;



            }

            EricImage

            {

                id: img

            }






        }



        function getImageRequestString() {

                return advancedUrl;

        }



    } // end ColumnLayout
}
