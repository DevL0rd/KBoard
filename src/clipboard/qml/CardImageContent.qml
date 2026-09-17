import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: content

    property string source
    property int imageWidth
    property int imageHeight
    property real maximumHeight: Kirigami.Units.gridUnit * 9

    readonly property real aspect: imageWidth > 0 ? imageHeight / imageWidth : 0.6

    implicitHeight: Math.round(Math.min(maximumHeight, width * Math.max(0.4, Math.min(1.2, aspect))))

    Kirigami.ShadowedTexture {
        anchors.fill: parent
        radius: ClipboardStyle.cardRadius
        color: "transparent"
        source: thumbnail.status === Image.Ready ? thumbnail : null
    }

    Image {
        id: thumbnail
        anchors.fill: parent
        visible: false
        source: content.source
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Kirigami.Units.iconSizes.medium
        height: width
        source: thumbnail.status === Image.Error ? "image-missing" : "image-x-generic"
        color: Kirigami.Theme.disabledTextColor
        visible: thumbnail.status !== Image.Ready
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: Kirigami.Units.smallSpacing * 1.5
        radius: height / 2
        height: sizeLabel.implicitHeight + Kirigami.Units.smallSpacing
        width: sizeLabel.implicitWidth + Kirigami.Units.largeSpacing * 1.5
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.8)

        Text {
            id: sizeLabel
            anchors.centerIn: parent
            text: content.imageWidth + " × " + content.imageHeight
            color: Kirigami.Theme.textColor
            font: Kirigami.Theme.smallFont
        }
    }
}
