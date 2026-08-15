import QtQuick
import QtQuick.Controls
import SerialStudio

//
// Every widget extension declares these four properties. Serial Studio sets them when it
// creates the widget: `model` carries the live data, `color` the dashboard accent colour,
// `windowRoot` the window the widget lives in, and `widgetId` the persistence key.
//
Item {
  id: root

  required property color color
  required property string widgetId
  required property var windowRoot
  required property ExtensionDataModel model

  readonly property real span: Math.max(model.maxValue - model.minValue, 1)
  readonly property real fill: Math.max(0, Math.min(1, (model.value - model.minValue) / span))

  function barColor() {
    switch (model.config["barColor"]) {
    case "amber":
      return "#e0a030"
    case "red":
      return "#d05050"
    default:
      return "#4caf50"
    }
  }

  Column {
    anchors.fill: parent
    anchors.margins: 8
    spacing: 6

    Label {
      text: model.title
      color: root.color
      elide: Text.ElideRight
      width: parent.width
    }

    Rectangle {
      width: parent.width
      height: 24
      radius: 3
      color: "#20000000"
      border.color: root.color
      border.width: 1

      Rectangle {
        radius: 2
        height: parent.height - 4
        color: root.barColor()
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        width: Math.max(0, (parent.width - 4) * root.fill)

        Behavior on width {
          NumberAnimation { duration: 80 }
        }
      }
    }

    Label {
      color: root.color
      visible: model.config["showValue"] === true
      text: model.text + (model.units.length > 0 ? " " + model.units : "")
    }
  }
}
