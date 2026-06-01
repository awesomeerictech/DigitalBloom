import QtQuick
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

Item
{
    id: pic

    property string url // picture url prefix
    property string pictureID

    property int size: app.sizes.xLarge * 5
    property alias picture: profilepic.source
    property alias myimage : profilepic
    property bool edit

    signal clicked
    signal select

    onPictureIDChanged: {
        if(pictureID !== "") profilepic.source = url + pictureID
    }

    width: size
    height: size

    Rectangle
    {
        anchors.fill: parent

        border.color: app.colors.white
        border.width: 2

        Image {
            id: profilepic
            fillMode: Image.PreserveAspectFit
            anchors.fill: parent
            cache: false
        }

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Item {
                width: profilepic.width
                height: profilepic.height
                Rectangle {
                    anchors.centerIn: parent
                    width: profilepic.width
                    height: profilepic.height
                    radius: profilepic.width /2
                }
            }
        }

        MouseArea
        {
            anchors.fill: parent
            onClicked: {
                if(edit) pic.select()
                else pic.clicked()
            }
        }
    }
}
