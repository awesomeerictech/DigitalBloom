import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.TextArea {
    id: control

    implicitWidth: Math.max(contentWidth + leftPadding + rightPadding,
                            implicitBackgroundWidth + leftInset + rightInset,
                            placeholder.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                             implicitBackgroundHeight + topInset + bottomInset,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    padding: 12

    color: colorText
    opacity: control.enabled ? 1 : 0.66

    font.pixelSize: EricTheme.componentFontSize
    verticalAlignment: Text.AlignTop

    text: ""
    placeholderText: ""
    placeholderTextColor: colorPlaceholderText

    selectByMouse: isDesktop
    selectionColor: colorSelection
    selectedTextColor: colorSelectedText

    EnterKey.type: Qt.EnterKeyDone

    onEditingFinished: focus = false
    Keys.onBackPressed: focus = false

    // colors
    property color colorText: EricTheme.colorComponentContent
    property color colorPlaceholderText: EricTheme.colorSubText
    property color colorBorder: EricTheme.colorComponentBorder
    property color colorBackground: EricTheme.colorComponentBackground
    property color colorSelection: EricTheme.colorPrimary
    property color colorSelectedText: "white"

    ////////////////

    background: Rectangle {
        implicitWidth: 256
        implicitHeight: EricTheme.componentHeight*2

        radius: EricTheme.componentRadius
        color: control.colorBackground

        border.width: 2
        border.color: control.activeFocus ? control.colorSelection : control.colorBorder
    }

    PlaceholderText {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
        renderType: control.renderType
    }

    ////////////////
}
