/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property WaterfallModel model

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Default minimum size for toolbar visibility
  //
  onWidthChanged: updateWidgetOptions()
  onHeightChanged: updateWidgetOptions()
  function updateWidgetOptions() {
    root.hasToolbar = (root.width >= toolbar.implicitWidth) && (root.height >= 200)
  }

  //
  // Reparent the painted-item model into the central waterfall area
  //
  onModelChanged: restoreSettings()
  function restoreSettings() {
    if (!model)
      return

    model.parent = plotArea
    model.anchors.fill = plotArea

    const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

    if (s["colorMap"] !== undefined)
      model.colorMap = s["colorMap"]

    if (s["historySize"] !== undefined)
      model.historySize = s["historySize"]

    if (s["minDb"] !== undefined)
      model.minDb = s["minDb"]

    if (s["maxDb"] !== undefined)
      model.maxDb = s["maxDb"]

    if (s["axisVisible"] !== undefined)
      model.axisVisible = s["axisVisible"]

    if (s["cursorEnabled"] !== undefined)
      model.cursorEnabled = s["cursorEnabled"]

    if (s["colorbarVisible"] !== undefined)
      model.colorbarVisible = s["colorbarVisible"]

    root.updateWidgetOptions()
  }

  //
  // Toolbar
  //
  RowLayout {
    id: toolbar

    spacing: 4
    visible: root.hasToolbar
    height: root.hasToolbar ? 48 : 0

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    ComboBox {
      id: colorMapCombo

      implicitWidth: 80
      implicitHeight: 28
      Layout.fillWidth: true
      Layout.maximumWidth: 140
      Layout.alignment: Qt.AlignVCenter
      currentIndex: root.model ? root.model.colorMap : 0

      model: {
        if (!root.model)
          return []

        const arr = []
        for (let i = 0; i < root.model.colorMapCount; ++i)
          arr.push(root.model.colorMapName(i))

        return arr
      }

      onActivated: {
        if (root.model) {
          root.model.colorMap = currentIndex
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "colorMap", currentIndex)
        }
      }

      contentItem: RowLayout {
        spacing: 6

        Image {
          Layout.leftMargin: 8
          Layout.preferredWidth: 16
          Layout.preferredHeight: 16
          sourceSize: Qt.size(16, 16)
          Layout.alignment: Qt.AlignVCenter
          fillMode: Image.PreserveAspectFit
          source: "qrc:/icons/dashboard-buttons/color.svg"
        }

        Label {
          elide: Text.ElideRight
          Layout.fillWidth: true
          font: colorMapCombo.font
          text: colorMapCombo.displayText
          Layout.alignment: Qt.AlignVCenter
          verticalAlignment: Text.AlignVCenter
          color: Cpp_ThemeManager.colors["button_text"]
        }
      }
    }

    Item {
      implicitWidth: 1
    }

    Label {
      id: minDbLabel

      Layout.alignment: Qt.AlignVCenter
      horizontalAlignment: Text.AlignRight
      color: Cpp_ThemeManager.colors["text"]
      text: (root.model ? Math.round(root.model.minDb) : -100) + " dB"
    }

    RangeSlider {
      id: rangeSlider

      to: 20
      from: -120
      stepSize: 1
      implicitWidth: 120
      implicitHeight: 28
      snapMode: RangeSlider.SnapAlways
      Layout.alignment: Qt.AlignVCenter
      second.value: root.model ? root.model.maxDb : 0
      first.value: root.model ? root.model.minDb : -100

      first.onMoved: {
        if (!root.model)
          return

        const v = Math.round(first.value)
        if (v < Math.round(second.value)) {
          root.model.minDb = v
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "minDb", v)
        }
      }

      second.onMoved: {
        if (!root.model)
          return

        const v = Math.round(second.value)
        if (v > Math.round(first.value)) {
          root.model.maxDb = v
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "maxDb", v)
        }
      }
    }

    Label {
      id: maxDbLabel

      Layout.alignment: Qt.AlignVCenter
      horizontalAlignment: Text.AlignLeft
      color: Cpp_ThemeManager.colors["text"]
      text: (root.model ? Math.round(root.model.maxDb) : 0) + " dB"
    }

    Item {
      implicitWidth: 1
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Show Colorbar")
      checked: root.model && root.model.colorbarVisible
      icon.source: "qrc:/icons/dashboard-buttons/color.svg"
      onClicked: {
        if (!root.model)
          return

        root.model.colorbarVisible = !root.model.colorbarVisible
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "colorbarVisible", root.model.colorbarVisible)
      }
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Show Axes & Grid")
      checked: root.model && root.model.axisVisible
      icon.source: "qrc:/icons/dashboard-buttons/abscissa.svg"
      onClicked: {
        if (!root.model)
          return

        root.model.axisVisible = !root.model.axisVisible
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "axisVisible", root.model.axisVisible)
      }
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Show Crosshair")
      checked: root.model && root.model.cursorEnabled
      icon.source: "qrc:/icons/dashboard-buttons/crosshair.svg"
      onClicked: {
        if (!root.model)
          return

        root.model.cursorEnabled = !root.model.cursorEnabled
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "cursorEnabled", root.model.cursorEnabled)
      }
    }

    DashboardToolButton {
      checked: !model.running
      ToolTip.text: model.running ? qsTr("Pause") : qsTr("Resume")
      icon.source: model.running
                   ? "qrc:/icons/dashboard-buttons/pause.svg"
                   : "qrc:/icons/dashboard-buttons/resume.svg"
      onClicked: model.running = !model.running
    }

    Item {
      Layout.fillWidth: true
    }
  }

  //
  // Main content
  //
  Item {
    id: contentArea

    readonly property int outerMargin: axesOn || colorbarOn ? 8 : 0
    readonly property bool axesOn: root.model && root.model.axisVisible
    readonly property bool colorbarOn: root.model && root.model.colorbarVisible

    anchors {
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
      topMargin: contentArea.outerMargin
      leftMargin: contentArea.outerMargin
      bottomMargin: contentArea.outerMargin
      rightMargin: contentArea.colorbarOn ? 8 : 0
    }

    //
    // Plot area
    //
    Item {
      id: plotArea

      anchors {
        top: parent.top
        left: parent.left
        bottom: parent.bottom
        rightMargin: contentArea.colorbarOn ? 8 : 0
        right: contentArea.colorbarOn ? colorbarColumn.left : parent.right
      }
    }

    //
    // Colorbar
    //
    Item {
      id: colorbarColumn

      width: 56
      visible: contentArea.colorbarOn

      anchors {
        top: parent.top
        right: parent.right
        bottom: parent.bottom
      }

      Label {
        id: colorbarMaxLabel

        anchors.top: parent.top
        color: Cpp_ThemeManager.colors["text"]
        anchors.horizontalCenter: parent.horizontalCenter
        text: model ? model.maxDb.toFixed(0) + " dB" : ""
        font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(1.0))
      }

      Rectangle {
        id: colorbarStrip

        width: 18
        border.width: 1
        border.color: Cpp_ThemeManager.colors["widget_border"]

        anchors {
          topMargin: 4
          bottomMargin: 4
          top: colorbarMaxLabel.bottom
          bottom: colorbarMinLabel.top
          horizontalCenter: parent.horizontalCenter
        }

        Canvas {
          id: colorbarCanvas

          anchors.margins: 1
          anchors.fill: parent

          Connections {
            target: model
            function onColorMapChanged() {
              colorbarCanvas.requestPaint()
            }
          }

          onPaint: {
            const ctx = getContext("2d")
            const w   = width
            const h   = height
            ctx.clearRect(0, 0, w, h)
            if (!root.model)
              return

            const steps = Math.max(64, Math.floor(h))
            for (let i = 0; i < steps; ++i) {
              const t  = 1.0 - i / (steps - 1)
              const c  = root.model.colorAt(t)
              ctx.fillStyle = c
              const y0 = i * h / steps
              const y1 = (i + 1) * h / steps
              ctx.fillRect(0, y0, w, y1 - y0 + 1)
            }
          }
        }
      }

      Label {
        id: colorbarMinLabel

        text: model ? model.minDb.toFixed(0) + " dB" : ""
        color: Cpp_ThemeManager.colors["text"]
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        font: (Cpp_Misc_CommonFonts.widgetFontRevision,
               Cpp_Misc_CommonFonts.widgetFont(1.0))
      }
    }
  }
}
