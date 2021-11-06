import QtQuick 2.12

Image {
    id: root

    signal clicked()
    property string name
    property string variant: "normal"

    width: sourceSize.width
    height: sourceSize.height
    sourceSize: Qt.size(20, 20)
    source: "qrc:/window-border/macOS/" + name + "-" + variant + ".svg"

    MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        onReleased: root.clicked()
        acceptedButtons: Qt.LeftButton
        onContainsMouseChanged: root.variant = (containsMouse ? "hover" : "normal")
        onContainsPressChanged: root.variant = (containsPress ? "active" : "normal")
    }
}
