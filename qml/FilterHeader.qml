import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import QSyncable 1.0
import SortFilterProxyModel 0.2
import QtQuick.Templates  as T






Qaterial.ToolBar {

    id: root


   property QSJsonListModel firstComboBoxModel: null
    property QSJsonListModel secondComboBoxModel: null
    property SortFilterProxyModel myproxymodel : proxyModel
    property color accentColor
    property bool enablevalftr
    property bool enablesearchftr
    signal comboboxValueChanged
    property string searchstr
    property string  valueoneftr
    signal valone(var one,var index)
    property string paths: jsontools.jsondatapath()
    property string assetpath : jsontools.jsonqrcpath()
    property string datapath : jsontools.datapath()




    Component.onCompleted: {


        //   updateProcess()
        harrythomas.start()



       }







    onValone: {



        if(index>0) {

            enablevalftr=true
            enablesearchftr=false
            valueoneftr=one




        }

        else {

            enablevalftr=false
            valueoneftr=""


        }

        console.log("val1: "+one)
        console.log("index is: "+index)


    }


    function stopheadertimers() {

        startupdateTimer.running=false
        startupdateTimer.stop()
        delayupdateTimer.running=false
        delayupdateTimer.stop()


    }


    function startheadertimers() {


        startupdateTimer.running=true
        startupdateTimer.start()



    }







    Timer {
               id: delayupdateTimer
              // interval: 20000
               interval: 2000
               repeat: false
               onTriggered: {


                   app.api.menuusergetall.data("reason="+String(ericsettings.read("token")),String(ericsettings.read("user"))).then(function(data) {




                                  myjson.tomap(data,root.paths,"mycustom.json")
                                  themodel.source= myjson.frommaptovariantlist(jsontools.jsondatapath(),"mycustom.json")




                                },function(serv){console.log("server error is: "+serv)},function(neterr){

                                                           if(neterr===6){

                                                               app.showError("update your app!")

                                                            }

                                                             else if(neterr===1){

                                                            app.showError("connect to the internet first!!")
                                                            }

                                                            else {

                                                                app.showError("network error")
                                                            }



                                                })







               } // end triggered

           } //end timer


        Timer {

               id: startupdateTimer
               interval: 1000
               repeat: false

               onTriggered: {


                   // Just for testing filters



                   app.api.menuusergetall.data("reason="+String(ericsettings.read("token")),String(ericsettings.read("user"))).then(function(data) {




                       myjson.tomap(data,root.paths,"custom.json")
                       themodel.source= myjson.frommaptovariantlist(jsontools.jsondatapath(),"custom.json")




                                },function(serv){console.log("server error is: "+serv)},function(neterr){

                                                           if(neterr===6){
                                                             //   app.showToast("please update your app!!!")

                                                            }

                                                             else if(neterr===1){

                                                           // app.showToast("connect to the internet first!!!")
                                                            }

                                                            else {
                                                              //  app.showToast("unknown network error!!!")
                                                            }



                                                })



                              delayupdateTimer.start()



               } // end triggered

           } //end timer




        function updateProcess() {


                startupdateTimer.start()
                startupdateTimer.running=true

            }


        ListModel {


          id: mydummy



                 }


        Timer {

            id: harrythomas
            interval: 2000
            repeat: false
            onTriggered: {

                var mycategory  = [];
                var myitem  = [];

                mycategory.push("vodka");
                mycategory.push("beer");
                mycategory.push("wines");
                mycategory.push("soft drinks");
                myitem.push("kibao");
                myitem.push("balozi");
                myitem.push("Henessy");
                myitem.push("coke");

                mycategory.push("vodka1");
                mycategory.push("beer1");
                mycategory.push("wines1");
                mycategory.push("soft drinks1");
                myitem.push("kibao1");
                myitem.push("balozi1");
                myitem.push("Henessy1");
                myitem.push("coke1");

                mycategory.push("vodka2");
                mycategory.push("beer2");
                mycategory.push("wines2");
                mycategory.push("soft drinks2");
                myitem.push("kibao2");
                myitem.push("balozi2");
                myitem.push("Henessy2");
                myitem.push("coke2");

                mycategory.push("vodka3");
                mycategory.push("beer3");
                mycategory.push("wines3");
                mycategory.push("soft drinks3");
                myitem.push("kibao3");
                myitem.push("balozi3");
                myitem.push("Henessy3");
                myitem.push("coke3");

                mycategory.push("vodka4");
                mycategory.push("beer4");
                mycategory.push("wines4");
                mycategory.push("soft drinks4");
                myitem.push("kibao4");
                myitem.push("balozi4");
                myitem.push("Henessy4");
                myitem.push("coke4");

                mycategory.push("vodka5");
                mycategory.push("beer5");
                mycategory.push("wines5");
                mycategory.push("soft drinks5");
                myitem.push("kibao5");
                myitem.push("balozi5");
                myitem.push("Henessy5");
                myitem.push("coke5");

                mycategory.push("vodka6");
                mycategory.push("beer6");
                mycategory.push("wines6");
                mycategory.push("soft drinks6");
                myitem.push("kibao6");
                myitem.push("balozi6");
                myitem.push("Henessy6");
                myitem.push("coke6");

                mycategory.push("vodka7");
                mycategory.push("beer7");
                mycategory.push("wines7");
                mycategory.push("soft drinks7");
                myitem.push("kibao7");
                myitem.push("balozi7");
                myitem.push("Henessy7");
                myitem.push("coke7");

                mycategory.forEach((num1, index) => {

                               var num2 = myitem[index];
                               mydummy.append({category: num1,item: num2});



                                           });


                var arr = [];
                var len = mydummy.count;

                for (var i = 0; i < len; i++) {


                console.log("category is: "+mydummy.get(i).category)
                console.log("item is: "+mydummy.get(i).item)



                 arr.push({

                     category: mydummy.get(i).category,
                     item: mydummy.get(i).item,




                                    });

                                }



                myjson.combinejson(arr,jsontools.jsondatapath(),"test.json")
                themodel.source= myjson.frommaptovariantlist(jsontools.jsondatapath(),"test.json")




            }
        }



        QSJsonListModel {

                id: themodel
                keyField: "item"



                  }


        SortFilterProxyModel { // start sort filter proxy model

                    id: proxyModel
                    sourceModel: themodel
                    sorters: [



                      StringSorter { roleName: "item" },
                      StringSorter { roleName: "category" }


                    ]

                    filters: [

                        AnyOf {


                             enabled: enablesearchftr

                            RegExpFilter {
                            roleName: "item"
                            pattern: "^" +  searchstr
                            caseSensitivity: Qt.CaseInsensitive

                        }
                            RegExpFilter {
                            roleName: "category"
                            pattern: "^" +  searchstr
                            caseSensitivity: Qt.CaseInsensitive

                        }


                        },

                        AllOf {

                               enabled:  enablevalftr
                                RegExpFilter {
                                roleName: "category"
                                pattern: "^" +  valueoneftr
                                caseSensitivity: Qt.CaseInsensitive
                            }





                        }
                    ]

                } // end sort filter proxy model






    implicitHeight: 136
    padding: 16


    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        SearchField {
                    id: _searchField
                    borderColorFocus: root.accentColor
                    placeholderText: qsTr("Search Here")
                    placeholderTextColor:  "black"
                    color: "black"
                    //(Theme.currentTheme === EricTheme.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"
                    font.family: "FuturaPTBook"
                    font.pixelSize: 18

                    Layout.fillWidth: true

                    onTextEdited: {


                        //  _searchField.displayText

                        enablevalftr=false
                        enablesearchftr=true
                        valueoneftr=""
                        searchstr=_searchField.text

                    }

                    onResetSearchFieldClicked: {

                      enablevalftr=false
                        enablesearchftr=true
                        _searchField.clear()
                        _searchField.update()
                        valueoneftr=""
                        searchstr=_searchField.displayText

                    }

                    onPreeditTextChanged: {

                       enablevalftr=false
                        enablesearchftr=true
                        valueoneftr=""
                        searchstr=_searchField.displayText

                    }
                } // end SearchField

        RowLayout {
            spacing: 16

            Layout.fillWidth: true

             TextIconComboBox {
                id: _comboone


               //model: root.firstComboBoxModel
                  model: themodel

                textRole: "category"




                Layout.fillWidth: true

                onActivated: {

             /*    console.log("model index is: "+ root.firstComboBoxModel.index(index, 0))
                 console.log("model value is: "+ varianthelpers.valuefrommap(root.firstComboBoxModel.get(index),"category"))
                  valueoneftr=varianthelpers.valuefrommap(root.firstComboBoxModel.get(index),"category")

                    root.valone(varianthelpers.valuefrommap(root.firstComboBoxModel.get(index),"category"),index) */

                    console.log("model index is: "+ themodel.index(index, 0))
                    console.log("model value is: "+ varianthelpers.valuefrommap(themodel.get(index),"category"))
                     valueoneftr=varianthelpers.valuefrommap(themodel.get(index),"category")

                       root.valone(varianthelpers.valuefrommap(themodel.get(index),"category"),index)




                    root.comboboxValueChanged()
                }
            }






        } // end RowLayout
    }  // end ColumnLayout
}

