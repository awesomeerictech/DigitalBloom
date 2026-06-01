import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl
import ComponentLibrary
import QtQuick.Templates  as T


T.ComboBox  {
    id: control

    property string iconRole
    property color accentColor

    property bool iconAlignLeft: true

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(
                        implicitBackgroundHeight + topInset + bottomInset,
                        implicitContentHeight + topPadding + bottomPadding,
                        implicitIndicatorHeight + topPadding + bottomPadding)

    leftPadding: padding + (!control.mirrored || !indicator
                            || !indicator.visible ? 0 : indicator.width + spacing)
    rightPadding: padding + (control.mirrored || !indicator
                             || !indicator.visible ? 0 : indicator.width + spacing)

    delegate: ItemDelegate {
        width: parent.width
        implicitHeight: 36
        padding: 0

        font.weight: Font.Normal
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled

        LabelComboBox {
            id: _content
            height: parent.height
            width: control.iconAlignLeft ? _content.implicitWidth : parent.width

            text: control.textRole ? (Array.isArray(
                                          control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData

            iconUrl: control.iconRole ? (Array.isArray(
                                             control.model) ? modelData[control.iconRole] : model[control.iconRole]) : ""

            textColor: control.currentIndex === index ? control.accentColor : EricSettings.black
            iconAlignLeft: control.iconAlignLeft
        }

        ColorImage {
            visible: index === 0
            x: control.mirrored ? control.padding : control.width - width - control.padding
            y: control.topPadding + (control.availableHeight - height) / 2
            color: EricSettings.placeholderText
            defaultColor: EricSettings.placeholderText
            source: "qrc:/qt-project.org/imports/QtQuick/Controls/Material/images/drop-indicator.png"
        }

        background: Rectangle {
            color: parent.highlighted ? EricSettings.gallery : "transparent"
            radius: 5
        }
    }

    indicator: ColorImage {
        x: control.mirrored ? control.padding : control.width - width - control.padding
        y: control.topPadding + (control.availableHeight - height) / 2
        color: EricSettings.placeholderText
        defaultColor: EricSettings.placeholderText
        source: "qrc:/qt-project.org/imports/QtQuick/Controls/Material/images/drop-indicator.png"
    }

    contentItem: T.TextField {
        leftPadding: !control.mirrored ? 16 : control.editable
                                         && activeFocus ? 3 : 1
        rightPadding: control.mirrored ? 16 : control.editable
                                         && activeFocus ? 3 : 1
        topPadding: 6 - control.padding
        bottomPadding: 6 - control.padding

        text: control.editable ? control.editText : control.displayText

        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator

        font: control.font
        color: control.enabled ? EricSettings.black : EricSettings.placeholderText
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        verticalAlignment: Text.AlignVCenter

        background: Rectangle {
            visible: control.enabled && control.editable && !control.flat
            color: EricSettings.white
        }
    }

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: 36
        radius: 5

        color: control.down ? EricSettings.dustyGray : control.palette.button
        border.color: control.palette.highlight
        border.width: !control.editable && control.visualFocus ? 2 : 0
        visible: !control.flat || control.down
    }

    popup: T.Popup {
        y: control.editable ? control.height - 5 : 0
        width: control.width
        height: Math.min(contentItem.implicitHeight,
                         control.Window.height - topMargin - bottomMargin)
        topMargin: 6
        bottomMargin: 6

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            highlightMoveDuration: 0

            Rectangle {
                z: 10
                width: parent.width
                height: parent.height
                color: "transparent"
                radius: 5
            }

       /*     T.ScrollIndicator.vertical: ScrollIndicator {
            
           
            
            
            } */
            
          ScrollBar.vertical : ScrollBar {

                                 policy: ScrollBar.AsNeeded
                                 contentItem: Rectangle {
                                 implicitWidth: 5
                                 implicitHeight: 5
                                 color:  (EricTheme.currentTheme === EricTheme.THEME_NIGHT) ? Qt.darker("orange",1.2) :  Qt.darker("teal", 1.2)
                                 
                                     }

                                 active: true;

                                 onActiveChanged: {

                                   if (!active)
                                    active = true;
                                  }
                            }
        }

        background: Rectangle {
            color: EricSettings.white
            radius: 5
        }
    }
}
