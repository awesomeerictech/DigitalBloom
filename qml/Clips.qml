import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia
import Fluid as FluidControls
import ComponentLibrary
import Qaterial 1.0 as Qaterial


Page {

  id: myclips
  property bool clickme: true
  property string pagename : "Media Clips"

  background: Rectangle {

      anchors.fill: parent
      color: EricTheme.colorBackground

            }

//   Loader {

//       id: _loader
//       height: parent.height
//       width: parent.width
//       asynchronous: true
//       sourceComponent: {


//              myawesomecomp



//                  }




//          }

// Component {

//     id: myawesomecomp

//     ListView {

//      anchors.fill: parent

//         model: Qt.fontFamilies()

//         delegate:  FluidControls.ListItem {



//             mainText: modelData
//             maintextcolor: EricTheme.colorText
//             rightItem:     Card {
//                 id: card
//                 background: Rectangle {

//                     anchors.fill: parent
//                     color: EricTheme.colorBackground

//                           }
//                 anchors.centerIn: parent
//                 width: 100
//                 height: 100




//                 MediaPlayer {

//                 id: mediaplayer
//                 source: (Qt.platform.os==="android") ? "/storage/emulated/0/Movies/masculine.mp4" : "file:///home/ericiots/Documents/tesla.mp4"
//                 videoOutput: videoOutput
//                 audioOutput: AudioOutput {
//                 volume: 1
//                 muted: false

//                            }

//            /*    onPlaybackStateChanged: {

//                 if (MediaPlayer.PlayingState) {

//                      mediaplayer.pause()
//                 }

//                 if (MediaPlayer.PausedState) {

//                      mediaplayer.pause()
//                 }



//                 } */

//                onMediaStatusChanged: {

//                if (status === MediaPlayer.BufferedMedia)
//                     {
//                   mediaplayer.pause()

//                      }

//             /*   if (status === MediaPlayer.LoadedMedia)
//                     {
//                   mediaplayer.pause()

//                      } */

//                          }





//                        }


//                 VideoOutput {

//                 id: videoOutput
//                 anchors.fill: parent

//                       }

//                  MouseArea {
//                           id: playArea
//                           anchors.fill: parent
//                           onPressed: {

//                               mediaplayer.play();
//                               myapp.clickme =false

//                           }

//                           onDoubleClicked: {
//                               mediaplayer.pause()
//                               myapp.clickme =true
//                           }
//                       }




//             }
//         }



//         ScrollIndicator.vertical: ScrollIndicator { }
//     }


// }

}

