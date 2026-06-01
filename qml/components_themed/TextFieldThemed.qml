import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates  as T

import ThemeEngine 1.0

T.TextField {
    id: control

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                   || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    padding: 12
    leftPadding: padding + 4

    text: ""
    color: colorText
    font.pixelSize: EricTheme.fontSizeComponent
    verticalAlignment: TextInput.AlignVCenter

    placeholderText: ""
    placeholderTextColor: colorPlaceholderText

    selectByMouse: false
    selectionColor: colorSelection
    selectedTextColor: colorSelectedText

    onEditingFinished: focus = false

    // colors
    property string colorText: EricTheme.colorComponentText
    property string colorPlaceholderText: EricTheme.colorSubText
    property string colorBorder: EricTheme.colorComponentBorder
    property string colorBackground: EricTheme.colorComponentBackground
    property string colorSelectedText: EricTheme.colorHighContrast
    property string colorSelection: EricTheme.colorPrimary

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

    background: Rectangle {
        implicitWidth: 256
        implicitHeight: EricTheme.componentHeight

        radius: EricTheme.componentRadius
        color: control.colorBackground

        border.width: 2
        border.color: control.activeFocus ? control.colorSelection : control.colorBorder
    }
}
