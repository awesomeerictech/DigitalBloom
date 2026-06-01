import QtQuick 2.15

import ThemeEngine 1.0

Rectangle {
    id: control
    implicitWidth: 80
    implicitHeight: 28

    width: contentText.contentWidth + 24

    radius: EricTheme.componentRadius
    color: backgroundColor

    property string text: "TAG"
    property string textColor: EricTheme.colorText
    property int textSize: EricTheme.fontSizeComponent

    property string backgroundColor: EricTheme.colorForeground

    Text {
        id: contentText
        anchors.centerIn: parent

        text: control.text
        textFormat: Text.PlainText

        color: control.textColor
        font.bold: true
        font.pixelSize: control.textSize
        font.capitalization: Font.AllUppercase
    }
}
