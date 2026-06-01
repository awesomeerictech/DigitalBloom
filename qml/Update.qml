import Qaterial as Qaterial
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {


    property string pagename : "Update Item"

    Qaterial.Label {

        font.family: "FuturaPTBook"
        font.pixelSize: 25
        anchors.centerIn: parent
        color: "cyan"
        text: "Update"

    }

}
