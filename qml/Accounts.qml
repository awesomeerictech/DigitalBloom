import Qaterial as Qaterial
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/"
import "qrc:/qml"
import ComponentLibrary





  Item
  {
    id: dataitem
    anchors.fill: parent
    anchors.topMargin: 20
    property string pagename : "Account SetUp"



    signal loginsuccess()
    signal loginfailed()
    signal verifycode()
    signal verifycodereg()

    onVerifycode: {

     /*  myverify.options.title="Verify Number"
        myverify.options.button1="Verify"
        myverify.options.button2="Cancel"
        myverify.options.message="enter the pin"
        myverify.open()  */


        myinput.options.title="Change Password"
        myinput.options.button1="Reset"
        myinput.options.button2="Cancel"
        myinput.open()
        inputme.close()


     }


    onVerifycodereg: {




     }


    onLoginsuccess: {


    appContent.state = "itemgridpage"

    }

    onLoginfailed: {




    }






    Qaterial.Card
    {
      anchors.centerIn: parent
      width: parent.width
      height: parent.height
      ColumnLayout
      {
        anchors.fill: parent
        Qaterial.TabBar
        {
          id: tabBar
          Layout.fillWidth: true
          Qaterial.TabButton
          {
            id: myin
            font.family: "FuturaPTBook"
            font.pixelSize: 25
            text: "Log in"
            width: 120
          }
          Qaterial.TabButton
          {
            id: myup
            font.family: "FuturaPTBook"
            font.pixelSize: 25
            text: "Sign up"
            width: 120
          }

        }


        Item
        {
          Layout.fillWidth: true
          Layout.fillHeight: true
          RowLayout
          {
            anchors.fill: parent
            Qaterial.SwipeView
            {
              id: swipeView
              currentIndex: tabBar.currentIndex
              Layout.fillWidth: true
              Layout.fillHeight: true
              clip: true

              onCurrentIndexChanged: {

                  if(currentIndex===0){

                      myin.checked=true
                  }

                  else if(currentIndex===1){

                      myup.checked=true
                  }

              }

              Flickable // start login Flickable
              {
                  contentHeight: mylogincolumn.height + sub1.height + 200

                  Column
                           {
                            id: mylogincolumn
                             x: 10
                             y: 10
                             spacing: 20
                             width: parent.width * 0.98
                             Qaterial.OutlineTextField
                             {
                               id: fieldloginuser
                               onTextChanged:text = text.replace(/\s+/g,'')
                               font.family: "FuturaPTBook"
                               font.pixelSize: 18
                               error: false
                               title: "Username"
                               helperText:  "business name eg TwoGalaxy_one"
                               helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                               placeholderText: ""
                               leadingIconInline: false
                               leadingIconColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                               prefixText: ""
                               suffixText:  ""
                               trailingInline: true
                               trailingVisible: true
                               inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText
                               leadingIconSource: Qaterial.Icons.account
                               width: parent.width - 40
                             }
                             Qaterial.OutlineTextField
                             {
                               id: fieldloginpass
                               onTextChanged:text = text.replace(/\s+/g,'')
                               font.family: "FuturaPTBook"
                               font.pixelSize: 18
                               error: false
                               title: "Password"
                               echoMode: Qaterial.TextField.Password
                               helperText: "your password"
                               helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                               placeholderText: ""
                               leadingIconInline: false
                               leadingIconSource:  Qaterial.Icons.lock
                               leadingIconColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                               trailingInline: true
                               trailingVisible: true
                               inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText
                               trailingContent: Qaterial.AppBarButton
                               {
                                 icon.source: (parent.parent.echoMode === Qaterial.TextField.Normal) ? Qaterial.Icons.eye : Qaterial.Icons.eyeOff
                                 icon.color : fieldloginpass.activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
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


                      CButton {

                           id: sub1
                           width: fieldloginpass.width - 20
                           anchors.horizontalCenter: parent.horizontalCenter
                           font.family: "FuturaPTBook"
                           font.pixelSize: 25
                           height: 60
                           name: "Log in"
                           baseColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)
                           borderColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)

                           onClicked: {

                            dataitem.loginsuccess()



                                }


                           }


                      ValueInput {

                              id: inputme
                              inputMethodHints: Qt.ImhDialableCharactersOnly | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText


                              onClosed: {



                              }


                              onPickvalue : {


                                  console.log("value is: "+myvalue)


                                  if(String(myvalue)==="") {

                                    // app.showError("The field cannot be empty")
                                     // app.showToast("The field cannot be empty")
                                     // app.showInfo("The field cannot be empty")
                                    //  app.showYesNo("The field cannot be empty")
                                    //  app.showUpdate("The field cannot be empty")
                                      dataitem.verifycode()




                                  }

                                  else {

                                  /*    var num = jsontools.rng()
                                      ericsettings.write("code",Number(num))
                                      ericsms.singleMessage("classapp",String(myvalue),String(num),String("ANETQUICK"),"0")
                                      ericsettings.write("tempnum",String(myvalue))
                                      Qaterial.SnackbarManager.show("verify your phone number by entering the code that we have just sent to you")
                                      inputme.close()
                                      root.verifycode() */

                                        dataitem.verifycode()


                                  }





                              }


                              onWascancelled : {

                                console.log("Called!!! ")
                                Qaterial.SnackbarManager.show("Operation cancelled by user")
                                inputme.close()


                              }



                          }

                      EricInputS {


                          id: myinput

                          onClosed: {



                          }



                          onPickvalue : {





                              if(String(pin)==="" || String(pass)==="" || String(confirm)==="") {

                                // app.showError("The field cannot be empty")
                                 // app.showToast("The field cannot be empty")
                                 // app.showInfo("The field cannot be empty")
                                //  app.showYesNo("The field cannot be empty")
                                //  app.showUpdate("The field cannot be empty")
                                dataitem.verifycode()




                              }

                              else {



                                  console.log("pin is: "+pin)
                                  console.log("pass is: "+pass)
                                  console.log("confirm pass is: "+confirm)



                              }





                          }


                          onWascancelled : {

                            console.log("Called!!! ")
                            myinput.close()


                          }



                      }



                     Text {

                     id: name
                     anchors.horizontalCenter: parent.horizontalCenter
                     text: '<html><style type="text/css"></style><a href="http://google.com">Forgot password?</a></html>' //qsTr("Forgot password?")
                     linkColor: EricTheme.colorText
                     font.pointSize: 14
                     color: EricTheme.colorText
                     onLinkActivated: {



                         inputme.options.title="Verify User"
                         inputme.options.button1="Verify"
                         inputme.options.button2="Cancel"
                         inputme.options.message="type your username here"
                         inputme.open()



                                    }

                          }


                           }

                       ScrollBar.vertical : ScrollBar {

                            policy: ScrollBar.AsNeeded

                            active: true;

                            onActiveChanged: {

                              if (!active)
                               active = true;
                             }
                       }

                      } // end login Flickable


                     Flickable // start reg Flickable
                          {
                            contentHeight: myregcolumn.height + sub2.height + 200
                            Column
                                 {
                                   id: myregcolumn
                                   x: 10
                                   y: 10
                                   spacing: 20
                                   width: parent.width * 0.98
                                   Qaterial.OutlineTextField
                                   {
                                     id: fieldreguser
                                     font.family: "FuturaPTBook"
                                     font.pixelSize: 18
                                     error: false
                                     onTextChanged:text = text.replace(/\s+/g,'')
                                     title: "Username"
                                     helperText:  "business name eg TwoGalaxy_one"
                                     helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                                     placeholderText: ""
                                     leadingIconInline: false
                                     prefixText: ""
                                     suffixText:  ""
                                     trailingInline: true
                                     trailingVisible: true
                                     leadingIconSource: Qaterial.Icons.account
                                     inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText
                                     leadingIconColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                                     width: parent.width - 40
                                   }
                                   Qaterial.OutlineTextField
                                   {
                                     id: fieldregpass
                                     onTextChanged:text = text.replace(/\s+/g,'')
                                     font.family: "FuturaPTBook"
                                     font.pixelSize: 18
                                     error: false
                                     title: "Password"
                                     echoMode: Qaterial.TextField.Password
                                     helperText: "your password"
                                     helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                                     placeholderText: ""
                                     leadingIconInline: false
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
                                     leadingIconInline: false
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

                                   Qaterial.OutlineTextField
                                   {
                                     id: fieldregphone
                                     font.family: "FuturaPTBook"
                                     font.pixelSize: 18
                                     error: false
                                     onTextChanged:text = text.replace(/\s+/g,'')
                                     title: "Phone Number"
                                     helperText:  "phone number eg 0723..."
                                     helperTextColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                                     placeholderText: ""
                                     leadingIconInline: false
                                     prefixText: ""
                                     suffixText:  ""
                                     trailingInline: true
                                     trailingVisible: true
                                     inputMethodHints: Qt.ImhDialableCharactersOnly | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                                     leadingIconSource:  Qaterial.Icons.phone
                                     leadingIconColor: activeFocus ? (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5) : Qaterial.Style.disabledTextColor()
                                     width: parent.width - 40
                                   }


                                   EricInput {

                                           id: inputreg
                                           inputMethodHints: Qt.ImhDialableCharactersOnly | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText


                                           onClosed: {



                                           }


                                           onPickvalue : {


                                               console.log("value is: "+myvalue)


                                               if(String(myvalue)==="") {

                                                 // app.showError("The field cannot be empty")
                                                  // app.showToast("The field cannot be empty")
                                                  // app.showInfo("The field cannot be empty")
                                                 //  app.showYesNo("The field cannot be empty")
                                                 //  app.showUpdate("The field cannot be empty")
                                                   dataitem.verifycodereg()




                                               }

                                               else {

                                               /*    var num = jsontools.rng()
                                                   ericsettings.write("code",Number(num))
                                                   ericsms.singleMessage("classapp",String(myvalue),String(num),String("ANETQUICK"),"0")
                                                   ericsettings.write("tempnum",String(myvalue))
                                                   Qaterial.SnackbarManager.show("verify your phone number by entering the code that we have just sent to you")
                                                   inputme.close()
                                                   root.verifycode() */

                                                     dataitem.verifycodereg()


                                               }





                                           }


                                           onWascancelled : {

                                             console.log("Called!!! ")
                                             Qaterial.SnackbarManager.show("Operation cancelled by user")
                                             inputreg.close()


                                           }



                                       }

                                 CButton {

                                  id: sub2
                                  width: fieldregpass.width - 20
                                  anchors.horizontalCenter: parent.horizontalCenter
                                  font.family: "FuturaPTBook"
                                  font.pixelSize: 25
                                  height: 60
                                  name: "Sign up"
                                  baseColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)
                                  borderColor: (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.lighter("orange",1.2) :  Qt.lighter("teal", 1.5)

                                  onClicked: {

                                      inputreg.options.title="Verify Number"
                                      inputreg.options.button1="Verify"
                                      inputreg.options.button2="Cancel"
                                      inputreg.options.message="enter the code sent to you"
                                      inputreg.open()



                                       }


                                    }

                                 }

                            ScrollBar.vertical : ScrollBar {

                                 policy: ScrollBar.AsNeeded

                                 active: true;

                                 onActiveChanged: {

                                   if (!active)
                                    active = true;
                                  }
                            }

                             } // end reg Flickable

                         }

                      }

                   }

               }

           }

        }

