

import QtQuick
import QtQuick.Layouts
import Fluid as Fluid
import "../.."

Fluid.Page {
    Fluid.ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: parent.width

            Fluid.ListItem {
                text: "ListItem with only text"
            }

            Fluid.ListItem {
                text: "ListItem with sub text"
                subText: "Sub text"
            }

            Fluid.ListItem {
                text: "ListItem with sub text and 2/2 lines"
                subText: "Sub text line 1\nSub text line 2"
            }

            Fluid.ListItem {
                text: "ListItem with sub text and 3/3 lines"
                subText: "Sub text line 1\nSub text line 2\nSub text line 3"
                maximumLineCount: 3
            }

            Fluid.ListItem {
                text: "ListItem with sub text and 4/4 lines"
                subText: "Sub text line 1\nSub text line 2\nSub text line 3\nSub text line 4"
                maximumLineCount: 4
            }

            Fluid.ListItem {
                text: "ListItem with sub text and 5/3 lines"
                subText: "Sub text line 1\nSub text line 2\nSub text line 3\nSub text line 4\nSub text line 5"
                maximumLineCount: 3
            }

            Fluid.ListItem {
                text: "ListItem with value text"
                valueText: "Value"
            }

            Fluid.ListItem {
                icon.source: Fluid.Utils.iconUrl("action/event")
                text: "ListItem with icon"
            }

            Fluid.ListItem {
                text: "ListItem with custom rightItem"
                rightItem: Fluid.Switch {
                    anchors.centerIn: parent
                    checked: true
                }
            }

            Fluid.ListItem {
                text: "ListItem with custom secondaryItem"
                secondaryItem: Fluid.Slider {
                    width: parent.width
                    from: 0
                    to: 100
                    value: 50
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
