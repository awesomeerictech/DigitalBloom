
import Qaterial as Qaterial
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Fluid as Fluid
import "../.." as Components

Components.StyledPageTwoColumns {
    leftColumn: ColumnLayout {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Enabled")

            Layout.alignment: Qt.AlignHCenter
        }

        Fluid.Button {
            text: qsTr("Button")
        }

        Fluid.Button {
            text: qsTr("Checked")
            checkable: false
            checked: true
        }

        Fluid.Button {
            text: qsTr("Flat")
            flat: true
        }

        Fluid.Button {
            text: qsTr("Highlighted")
            highlighted: true
        }

        Fluid.Button {
            text: qsTr("Flat Highlighted")
            flat: true
            highlighted: true
        }
    }

    rightColumn: ColumnLayout {
        anchors.centerIn: parent

        Fluid.TitleLabel {
            text: qsTr("Disabled")

            Layout.alignment: Qt.AlignHCenter
        }

        Fluid.Button {
            text: qsTr("Button")
            enabled: false
        }

        Fluid.Button {
            text: qsTr("Checked")
            checkable: false
            checked: true
            enabled: false
        }

        Fluid.Button {
            text: qsTr("Flat")
            flat: true
            enabled: false
        }

        Fluid.Button {
            text: qsTr("Highlighted")
            highlighted: true
            enabled: false
        }

        Fluid.Button {
            text: qsTr("Flat Highlighted")
            flat: true
            highlighted: true
            enabled: false
        }
    }
	
	
Column
{
  Grid
  {
    width: 300
    spacing: 16

    columns: 2

    Repeater
    {
      model: [0, 1, 2, 3, 4, 6, 8, 12, 16, 24]

      delegate: Qaterial.Card
      {
        id: card

        Qaterial.Theme.elevation: modelData
        elevation: Qaterial.Theme.elevation
        outlined: Qaterial.Theme.elevation === 0
     //   backgroundColor: Qaterial.Style.colorTheme.getElevatedColor(Qaterial.Style.colorTheme.background0, Qaterial.Theme.elevation)
        backgroundColor: "red"
        width: 112
        height: 112

        Qaterial.LabelSubtitle1
        {
          anchors
          {
            left: parent.left
            top: parent.top

            leftMargin: 12
            topMargin: 12
          }

          text: `${card.Qaterial.Theme.elevation}dp`
        }

        Qaterial.LabelCaption
        {
          anchors
          {
            right: parent.right
            bottom: parent.bottom

            rightMargin: 12
            bottomMargin: 12
          }

          text: `Overlay : ${(Qaterial.Style.colorTheme.getOverlayForElevation(card.Qaterial.Theme.elevation)*100).toFixed(0)}%`
        }
      }
    }
  }

}

}
