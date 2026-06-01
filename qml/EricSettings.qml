pragma Singleton
import QtQuick

Item {

    property int wHeight
    property int wWidth

    // Colors
    readonly property color backgroundColor: "#2d3037"
    //readonly property color buttonColor: "#4CAF50"
    //readonly property color buttonPressedColor: "#6ccaf2"
    readonly property color buttonColor: "#009688"
    readonly property color buttonPressedColor: "#03A9F4"
    readonly property color disabledButtonColor: "#555555"
    readonly property color viewColor: "#202227"
    readonly property color maincolor: "#FF1371"
    readonly property color delegate1Color: Qt.darker(viewColor, 1.2)
    readonly property color delegate2Color: Qt.lighter(viewColor, 1.2)
    readonly property color maincolor1: Qt.darker(maincolor, 1.2)
    readonly property color maincolor2: Qt.lighter(maincolor, 1.2)
    readonly property color textColor: "#ffffff"
    readonly property color textDarkColor: "#232323"
    readonly property color disabledTextColor: "#777777"
    readonly property color sliderColor: "#6ccaf2"
    readonly property color errorColor: "#ba3f62"
    readonly property color infoColor: "#3fba62"
    readonly property color ericcolor : "#ff5733"
    readonly property string menu: "\ue5d2"
    readonly property string back: "\uE5C4"
    readonly property string navigateBefore: "\uE408"
    readonly property string navigateNext: "\uE409"
    readonly property string apps: "\uE5C3"
    readonly property string search: "\uE8B6"
    readonly property string download: "\ue258"
    readonly property string index: "\ue242"
    //Color
    readonly property color black: "#000000"
    readonly property color white: "#FFFFFF"
    readonly property color background: "#170E12"
    readonly property color snuff: "#F3E5F5"
    readonly property color mineShaft: "#212121"
    readonly property color flamingo: "#F04937"
    readonly property color goldenFizz: "#FDEF30"
    readonly property color apple: "#62BB46"
    readonly property color cerulean: "#0099D8"
    readonly property color maroonFlush: "#B01B55"
    readonly property color grayNurse: "#DFE1DF"
    readonly property color dustyGray: "#999999"
    readonly property color gallery: "#EEEEEE"
    readonly property color silverChalice: "#AFAFAF"
    readonly property color cadillac: "#B7537A"
    readonly property color voodoo: "#4E3A53"
    readonly property color tequila: "#FEE4CD"
    readonly property color placeholderText: "#7F7F7F"
    readonly property color daycolor: "#07bf97"
    readonly property color nightcolor: "#b16bee"




    readonly property color oneMain: "#FF1371"
    readonly property color pink: "#FFC1DC"
    readonly property color darkPink: "#CF2568"
    readonly property color lightPink: "#FFDEEC"
    readonly property color pinkLavenderBlush: "#FFEDF5"


    readonly property color twoMain: "#6DE4E9"
    readonly property color twoDark: "#819FA5"
    readonly property color twoLight: "#C3DDE2"
    readonly property color twoBlack: "#0E1617"


    readonly property color threeMain: "#24E082"
    readonly property color threeDark: "#83A581"
    readonly property color threeLight: "#C5E2C3"
    readonly property color threeBlack: "#0E170F"

    // Font sizes
    property real microFontSize: hugeFontSize * 0.2
    property real tinyFontSize: hugeFontSize * 0.4
    property real smallTinyFontSize: hugeFontSize * 0.5
    property real smallFontSize: hugeFontSize * 0.6
    property real mediumFontSize: hugeFontSize * 0.7
    property real bigFontSize: hugeFontSize * 0.8
    property real largeFontSize: hugeFontSize * 0.9
    property real hugeFontSize: (wWidth + wHeight) * 0.03
    property real giganticFontSize: (wWidth + wHeight) * 0.04

    // Some other values
    property real fieldHeight: wHeight * 0.08
    property real fieldMargin: fieldHeight * 0.5
    property real buttonHeight: wHeight * 0.08
    property real buttonRadius: buttonHeight * 0.1

    // Some help functions
    function widthForHeight(h, ss)
    {
        return h/ss.height * ss.width;
    }

    function heightForWidth(w, ss)
    {
        return w/ss.width * ss.height;
    }

    // Alpha must be between 0 and 1
    function transparentize(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

}
