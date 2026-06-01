import QtQuick
import QtQuick.Controls
import "qrc:/"
import "qrc:/qml"
import ComponentLibrary

Rectangle {
    anchors.fill: parent
    property string pagename : "Welcome"

    color: EricTheme.colorHeader

    property int lastPage: 3
    property string entryPoint: "accountsetuppage"

    ////////////////////////////////////////////////////////////////////////////

    function open() {
        entryPoint = "accountsetuppage"

        if (!tutorialLoader.sourceComponent) {
            tutorialLoader.sourceComponent = componentTutorial
        }

        appContent.state = "Tutorial"
    }

    function reopen() {
        entryPoint = "aboutpage"

        if (!tutorialLoader.sourceComponent) {
            tutorialLoader.sourceComponent = componentTutorial
        }

        tutorialLoader.item.reset()

        appContent.state = "Tutorial"
    }

    ////////////////////////////////////////////////////////////////////////////

    Loader {
        id: tutorialLoader
        anchors.fill: parent

        sourceComponent: null
        asynchronous: false
    }

    ////////////////////////////////////////////////////////////////////////////

    Component {
        id: componentTutorial

        Item {
            id: itemTutorial

            function reset() {
                tutorialPages.disableAnimation()
                tutorialPages.currentIndex = 0
                tutorialPages.enableAnimation()
            }

            SwipeView {
                id: tutorialPages
                anchors.fill: parent
                anchors.leftMargin: screenPaddingLeft
                anchors.rightMargin: screenPaddingRight
                anchors.bottomMargin: 56

                currentIndex: 0
                onCurrentIndexChanged: {
                    if (currentIndex < 0) currentIndex = 0
                    if (currentIndex > lastPage) {
                        currentIndex = 0 // reset
                        appContent.state = entryPoint
                    }
                }

                function enableAnimation() {
                    contentItem.highlightMoveDuration = 333
                }
                function disableAnimation() {
                    contentItem.highlightMoveDuration = 0
                }

                ////////

                Item {
                    id: page1

                    Column {
                        id: column
                        anchors.right: parent.right
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 32

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("<b>Digital Bloom</b> Lorem ipsum dolor sit amet, consectetur '<b>adipiscing elit</b>' and '<b> Etiam ipsum sem</b>' posuere at velit vel, consectetur pulvinar diam '<b>Suspendisse nunc dui, sagittis sit amet eleifend at, mattis in lorem</b>' Ut blandit dignissim.")
                            textFormat: Text.StyledText
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            font.pixelSize: EricTheme.fontSizeContentBig
                            color: EricTheme.colorHeaderContent
                            horizontalAlignment: Text.AlignHCenter
                        }
                        IconSvg {
                            width: tutorialPages.width * (tutorialPages.height > tutorialPages.width ? 0.8 : 0.4)
                            height: width*0.229
                            anchors.horizontalCenter: parent.horizontalCenter

                            source: "qrc:/assets/tutorial/welcome-devices.svg"
                            color: EricTheme.colorHeaderContent
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("urna sapien vehicula lacus, luctus tempus ipsum augue at est <b>Nunc condimentum</b> Donec auctor commodo magna eget mattis. In arcu massa <b> ipsum eget velit varius, ut posuere ipsum fringilla. Mauris posuere rhoncus lorem</b> and <b>Sed tincidunt aliquet velit, et efficitur orci commodo vitae</b>!")
                            textFormat: Text.StyledText
                            color: EricTheme.colorHeaderContent
                            font.pixelSize: EricTheme.fontSizeContentBig
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }
                    }
                }

                Item {
                    id: page2

                    Column {
                        anchors.right: parent.right
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 32

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("To start using Digital Bloom, you'll need to <b>Donec interdum neque vel</b> for <b>compatible Bluetooth sensors</b> sapien molestie, at consectetur augue molestie. Lorem ipsum dolor sit amet, consectetur adipiscing elit.")
                            textFormat: Text.StyledText
                            color: EricTheme.colorHeaderContent
                            font.pixelSize: EricTheme.fontSizeContentBig
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            horizontalAlignment: Text.AlignHCenter
                        }
                        IconSvg {
                            width: tutorialPages.width * (tutorialPages.height > tutorialPages.width ? 0.8 : 0.4)
                            height: width*0.777
                            anchors.horizontalCenter: parent.horizontalCenter

                            source: "qrc:/assets/tutorial/welcome-bluetooth-searching.svg"
                            color: EricTheme.colorHeaderContent
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("You can <b>Vestibulum ante ipsum</b> primis in faucibus orci luctus et ultrices posuere cubilia curae, or <b> consectetur adipiscing elit.</b> Phasellus porttitor gravida felis, eu elementum est sagittis nec.")
                            textFormat: Text.StyledText
                            font.pixelSize: EricTheme.fontSizeContentBig
                            color: EricTheme.colorHeaderContent
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }

                Item {
                    id: page3

                    Column {
                        anchors.right: parent.right
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 32

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("Vestibulum rutrum vitae elit nec bibendum <b>In hac habitasse platea dictumst</b>, Cras quam tellus, egestas eu aliquet id, venenatis vitae elit <b>Aliquam ex dui</b> maximus vel leo non.")
                            textFormat: Text.StyledText
                            color: EricTheme.colorHeaderContent
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: EricTheme.fontSizeContentBig
                        }
                        IconSvg {
                            width: tutorialPages.width * (tutorialPages.height > tutorialPages.width ? 0.8 : 0.4)
                            height: width*0.229
                            anchors.horizontalCenter: parent.horizontalCenter

                            source: "qrc:/assets/tutorial/welcome-app-connected.svg"
                            color: EricTheme.colorHeaderContent
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("In feugiat sit amet ante eu dapibus <b>Donec sit amet ex est</b>, <b>Nunc mattis dui non nibh dignissim</b> quis facilisis augue dignissim <b>Sed sed faucibus orci, a rutrum ex</b>.")
                            textFormat: Text.StyledText
                            color: EricTheme.colorHeaderContent
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: EricTheme.fontSizeContentBig
                        }
                    }
                }

                Item {
                    id: page4

                    Column {
                        anchors.right: parent.right
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 32

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("Integer facilisis luctus nisl, et interdum nibh malesuada eu <b>Ut rhoncus pulvinar fringilla</b> Vivamus sed porttitor urna <b>Proin a facilisis tortor</b>.")
                            textFormat: Text.StyledText
                            color: EricTheme.colorHeaderContent
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: EricTheme.fontSizeContentBig
                        }
                        IconSvg {
                            width: tutorialPages.width * (tutorialPages.height > tutorialPages.width ? 0.8 : 0.4)
                            height: width*0.328
                            anchors.horizontalCenter: parent.horizontalCenter

                            source: "qrc:/assets/tutorial/welcome-limits.svg"
                            color: EricTheme.colorHeaderContent
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 32
                            anchors.left: parent.left
                            anchors.leftMargin: 32

                            text: qsTr("Nam in ligula erat <b>Etiam interdum tellus non sapien viverra</b> Maecenas eu molestie libero <b>Orci varius natoque penatibus et magnis dis parturient montes</b> nascetur ridiculus mus <b>Nunc ultrices nulla molestie eros</b> Aenean tortor est, dapibus eu lectus vitae, eleifend placerat sem.")
                            textFormat: Text.StyledText
                            color: EricTheme.colorHeaderContent
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: EricTheme.fontSizeContentBig
                        }
                    }
                }
            }

            ////////

            Text {
                id: pagePrevious
                anchors.left: parent.left
                anchors.leftMargin: 32
                anchors.verticalCenter: pageIndicator.verticalCenter

                visible: (tutorialPages.currentIndex !== 0)

                text: qsTr("Previous")
                textFormat: Text.PlainText
                color: EricTheme.colorHeaderContent
                font.bold: true
                font.pixelSize: EricTheme.fontSizeContent

                Behavior on opacity { OpacityAnimator { duration: 133 } }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 0.8
                    onExited: parent.opacity = 1
                    onClicked: tutorialPages.currentIndex--
                }
            }

            PageIndicatorThemed {
                id: pageIndicator
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16

                count: tutorialPages.count
                currentIndex: tutorialPages.currentIndex
            }

            Text {
                id: pageNext
                anchors.right: parent.right
                anchors.rightMargin: 32
                anchors.verticalCenter: pageIndicator.verticalCenter

                text: (tutorialPages.currentIndex === lastPage) ? qsTr("All right!") : qsTr("Next")
                textFormat: Text.PlainText
                color: EricTheme.colorHeaderContent
                font.bold: true
                font.pixelSize: EricTheme.fontSizeContent

                Behavior on opacity { OpacityAnimator { duration: 133 } }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 0.8
                    onExited: parent.opacity = 1
                    onClicked: tutorialPages.currentIndex++
                }
            }
        }
    }
}
