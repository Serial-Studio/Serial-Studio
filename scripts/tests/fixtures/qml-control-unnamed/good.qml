import QtQuick
import QtQuick.Controls

Item {
  id: root

  Label {
    text: qsTr("Baud Rate")
  } ComboBox {
    objectName: "setup.uart.baudRate"
    model: ["9600", "115200"]
  }
}
