import Fluid as FluidControls
import QtQuick
import QtQuick.Controls
import ComponentLibrary
import Qaterial 1.0 as Qaterial
import "qrc:/"
import "qrc:/qml"
import QtQuick.Layouts
import Quickeric

EricInput {
       id: input
       property string mytitle
       property string mytext
      // property string mymask
       property string myplaceholder
       signal phonenumber(var phone)

       x: (parent.width - width) / 2
       y: (parent.height - height) / 2
       title: mytitle
       text: mytext
    //   textField.inputMask: mymask
       textField.placeholderText: myplaceholder
   }
