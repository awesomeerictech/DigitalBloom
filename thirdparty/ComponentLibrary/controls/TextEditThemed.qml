import QtQuick

import ComponentLibrary

TextEdit {
    color: colorText
    opacity: enabled ? 1 : 0.66
    font.pixelSize: EricTheme.componentFontSize

    selectByMouse: isDesktop
    selectionColor: EricTheme.colorPrimary
    selectedTextColor: "white"

    readOnly: false
    wrapMode: Text.NoWrap
    EnterKey.type: Qt.EnterKeyDone

    onEditingFinished: focus = false
    Keys.onBackPressed: focus = false

    // colors
    property color colorText: EricTheme.colorText
}
