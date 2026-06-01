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
    signal pickvalue(var pin , var pass, var confirm)
    signal wascancelled()
    width: parent.width * 0.9
    contentHeight: root.implicitHeight
    property alias codeField: codeField
    property alias fieldregpass: fieldregpass
    property alias fieldregpassconfirm: fieldregpassconfirm
    property string valuepin
    property string valuepass
    property string valueconfirmpass






    modal: true
    focus: true


    closePolicy: Popup.NoAutoClose


    Flickable { // start flickeric


       id: flickeric
       anchors.fill: parent
       contentWidth: parent.width
       contentHeight: root.implicitHeight + 200
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


         Qaterial.OutlineTextField {

          id: fieldregpass
          Layout.fillWidth: true
          onTextChanged:text = text.replace(/\s+/g,'')
          font.family: "FuturaPTBook"
          font.pixelSize: 18
          error: false
          title: "Password"
          echoMode: Qaterial.TextField.Password
          helperText: "your password"
          helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
          placeholderText: ""
          leadingIconInline: true
          leadingIconSource:  Qaterial.Icons.lock
          leadingIconColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
          trailingInline: true
          trailingVisible: true

                  /*

                  RegEx	Description
                  ^	The password string will start this way
                  (?=.*[a-z])	The string must contain at least 1 lowercase alphabetical character
                  (?=.*[A-Z])	The string must contain at least 1 uppercase alphabetical character
                  (?=.*[0-9])	The string must contain at least 1 numeric character
                  (?=.*[!@#$%^&*])	The string must contain at least one special character, but we are escaping reserved RegEx characters to avoid conflict
                  (?=.{8,})	The string must be eight characters or longer

                   */

            validator:  RegularExpressionValidator { regularExpression: /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!@#\$%\^&\*])(?=.{8,})/}
            inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText
            trailingContent: Qaterial.AppBarButton
              {

              icon.source: (parent.parent.echoMode === Qaterial.TextField.Normal) ? Qaterial.Icons.eye : Qaterial.Icons.eyeOff
               icon.color : fieldregpass.activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                    rightInset: 0
                    onClicked:
                    {
                      if(parent.parent.echoMode === Qaterial.TextField.Normal)
                      {
                        parent.parent.echoMode = Qaterial.TextField.Password
                      }
                      else
                      {
                        parent.parent.echoMode = Qaterial.TextField.Normal
                      }
                    }
                  }
                  width: parent.width - 40
                }

                Qaterial.OutlineTextField
                {
                  id: fieldregpassconfirm
                  Layout.fillWidth: true
                  onTextChanged:text = text.replace(/\s+/g,'')
                  font.family: "FuturaPTBook"
                  font.pixelSize: 18
                  error: (fieldregpass.text !== fieldregpassconfirm.text )
                  errorText: "the two passwords do not match"
                  title: "Re-enter Password"
                  echoMode: Qaterial.TextField.Password
                  helperText: "the two passwords are matching"
                  helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                  placeholderText: ""
                  leadingIconInline: true
                  leadingIconSource:  Qaterial.Icons.lock
                  leadingIconColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                  trailingInline: true
                  trailingVisible: true

                  /*

                  RegEx	Description
                  ^	The password string will start this way
                  (?=.*[a-z])	The string must contain at least 1 lowercase alphabetical character
                  (?=.*[A-Z])	The string must contain at least 1 uppercase alphabetical character
                  (?=.*[0-9])	The string must contain at least 1 numeric character
                  (?=.*[!@#$%^&*])	The string must contain at least one special character, but we are escaping reserved RegEx characters to avoid conflict
                  (?=.{8,})	The string must be eight characters or longer

                   */

                  validator:  RegularExpressionValidator { regularExpression: /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!@#\$%\^&\*])(?=.{8,})/ }
                  inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText
                  trailingContent: Qaterial.AppBarButton
                  {
                    icon.source: (parent.parent.echoMode === Qaterial.TextField.Normal) ? Qaterial.Icons.eye : Qaterial.Icons.eyeOff
                //    Qaterial.TextFieldAlertIcon { visible: (fieldregpass.text !== fieldregpassconfirm.text) }
                    icon.color : fieldregpassconfirm.activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                    rightInset: 0
                    onClicked:
                    {
                      if(parent.parent.echoMode === Qaterial.TextField.Normal)
                      {
                        parent.parent.echoMode = Qaterial.TextField.Password
                      }
                      else
                      {
                        parent.parent.echoMode = Qaterial.TextField.Normal
                      }
                    }
                  }
                  width: parent.width - 40
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
                            popup.pickvalue(valuepin,valuepass,valueconfirmpass)



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
                            valuepass =""
                            valueconfirmpass =""
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
