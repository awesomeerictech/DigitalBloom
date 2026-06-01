import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ThemeEngine 1.0

T.ComboBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    leftPadding: 12
    rightPadding: 12

    font.pixelSize: EricTheme.fontSizeComponent

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: EricTheme.componentHeight

        radius: EricTheme.componentRadius
        color: control.down ? EricTheme.colorComponentDown : EricTheme.colorComponent
        border.width: 1
        border.color: EricTheme.colorComponentBorder
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Text {
        text: control.displayText
        textFormat: Text.PlainText

        font: control.font
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter

        color: EricTheme.colorComponentContent
    }

    ////////////////////////////////////////////////////////////////////////////

    indicator: Canvas {
        x: control.width - width - control.rightPadding
        y: control.topPadding + ((control.availableHeight - height) / 2)
        width: 12
        height: 8
        rotation: control.popup.visible ? 180 : 0

        Connections {
            target: ThemeEngine
            function onCurrentThemeChanged() { indicator.requestPaint() }
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.moveTo(0, 0)
            ctx.lineTo(width, 0)
            ctx.lineTo(width / 2, height)
            ctx.closePath()
            ctx.fillStyle = EricTheme.colorComponentContent
            ctx.fill()
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    delegate: T.ItemDelegate {
        width: control.width - 2
        height: control.height
        highlighted: (control.highlightedIndex === index)

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: EricTheme.componentHeight

            radius: EricTheme.componentRadius
            opacity: enabled ? 1 : 0.3
            color: highlighted ? "#F6F6F6" : "transparent"
        }

        contentItem: Text {
            leftPadding: control.leftPadding
            text: modelData
            color: highlighted ? "black" : EricTheme.colorSubText
            font.pixelSize: EricTheme.fontSizeComponent
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    popup: T.Popup {
        y: control.height - 1
        width: control.width
        implicitHeight: (contentItem.implicitHeight) ? contentItem.implicitHeight + 2 : 0
        padding: 1

        contentItem: ListView {
            implicitHeight: contentHeight
            clip: true
            currentIndex: control.highlightedIndex
            model: control.popup.visible ? control.delegateModel : null
        }

        background: Rectangle {
            radius: EricTheme.componentRadius
            color: "white"
            border.color: EricTheme.colorComponentBorder
            border.width: control.visualFocus ? 0 : 1
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
