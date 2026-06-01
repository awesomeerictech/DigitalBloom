import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    focusPolicy: Qt.NoFocus

    property url source
    property int sourceSize: UtilsNumber.alignTo(height * 0.666, 2)

    ////////////////

    background: Rectangle {
        implicitWidth: EricTheme.componentHeight
        implicitHeight: EricTheme.componentHeight

        radius: EricTheme.componentHeight
        opacity: control.enabled ? 1 : 0.66
        color: control.down ? EricTheme.colorComponentDown : EricTheme.colorComponent
        border.width: 2
        border.color: EricTheme.colorComponentBorder
    }

    ////////////////

    contentItem: Item {
        IconSvg {
            anchors.centerIn: parent
            width: control.sourceSize
            height: control.sourceSize

            opacity: control.enabled ? 1 : 0.66
            source: control.source
            color: EricTheme.colorComponentContent
        }
    }

    ////////////////
}
