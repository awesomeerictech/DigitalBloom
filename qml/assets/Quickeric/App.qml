import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia
import Qt.labs.settings
import QtQml
import Quickeric 1.0
import Org.Eric.Technologies 1.0
import Qaterial as Qaterial
import Fluid as FluidControls


FluidControls.ApplicationWindow {


   property bool production: true



    visible: true
    property QtObject api
    property QtObject logic
    property Item pages
    property Item headers
    property Item footers
    property Margins margins: Margins{}
    property Spacing spacing: Spacing{}
    property Sizes sizes: Sizes{}
    property Urls urls: Urls{}




    Component.onCompleted: console.log("app ready")
    function ready(callback){


    }



    Connections {
        target: Gu
        onBack: back()
    }

    property RestConsumer2 restConsumer: RestConsumer2 {
        host: app.urls.root

        Component.onCompleted: {

            console.log("restConsumer ready")
            addHeader("Accept", "application/json")
            addHeader("Content-Type", "application/json")
        }
    }

    property RestConsumer3 restConsumer3: RestConsumer3 {
        host: app.urls.uploadUrl

        Component.onCompleted: {

            console.log("upload restConsumer ready")
            addHeader("Accept", "application/json")
            addHeader("Content-Type", "application/json")

        }
    }

}

