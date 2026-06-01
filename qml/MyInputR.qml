import QtQuick
import QtQuick.Controls
import ComponentLibrary
import Qaterial as Qaterial
import QtQuick.Layouts
import Quickeric

Popup
{
    id: popup


    property DialogParams  options: DialogParams {}
    signal pickvalue(var pin)
    signal wascancelled()
    width: parent.width * 0.8
    contentHeight: root.implicitHeight
    property alias codeField: codeField
    property string valuepin
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose


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



         ColumnLayout {

          id: mycolumn
          width: parent.width - 15
          spacing: app.spacing.large




          Label
          {
              width: parent.width
              text: options.title
              font.family: "FuturaPTBook"
              font.pixelSize: 18
              font.bold: true
              color: EricTheme.colorText
          }

         Qaterial.OutlineTextField {

         id: codeField
         Layout.fillWidth: true
         onTextChanged:text = text.replace(/\s+/g,'')
         font.family: "FuturaPTBook"
         font.pixelSize: 18
         width: parent.width
         title: "input pin sent here"
         helperText: "verification code"
         helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
         inputMethodHints: Qt.ImhDialableCharactersOnly | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText


           }



                Row
                {
                    Layout.fillWidth: parent
                    spacing: app.spacing.normal

                    Qaterial.Button
                    {
                        text: options.button1
                        font.family: "FuturaPTBook"
                        font.pixelSize: 18
                        backgroundColor: Qaterial.Style.accentColor
                        onClicked: {

                            valuepin = codeField.text
                            valuepass = fieldregpass.text
                            valueconfirmpass = fieldregpassconfirm.text
                            popup.pickvalue(valuepin)


                        }
                    }

                    Qaterial.Button
                    {
                        text: options.button2
                        font.family: "FuturaPTBook"
                        font.pixelSize: 18
                        backgroundColor: Qaterial.Style.accentColor
                        visible: options.button2 !== ""
                        onClicked: {
                            valuepin =""
                            popup.wascancelled()

                        }
                    }
                }










                }



             }



       ScrollBar.vertical : ScrollBar {

                              policy: ScrollBar.AsNeeded
                              contentItem: Rectangle {
                              implicitWidth: 5
                              implicitHeight: 5
                              color:  (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker("orange",1.2) :  Qt.darker("teal", 1.2)

                                  }

                              active: true;

                              onActiveChanged: {

                                if (!active)
                                 active = true;
                               }
                         }



         } // end flickeric





}
