import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.impl
import QtQuick.Templates as T
import Qt.labs.qmlmodels

import ComponentLibrary

T.Popup {
    id: actionMenu

    implicitWidth: parent.width
    implicitHeight: actualHeight

    y: appWindow.height

    padding: 0
    margins: 0

    modal: true
    dim: true
    focus: appWindow.isMobile
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutside
    parent: T.Overlay.overlay

    property string titleTxt
    property string titleSrc // disabled

    property var model: null

    property int layoutDirection: Qt.LeftToRight

    signal menuSelected(var index)

    ////////////////

    property int actualHeight: {
        if (typeof mobileMenu !== "undefined" && mobileMenu.height)
            return contentColumn.height + appWindow.screenPaddingNavbar + appWindow.screenPaddingBottom
        return contentColumn.height
    }

    property bool opening: false
    property bool closing: false

    onAboutToShow: {
        opening = true
        closing = false
    }
    onAboutToHide: {
        opening = false
        closing = true
    }

    ////////////////

    enter: Transition { NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 233; } }
    exit: Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0.66; duration: 233; } }

    ////////////////

    T.Overlay.modal: Rectangle {
        color: "#000"
        opacity: EricTheme.isLight ? 0.24 : 0.48
    }

    background: Item { }

    contentItem: Item { }

    ////////////////

    Rectangle {
        id: actualPopup
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        height: opening ? actionMenu.actualHeight : 0
        Behavior on height { NumberAnimation { duration: 233 } }

        color: EricTheme.colorComponentBackground

        Rectangle { // separator
            anchors.left: parent.left
            anchors.right: parent.right
            height: EricTheme.componentBorderWidth
            color: EricTheme.colorSeparator
        }

        Column { // content
            id: contentColumn
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: screenPaddingLeft
            anchors.right: parent.right
            anchors.rightMargin: screenPaddingRight

            topPadding: EricTheme.componentMargin
            bottomPadding: 4
            spacing: 0

            ////

            Text { // title
                anchors.left: parent.left
                anchors.leftMargin: EricTheme.componentMargin + 4
                anchors.right: parent.right
                anchors.rightMargin: EricTheme.componentMargin

                height: EricTheme.componentHeight
                visible: actionMenu.titleTxt

                text: actionMenu.titleTxt
                textFormat: Text.PlainText

                color: EricTheme.colorSubText
                font.bold: false
                font.pixelSize: EricTheme.fontSizeContentVeryBig
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            ////

            Repeater {
                model: actionMenu.model
                delegate: DelegateChooser {
                    id: chooser
                    role: "t"
                    DelegateChoice {
                        roleValue: "sep"
                        ActionMenuItem_separator {
                            width: actionMenu.width
                        }
                    }
                    DelegateChoice {
                        roleValue: "itm"
                        ActionMenuItem_button {
                            width: actionMenu.width
                            height: EricTheme.componentHeightL
                            layoutDirection: actionMenu.layoutDirection
                            index: idx
                            text: txt
                            source: src
                            onClicked: {
                                actionMenu.menuSelected(idx)
                                actionMenu.close()
                            }
                        }
                    }
                }
            }

            ////
        }
    }

    ////////////////
}
