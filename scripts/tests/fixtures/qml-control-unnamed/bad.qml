import QtQuick
import QtQuick.Controls

Item {
  id: root

  Label {
    text: qsTr("Baud Rate")
  } ComboBox {
    model: ["9600", "115200"]
  }
}
