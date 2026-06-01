import QtQuick
import QtQuick.Controls



EricComboBox {
    id: root

    property string urlIcon
    property int roleName

    onCurrentIndexChanged: {
        // This operation is necessary because we need to update the icon in contentItem.
        root.urlIcon = root.iconRole ? root.model.data(root.model.index(
                                                           root.currentIndex,
                                                           0),
                                                       root.roleName) : ""
    }

    contentItem: Item {
        width: parent.width
        implicitHeight: 36

        LabelComboBox {
            id: _contentItem
            height: parent.height
            width: root.width

            text: root.displayText
            iconUrl: root.urlIcon
             // Palette is the only thing here
            textColor: root.enabled ? EricSettings.black : EricSettings.placeholderText
            iconAlignLeft: root.iconAlignLeft
        }
    }
}
