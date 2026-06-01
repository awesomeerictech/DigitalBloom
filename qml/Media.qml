import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Window
import QtMultimedia

Page {

    id: mediaapp
    property string pagename : "Dimetron TV"
    anchors.fill: parent
    anchors.topMargin: 20
    focus: true
    signal playmedia
    signal stopmedia
    signal pausemedia
    signal mutemedia
    signal requestFullScreen
    signal requestNormalSize
    signal doclose
    property int det :0
    objectName: "mediaplayer"

    Component.onCompleted: {



     
    }





   




    function doclosewin() {

   Qt.quit()
       
    }




    Keys.onPressed: {
          if (event.key === Qt.Key_Escape || event.key === Qt.Key_Back) {

              Qt.quit()
                   
                    }

       }



    Keys.onEscapePressed: {

      

       }

    Keys.onSpacePressed: {

    }

    Keys.onLeftPressed: {

    }

     Keys.onRightPressed: {


     }




    MouseArea {
               anchors.fill: parent
               onClicked: {


             



               }
               onDoubleClicked: {



               }
       }


   




}

