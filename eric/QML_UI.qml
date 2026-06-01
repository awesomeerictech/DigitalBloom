import QtQuick 2.15
import QtQuick.Controls 2.15
import Deliva.Server 1.0

ApplicationWindow {
    id: win
    width: 640
    height: 480
    visible: true
    title: "Server Control"

    Column { spacing: 12; padding: 12
        Row {
            spacing: 8
            Text { text: "Wt (C++ Wt) Server:" }
            WtServerController {
                id: wtServer
                // optional: set properties if you exposed them
                onStarted: console.log("Wt started at", url)
                onErrorOccurred: console.warn("Wt error:", err)
            }
            Button {
                text: wtServer.running ? "Stop" : "Start"
                onClicked: {
                    if (wtServer.running) wtServer.stopServer()
                    else wtServer.startServer(5050) // if your startServer signature accepts port; adapt if not
                }
            }
        }

        Row {
            spacing: 8
            Text { text: "QxHttp Server:" }
            QxHttpServerController {
                id: qxServer
                onStarted: console.log("Qx server started at", url)
                onErrorOccurred: console.warn("Qx error:", err)
            }
            Button {
                text: qxServer.running ? "Stop" : "Start"
                onClicked: {
                    if (qxServer.running) qxServer.stopServer()
                    else qxServer.startServer() // uses default port (880) or port property if you set it
                }
            }
        }

        Row {
            spacing: 8
            Text { text: "IP Addresses:" }
            Button {
                text: "Refresh"
                onClicked: ipList.model = qxServer.availableIpAddresses()
            }
        }
        ListView {
            id: ipList
            width: parent.width - 40
            height: 120
            model: qxServer.availableIpAddresses()
            delegate: Text { text: modelData }
        }
    }
}
