import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 600
    height: 400
    visible: true
    title: "Android File Saver Qt 6"

    Column {
        anchors.centerIn: parent
        spacing: 15

        Button {
            text: "Save File to Android 15"
            width: 250
            height: 50

            onClicked: {
                // Call the C++ invokable method
                AndroidFileSaver.showSaveDialog("MyTestFile.txt", "This data was written using Qt 6 JNI SAF.");
            }
        }

        Text {
            id: statusMessage
            text: "Ready."
        }
    }

    // Connect to the signals emitted by the C++ object
    Connections {
        target: AndroidFileSaver
        onFileSaveSuccess: {
            statusMessage.text = "Success! URI: " + uri
            statusMessage.color = "green"
        }
        onFileSaveError: {
            statusMessage.text = "Error: " + message
            statusMessage.color = "red"
        }
    }
}
