import QtQuick 2.15
// ...

Window {
    // ...
    Column {
        // ... (Save button remains the same) ...

        Button {
            text: "Open Any Document (SAF)"
            width: 250
            height: 50
            onClicked: {
                AndroidFileSaver.showOpenDialog(); // Calls the C++ bridge
            }
        }
        
        TextArea {
            id: fileContentArea
            width: 300
            height: 150
            placeholderText: "File content will appear here after opening..."
        }
    }

    Connections {
        target: AndroidFileSaver
        // ... (Connections for save operations) ...

        // New Connections for read operations
        onFileReadSuccess: {
            statusMessage.text = "File opened successfully: " + uri
            statusMessage.color = "blue"
            fileContentArea.text = content // Display the read content
        }
        onFileReadError: {
            statusMessage.text = "Read Error: " + message
            statusMessage.color = "red"
        }
    }
}
