import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import Qaterial as Qaterial


TextField {

    id: control
    property color borderColorFocus: EricSettings.white
    signal resetSearchFieldClicked
  //  focus: true
    implicitHeight: 48
    topPadding: 13
    bottomPadding: 13
    leftPadding: 64
    rightPadding: 19
    font.family: "FuturaPTBook"
    font.pixelSize: 25
    focus: true
    inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText



    Qaterial.ColorIcon
         {

         id: iconSearch
         height: parent.height
         anchors.left: parent.left
         anchors.leftMargin: 19


         color:
         {
         Qaterial.Style.accentColor
         }
         source: Qaterial.Icons.magnify

         } // ColorIcon

    PlaceholderText {
        id: placeholder
        anchors.left: iconSearch.right
        padding: 0
        font.family: "FuturaPTBook"
        color: Qt.lighter("green",1.2)
        verticalAlignment: control.verticalAlignment
    }

    Qaterial.Button {

        id: _resetTextButton
        anchors.right: parent.right
        height: parent.height
        width: parent.height
        visible: false
        z: 100

        background: null

        Image {
            anchors.fill: parent
            source: "qrc:/assets/icons/clear_filter_icon.svg"
            sourceSize.width: parent.width
            sourceSize.height: parent.height
            fillMode: Image.PreserveAspectFit
        }

        onClicked: {
            control.resetSearchFieldClicked()
        }

        onPressed: {
            control.resetSearchFieldClicked()
        }

    }

     Qaterial.Button {

        id: _filtericon
        anchors.right: parent.right
        height: parent.height
        width: parent.height
        property bool resetfilter
        visible: (_resetTextButton.visible===false)
        z: 100

        background: null

        Component.onCompleted: {

            _filtericon.resetfilter=false
        }

        Image {
            anchors.fill: parent
            source: ( _filtericon.resetfilter===true) ? Qaterial.Icons.filterVariantMinus : Qaterial.Icons.filterVariantPlus
            sourceSize.width: parent.width
            sourceSize.height: parent.height
            fillMode: Image.PreserveAspectFit
        }

        onClicked: {

            if(_filtericon.resetfilter){

                _filtericon.resetfilter = false

            }

            else {


                 _filtericon.resetfilter = true

            }

            control.resetSearchFieldClicked()
        }


    }

    background: Rectangle {

        color: "white"
        border.color: control.activeFocus ? control.borderColorFocus : "white"
        border.width: 2
        radius: 5

    }

    onPreeditTextChanged: {

        if (control.displayText.length > 0) {
            _resetTextButton.visible = true
        } else {
            _resetTextButton.visible = false
        }

    }

    onTextEdited: {

        if (control.text.length > 0) {
            _resetTextButton.visible = true
        } else {
            _resetTextButton.visible = false
        }

    }

    onResetSearchFieldClicked: {

        _resetTextButton.visible = false

    }

}
