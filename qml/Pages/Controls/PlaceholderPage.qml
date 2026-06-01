

import QtQuick
import QtQuick.Layouts
import Fluid as Fluid
import "../.." as Components

Components.StyledPage {
    Fluid.Placeholder {
        anchors.centerIn: parent
        icon.source: Fluid.Utils.iconUrl("social/notifications_none")
        text: qsTr("No notifications")
        subText: qsTr("At the moment there are no notifications available")
    }
}
