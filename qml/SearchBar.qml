import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import QtQuick.Controls.Material

Pane {
    id: ericpane
    property bool filterFavorite: false
    property alias text: textField.displayText
    Material.elevation: 1
    Material.background: "white"
    leftPadding: 0
    topPadding: 0
    bottomPadding: 0
    rightPadding: 24
    signal wasclicked()

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        Item {
            width: 48 - rowLayout.spacing
            ColorImage {
                anchors.centerIn: parent
                source: "qrc:/assets/filterpic/search.svg"
                color: Material.color(Material.Grey)
            }
        }
        TextField {
            id: textField
            Layout.fillWidth: true
            topPadding: 0
            bottomPadding: 0
            focus: true
            selectByMouse: true
            background: null
            inputMethodHints: Qt.ImhSensitiveData |  Qt.ImhNoPredictiveText
            placeholderText: "Search Menu Items"
        }
        RoundButton {
            Material.elevation: 0
            contentItem: ColorImage  {
                source: filterFavorite ? "qrc:/assets/filterpic/star.svg" : "qrc:/assets/filterpic/star_border.svg"
                color: filterFavorite ? Material.color(Material.Amber) : Material.color(Material.Grey)
            }
            onClicked: {

                filterFavorite = !filterFavorite

                if(filterFavorite) {

                   ericpane.wasclicked()
                }



            }
        }
    }
}
