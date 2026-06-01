import QtQuick
import Qt5Compat.GraphicalEffects

Image {
    id: colorImage
    property color color
    layer.enabled: true
    layer.effect: ColorOverlay {
        color: colorImage.color
    }
}
