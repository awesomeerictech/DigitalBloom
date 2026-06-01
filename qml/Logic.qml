import Qaterial as Qaterial
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    property bool isLoading: false
    property string pagename : "Logic"

    Qaterial.Label {

        font.family: "FuturaPTBook"
        font.pixelSize: 25
        anchors.centerIn: parent
        color: "cyan"
        text: "Me"

    }

}
