import QtQuick
import QtQuick.Controls

import ComponentLibrary

Rectangle {
    id: screenMainMenu
    anchors.fill: parent

    color: EricTheme.colorBackground

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        appContent.state = "ScreenMainMenu"
    }

    ////////////////////////////////////////////////////////////////////////////
}
