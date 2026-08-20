/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../../Widgets" as Widgets

Window {
  id: root

  Widgets.WindowMirror {}

  width: 960
  height: 640
  minimumWidth: 720
  minimumHeight: 480
  flags: Qt.Dialog
  modality: Qt.NonModal
  title: qsTr("Canvas Live Preview")
  color: Cpp_ThemeManager.colors["window"]

  property string painterCode: ""
  property string groupTitle: qsTr("Preview")
  property var simulatedDatasets: []

  function showDialog(code, title, datasets) {
    root.painterCode        = code || ""
    root.groupTitle         = title || qsTr("Preview")
    root.simulatedDatasets  = (datasets && datasets.length > 0) ? datasets : root._defaultDatasets()
    root._rebuildSimList()
    show()
    raise()
    requestActivate()
  }

  function _defaultDatasets() {
    return [
      { title: "X", value:  0, min: -100, max: 100, units: "" },
      { title: "Y", value:  0, min: -100, max: 100, units: "" },
      { title: "Z", value: 50, min: -100, max: 100, units: "" }
    ]
  }

  function _rebuildSimList() {
    simModel.clear()
    for (let i = 0; i < root.simulatedDatasets.length; ++i) {
      const d = root.simulatedDatasets[i]
      simModel.append({
        title:  d.title  !== undefined ? d.title  : ("DS" + (i + 1)),
        value:  d.value  !== undefined ? d.value  : 0,
        min:    d.min    !== undefined ? d.min    : 0,
        max:    d.max    !== undefined ? d.max    : 100,
        units:  d.units  !== undefined ? d.units  : ""
      })
    }
    root._pushSnapshot()
  }

  function _pushSnapshot() {
    const arr = []
    for (let i = 0; i < simModel.count; ++i) {
      const r = simModel.get(i)
      arr.push({
        title: r.title,
        value: r.value,
        min:   r.min,
        max:   r.max,
        units: r.units
      })
    }
    previewWidget.setSimulatedDatasets(arr)
  }

  ListModel { id: simModel }

  Component.onCompleted: {
    previewWidget.setPreviewMode(true)
    previewWidget.setPreviewGroupTitle(root.groupTitle)
    previewWidget.setUserCode(root.painterCode)
  }

  onPainterCodeChanged: previewWidget.setUserCode(root.painterCode)
  onGroupTitleChanged:  previewWidget.setPreviewGroupTitle(root.groupTitle)

  //
  // Side panel: simulated values
  //
  Item {
    id: sidebar

    width: 280
    anchors {
      margins: 8
      top: parent.top
      right: parent.right
      bottom: parent.bottom
    }

    ColumnLayout {
      spacing: 6
      anchors.fill: parent

      Label {
        text: qsTr("Simulated datasets")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["text"]
      }

      ListView {
        id: simView

        clip: true
        spacing: 6
        model: simModel
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        delegate: Rectangle {
          width: simView.width - 8
          implicitHeight: rowCol.implicitHeight + 12
          color: Cpp_ThemeManager.colors["alternate_base"]
          border.color: Cpp_ThemeManager.colors["widget_border"]
          border.width: 1
          radius: 4

          ColumnLayout {
            id: rowCol

            spacing: 4
            anchors.margins: 6
            anchors.fill: parent

            RowLayout {
              spacing: 4
              Layout.fillWidth: true

              Label {
                text: model.title
                Layout.fillWidth: true
                elide: Text.ElideRight
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.uiFont
              }

              Label {
                text: model.value.toFixed(1) + (model.units ? " " + model.units : "")
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.monoFont
              }
            }

            Slider {
              from: model.min
              to:   model.max
              value: model.value
              Layout.fillWidth: true
              onMoved: {
                simModel.setProperty(index, "value", value)
                root._pushSnapshot()
              }
            }
          }
        }
      }

      Label {
        text: previewWidget.runtimeOk
              ? qsTr("Runtime OK")
              : (previewWidget.lastError || qsTr("Awaiting first frame..."))
        color: previewWidget.runtimeOk
               ? Cpp_ThemeManager.colors["text"]
               : Cpp_ThemeManager.colors["error"]
        font: Cpp_Misc_CommonFonts.monoFont
        wrapMode: Text.Wrap
        Layout.fillWidth: true
      }

      //
      // Console output (console.log/warn/error from the painter script)
      //
      Label {
        text: qsTr("Console")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["text"]
      }

      Rectangle {
        color: Cpp_ThemeManager.colors["base"]
        border.color: Cpp_ThemeManager.colors["widget_border"]
        border.width: 1
        radius: 2
        Layout.fillWidth: true
        Layout.preferredHeight: 140

        ListView {
          id: consoleView

          clip: true
          spacing: 1
          anchors.margins: 4
          model: consoleModel
          anchors.fill: parent
          boundsBehavior: Flickable.StopAtBounds

          ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

          delegate: Label {
            text: model.text
            wrapMode: Text.Wrap
            width: consoleView.width - 8
            font: Cpp_Misc_CommonFonts.monoFont
            color: model.level === 2 ? Cpp_ThemeManager.colors["error"]
                 : model.level === 1 ? Cpp_ThemeManager.colors["highlight"]
                 :                     Cpp_ThemeManager.colors["text"]
          }
        }
      }

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Button {
          text: qsTr("Clear console")
          onClicked: consoleModel.clear()
        }

        Item {
          Layout.fillWidth: true
        }

        Button {
          text: qsTr("Close")
          onClicked: root.close()
        }
      }
    }
  }

  ListModel { id: consoleModel }

  Connections {
    target: previewWidget
    function onConsoleLine(level, text) {
      consoleModel.append({ level: level, text: text })
      while (consoleModel.count > 200)
        consoleModel.remove(0)

      consoleView.positionViewAtEnd()
    }
  }

  //
  // Preview canvas
  //
  Rectangle {
    color: Cpp_ThemeManager.colors["base"]
    border.color: Cpp_ThemeManager.colors["widget_border"]
    border.width: 1
    anchors {
      margins: 8
      rightMargin: 8
      top: parent.top
      left: parent.left
      right: sidebar.left
      bottom: parent.bottom
    }

    PainterWidget {
      id: previewWidget

      anchors.margins: 1
      anchors.fill: parent
    }
  }
}
