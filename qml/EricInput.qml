import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial 1.0 as Qaterial
import ComponentLibrary


Popup
{
    id: popup

    property DialogParams  options: DialogParams{}
    property string value
    property int inputMethodHints
    signal pickvalue(var myvalue)
    signal wascancelled()
    contentWidth: 250
    contentHeight: coulumn1.height

    modal: true
    focus: true

    x: ((app.width - width) / 2).toFixed(0)
    y: ((app.height - height) / 2).toFixed(0)

    closePolicy: Popup.NoAutoClose

    Column
    {
        id: coulumn1
        width: parent.width - app.margins.small * 2
        x: app.margins.small

        spacing: app.spacing.large

        Label
        {
            width: parent.width
            text: options.title
            font.family: "FuturaPTBook"
            font.pixelSize: 18
            font.bold: true
            color: EricTheme.colorText
        }

        Qaterial.OutlineTextField {

            id: valueField
            font.family: "FuturaPTBook"
            font.pixelSize: 18
            width: parent.width
            title: options.message
            inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText

        }

        Row
        {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: app.spacing.normal

            Qaterial.Button
            {
                text: options.button1
                font.family: "FuturaPTBook"
                font.pixelSize: 18
                backgroundColor: Qaterial.Style.accentColor
                onClicked: {
                    value = valueField.text
                    if(options.yesCallback) options.yesCallback(value)
                    valueField.text = ""
                    popup.pickvalue(value)

                }
            }

            Qaterial.Button
            {
                text: options.button2
                font.family: "FuturaPTBook"
                font.pixelSize: 18
                backgroundColor: Qaterial.Style.accentColor
                visible: options.button2 !== ""
                onClicked: {
                    value = ""
                    popup.wascancelled()
                    if(options.noCallback) options.noCallback()
                }
            }
        }
    }
}
