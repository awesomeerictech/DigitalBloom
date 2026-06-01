import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Quickeric 1.0
import Org.Eric.Technologies 1.0

Popup
{
    id: popup

    function upload(url, files,data,query){
        progressBar.value = 0.0
        busyIndicator.running = true
        open()
        uploader.upload(url, files,data,query)
    }

    RestUploader {
        id: uploader

        restConsumer: app.restConsumer3

        onProgress: {
            progressBar.value = sent

            if(total === -1)
                progressBar.indeterminate = true

            progressBar.to = total
        }

    /*    onProgress: {
            progressBar.value = sent

            if(total === -1) {
                progressBar.indeterminate = true
            }
            else if(total === sent){

                busyIndicator.running = false
                popup.finished(total)
                close()

            }
            else {

            progressBar.to = total
            }
        }
        */


        onError: {
            console.log("RestUploader error", err)
            busyIndicator.running = false
            popup.error(err)
            close()
        }

        onNetworkError: {
            console.log("onNetworkError", err)
            busyIndicator.running = false
            popup.networkError(err)
            close()
        }
        onServerError: {
            console.log("onServerError", err)
            busyIndicator.running = false
            popup.serverError(err)
            close()
        }

        onFinished: {
            busyIndicator.running = false
            popup.finished(total)
            close()
        }
    }

    signal finished (var total)
    signal networkError(var err)
    signal serverError(var err)
    signal error(var err)

    contentWidth: 250
    contentHeight: colum.height

    modal: true
    focus: true

    x: (app.width/2) - width / 2
    y: (app.height/2) - height / 2

    closePolicy: Popup.NoAutoClose

    Column {

        id: colum
        width: parent.width

        spacing: app.spacing.large

        Label
        {
            width: parent.width
            text: qsTr("Uploading")
            font.bold: true
        }

        RowLayout {
            width: parent.width
            height: app.sizes.xxxLarge * 2
            spacing: app.spacing.normal

            BusyIndicator
            {
                id: busyIndicator
                Layout.preferredWidth: width
                Layout.preferredHeight: height

                anchors.verticalCenter: parent.verticalCenter
                running: visible

                Label {
                    anchors.centerIn: parent
                    text: (progressBar.visualPosition * 100).toFixed(0) + "%"
                }
            }

            Column {
                Layout.fillWidth: true
                Layout.preferredHeight: height

                anchors.verticalCenter: parent.verticalCenter
                spacing: app.spacing.normal

                Space  {
                    height: app.sizes.small
                }

                ProgressBar {
                    id: progressBar
                    width: parent.width
                    from: 1
                }

                Label
                {
                    anchors.right: parent.right
                    text: uploader.current + "/" + uploader.total
                }
            }
        }
    }
}
