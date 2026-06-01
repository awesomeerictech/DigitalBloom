import QtQuick 2.15
import QtQuick.Controls.impl 2.15
import QtQuick.Templates 2.15 as T

import ThemeEngine 1.0

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    font.pixelSize: EricTheme.fontSizeComponent
    font.bold: true

    flat: true
    focusPolicy: Qt.NoFocus

    // colors
    property string colorHighlighted: EricTheme.colorPrimary
    property string colorHovered: EricTheme.colorHeader

    background: Rectangle {
        implicitWidth: 80
        implicitHeight: EricTheme.componentHeight

        radius: 2
        opacity: (control.hovered && !control.highlighted) ? 0.3 : 1
        color: {
            if (control.highlighted) return control.colorHighlighted
            if (control.hovered) return control.colorHovered
            return "transparent"
        }
    }

    contentItem: Text {
        text: control.text
        textFormat: Text.PlainText

        font: control.font
        elide: Text.ElideMiddle
        //wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        opacity: 1
        color: control.highlighted ? "white" : EricTheme.colorText
    }
}
