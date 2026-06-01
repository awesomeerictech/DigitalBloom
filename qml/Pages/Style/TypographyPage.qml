

import QtQuick
import QtQuick.Layouts
import Fluid as Fluid
import "../.."

Flickable {
    clip: true
    contentHeight: Math.max(layout.implicitHeight, height)

    Fluid.ScrollBar.vertical: Fluid.ScrollBar {}

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: Fluid.Units.mediumSpacing
        spacing: Fluid.Units.smallSpacing

        Fluid.DisplayLabel {
            level: 4
            text: "Display 4"
        }

        Fluid.DisplayLabel {
            level: 3
            text: "Display 3"
        }

        Fluid.DisplayLabel {
            level: 2
            text: "Display 2"
        }

        Fluid.DisplayLabel {
            level: 1
            text: "Display 1"
        }

        Fluid.HeadlineLabel {
            text: "Headline"
        }

        Fluid.TitleLabel {
            text: "Title"
        }

        Fluid.SubheadingLabel {
            text: "Subheading"
        }

        Fluid.BodyLabel {
            level: 2
            text: "Body 2"
        }

        Fluid.BodyLabel {
            level: 1
            text: "Body 1"
        }

        Fluid.CaptionLabel {
            text: "Caption"
        }

        Fluid.Label {
            text: "Label"
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
