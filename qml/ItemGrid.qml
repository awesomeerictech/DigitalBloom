import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import QtQml
import QtQml.Models
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts
import Fluid as FluidControls
import ComponentLibrary
import "qrc:/"
import "qrc:/qml"
import QtQuick.Controls.Material


Page {

    id: mygrid
    width: parent.width
    anchors.topMargin: 20
    property string pagename : "Menu Items"
    readonly property int numElementsInRow: 3
    readonly property int currentCellWidth: Math.floor( mygrid.availableWidth / mygrid.numElementsInRow)



    background: Rectangle {

        anchors.fill: parent
        color: EricTheme.colorBackground

              }


    FluidControls.BottomSheet {

              id: myBottomSheet
              anchors.centerIn: parent

              background: Rectangle {

                  anchors.fill: parent
                  color: EricTheme.colorBackground

                        }


              Column {


                width: parent.width
                height: 300

               Flickable { // start flickeric


                  id: flickeric
                  anchors.fill: parent
                  contentWidth: parent.width
                  contentHeight: root.implicitHeight
                  anchors.leftMargin: screenLeftPadding
                  anchors.rightMargin: screenRightPadding
                  clip: true


                  Pane {

                   id: root
                   anchors.leftMargin: unsafeArea.unsafeLeftMargin
                   anchors.rightMargin: unsafeArea.unsafeRightMargin
                   anchors.fill: parent

                   background: Rectangle {

                         anchors.fill: parent
                         color: EricTheme.colorBackground


                                     }


                    ColumnLayout {

                     id: mycolumn
                     width: parent.width
                     spacing: 20

                      RoundedImage {

                       id: roundimg
                       source: "qrc:/assets/images/mysthea_button_back.png"
                       fillMode: Image.PreserveAspectCrop
                       width: root.width
                       borderWidth: 1
                       Layout.fillWidth: true
                       Layout.leftMargin: 0
                       Layout.rightMargin: 10
                       Layout.preferredHeight: 300
                       Layout.preferredWidth: 300
                      // Layout.rightMargin: Layout.leftMargin
                       Layout.topMargin: 20

                         }

                      RowLayout {


                        Label {

                          leftPadding: 10
                          rightPadding: 10
                          wrapMode: Text.WordWrap
                          text: qsTr("text1")
                          color: {



                              Qaterial.Style.accentColor



                                }



                             }




                           }

                      RowLayout {


                        Label {

                          leftPadding: 10
                          rightPadding: 10
                          wrapMode: Text.WordWrap
                          text: qsTr("text2")
                          color: {



                              Qaterial.Style.accentColor



                                }



                             }




                           }


                      RowLayout {


                        Label {

                          leftPadding: 10
                          rightPadding: 10
                          wrapMode: Text.WordWrap
                          text: qsTr("text3")
                          color: {



                              Qaterial.Style.accentColor



                                }



                             }




                           }





                           }



                        }



                  // ScrollBar.vertical : ScrollBar {

                  //                        policy: ScrollBar.AsNeeded
                  //                        contentItem: Rectangle {
                  //                        implicitWidth: 5
                  //                        implicitHeight: 5
                  //                        color:  (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker("orange",1.2) :  Qt.darker("teal", 1.2)

                  //                            }

                  //                        active: true;

                  //                        onActiveChanged: {

                  //                          if (!active)
                  //                           active = true;
                  //                         }
                  //                   }


                  ScrollBar.vertical: ScrollBar {
                         id: vscroll
                         policy: ScrollBar.AsNeeded

                         // THE visible thickness of the scrollbar — change to 2, 4, 6, etc.
                         width: 6

                         // track / background
                         background: Rectangle {
                             anchors.fill: parent
                             radius: width / 2
                             opacity: 0.12
                             color: EricTheme.colorBackground
                         }

                         // thumb / handle (QQC2 uses contentItem as the indicator)
                         contentItem: Rectangle {
                             id: thumb
                             width: parent.width             // important — match the scrollbar width
                             radius: width / 2
                             implicitHeight: 48              // minimum visible thumb size
                             anchors.horizontalCenter: parent.horizontalCenter

                             color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT)
                                    ? Qt.darker("orange", 0.9) : Qt.darker("teal", 0.9)

                             Behavior on y { NumberAnimation { duration: 120 } }
                         }

                         // keep visible if you prefer (optional)
                         active: true
                         onActiveChanged: { if (!active) active = true }
                     }



                    } // end flickeric

                  }




    }







    Column  { // start  Column data

          spacing: 6
          anchors.fill: parent

           FilterHeader  {
               id: _filtersHeader
               visible: (app.navi()!==1)
               width: parent.width
               accentColor: EricSettings.black
               backgroundColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? "#5483EF" : EricTheme.colorHeader

               onComboboxValueChanged: {

                 // In loader we don't have only listView so we check if the item has this property

                if (_loader.item.hasOwnProperty('positionViewAtBeginning')) {

                  _loader.item.positionViewAtBeginning()

                   }


               }


           }


     Loader {

     id: _loader
     visible: {

    if(app.navi()===1)
       {

      false

      }

     else if(app.navi()===2) {

     true
        }

     else {

       true
        }

        }
     active: {

    if(app.navi()===1)
         {

       false
       }

      else if(app.navi()===2) {

           true
          }

     else {

      true
         }

         }
   height: parent.height - _filtersHeader.height
   width: parent.width
   asynchronous: false
   sourceComponent: {

    if(app.navi()===1)
        {

      null
        }

    else if(app.navi()===2) {

     myawesomegrid
        }

     else {

     myawesomegrid
       }


          }




     }






Component {

  id: myawesomegrid

  GridView {

        id: grid
        interactive: true

        FluidControls.Placeholder {

         background: Rectangle {

         anchors.fill: parent
         color: Qaterial.Style.accentColor


              }


         visible: (grid.model.count === 0)
         anchors.fill: parent
         icon.source: FluidControls.Utils.iconUrl("action/info_outline")
         text: qsTr("No Items")
         subText: qsTr("Loading...")


              }



        clip: true
        width: parent.width
        cellHeight: cellWidth * 1.9
        cellWidth: mygrid.currentCellWidth
        implicitHeight: Math.ceil( model.count / mygrid.numElementsInRow) * cellHeight
        model: _filtersHeader.myproxymodel

        delegate: Pane {
                   height: grid.cellHeight
                   width: grid.cellWidth
                   topPadding: 16
                   bottomPadding: 16
                   leftPadding: 8
                   rightPadding: 8
                   background: null

                   ColumnLayout {
                       id: _col
                       anchors.fill: parent
                       spacing: 0

                       RoundedImage {
                           fillMode: Image.PreserveAspectCrop
                           //  fillMode: Image.PreserveAspectFit
                           asynchronous: false
                           source: "qrc:/assets/images/buzichoma.jpg"
                           borderWidth: 0
                           borderRadius: 5
                           Layout.preferredHeight: parent.height - _ericLabel.height
                           Layout.maximumWidth: parent.width
                           Layout.alignment: Qt.AlignHCenter
                           Layout.fillHeight: true
                           Layout.fillWidth: true
                       }

                       Label {
                           id: _ericLabel
                           text: model.item
                           font.family: "FuturaPTBold"
                           font.letterSpacing: 0
                           font.pixelSize: 20
                           wrapMode: Text.Wrap
                           Layout.fillWidth: true
                           color: app.myforegroundColor
                           Layout.topMargin: 6
                           Layout.alignment: Qt.AlignHCenter
                       }
                   }

                   MouseArea {

                       anchors.fill: parent

                       onClicked: {

                         myBottomSheet.open()

                       }

                   }
               }


      //  onFlickEnded: bottomsheet.open()

        // ScrollBar.vertical : ScrollBar {

        //                        policy: ScrollBar.AsNeeded
        //                        contentItem: Rectangle {
        //                        implicitWidth: 2
        //                        implicitHeight: 2
        //                        color:  (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker("orange",1.2) :  Qt.darker("teal", 1.2)

        //                            }

        //                        active: true;

        //                        onActiveChanged: {

        //                          if (!active)
        //                           active = true;
        //                         }
        //                   }

        ScrollBar.vertical: ScrollBar {
               id: vscroll
               policy: ScrollBar.AsNeeded

               // THE visible thickness of the scrollbar — change to 2, 4, 6, etc.
               width: 6

               // track / background
               background: Rectangle {
                   anchors.fill: parent
                   radius: width / 2
                   opacity: 0.12
                   color: EricTheme.colorBackground
               }

               // thumb / handle (QQC2 uses contentItem as the indicator)
               contentItem: Rectangle {
                   id: thumb
                   width: parent.width             // important — match the scrollbar width
                   radius: width / 2
                   implicitHeight: 48              // minimum visible thumb size
                   anchors.horizontalCenter: parent.horizontalCenter

                   color: (EricTheme.currentTheme === EricTheme.THEME_NIGHT)
                          ? Qt.darker("orange", 0.9) : Qt.darker("teal", 0.9)

                   Behavior on y { NumberAnimation { duration: 120 } }
               }

               // keep visible if you prefer (optional)
               active: true
               onActiveChanged: { if (!active) active = true }
           }




    }

}


} // end  Column data











}
