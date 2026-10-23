import QtQuick
import QtQuick.Controls

Item {
  id: root

  required property int widgetIndex

  Button {
    objectName: "io.toggleConnection"
    text: qsTr("Connect")
  }

  Button {
    objectName: "dashboard.tile." + root.widgetIndex + ".close"
    text: qsTr("Close")
  }
}
