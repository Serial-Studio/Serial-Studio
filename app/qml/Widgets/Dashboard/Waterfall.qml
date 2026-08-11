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

import "../" as Widgets

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
  readonly property bool hasToolbar: toolbar.shown

  //
  // Size-aware font scale: shrinks colorbar text as the window gets small
  //
  readonly property real uiScale: Cpp_Misc_CommonFonts.autoScale(Math.min(width, height), 260)

  //
  // Reparent the painted-item model into the central waterfall area
  //
  onModelChanged: {
    if (model) {
      model.parent = plotArea
      model.anchors.fill = plotArea
    }
  }

  //
  // Restore persisted settings; runs after ALL initial properties are applied, since
  // onModelChanged can fire while widgetId is still empty during object creation
  //
  Component.onCompleted: {
    if (!model)
      return

    const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

    if (s["colorMap"] !== undefined)
      model.colorMap = s["colorMap"]

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

    if (s["showFrequencyMarkers"] !== undefined)
      model.markersVisible = s["showFrequencyMarkers"]
  }

  //
  // Toolbar
  //
  WidgetToolbar {
    id: toolbar

    minWidgetHeight: 200
    windowRoot: root.windowRoot

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    Widgets.Combo {
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
          source: Cpp_Misc_IconRegistry.icon("commands", "color", 16)
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
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "color", 16)
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
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "abscissa", 16)
      onClicked: {
        if (!root.model)
          return

        root.model.axisVisible = !root.model.axisVisible
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "axisVisible", root.model.axisVisible)
      }
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Show Crosshair")
      checked: root.model && root.model.cursorEnabled
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "crosshair", 16)
      onClicked: {
        if (!root.model)
          return

        root.model.cursorEnabled = !root.model.cursorEnabled
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "cursorEnabled", root.model.cursorEnabled)
      }
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Show Frequency Markers")
      checked: root.model && root.model.markersVisible
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "labels", 16)
      onClicked: {
        if (!root.model)
          return

        root.model.markersVisible = !root.model.markersVisible
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId,
                                                "showFrequencyMarkers",
                                                root.model.markersVisible)
      }
    }

    Rectangle {
      visible: Cpp_CommercialBuild
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    //
    // Record the widget's time-domain input to WAV (Pro)
    //
    DashboardToolButton {
      visible: Cpp_CommercialBuild
      ToolTip.text: qsTr("Record Audio")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "audio-file", 16)
      checked: Cpp_CommercialBuild && root.model && root.model.audioRecordingEnabled
      onClicked: {
        if (root.model)
          root.model.audioRecordingEnabled = !root.model.audioRecordingEnabled
      }
    }

    //
    // Reveal the folder holding this widget's recorded WAV files (Pro)
    //
    DashboardToolButton {
      visible: Cpp_CommercialBuild
      ToolTip.text: qsTr("Open Recordings Folder")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "sound-folder", 16)
      onClicked: {
        if (root.model)
          Cpp_Misc_Utilities.revealFile(root.model.recordingsFolder())
      }
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      checked: !model.running
      ToolTip.text: model.running ? qsTr("Pause") : qsTr("Resume")
      icon.source: model.running
                   ? Cpp_Misc_IconRegistry.icon("commands", "pause", 16)
                   : Cpp_Misc_IconRegistry.icon("commands", "resume", 16)
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

    readonly property bool axesOn: root.model && root.model.axisVisible
    readonly property bool colorbarOn: root.model && root.model.colorbarVisible

    anchors {
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
      margins: contentArea.axesOn || contentArea.colorbarOn ? 8 : -1
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
        font: (Cpp_Misc_CommonFonts.widgetFontRevision,
               Cpp_Misc_CommonFonts.widgetFont(root.uiScale))
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

        MouseArea {
          id: colorbarHover

          hoverEnabled: true
          anchors.fill: parent
          cursorShape: Qt.CrossCursor
          acceptedButtons: Qt.NoButton

          readonly property real hoverDb: {
            if (!root.model || height <= 0)
              return 0

            const t = Math.max(0, Math.min(1, mouseY / height))
            return root.model.maxDb - t * (root.model.maxDb - root.model.minDb)
          }
        }

        ToolTip {
          delay: 0
          parent: colorbarStrip
          x: colorbarHover.mouseX + 12
          y: colorbarHover.mouseY + 12
          visible: colorbarHover.containsMouse
          text: colorbarHover.hoverDb.toFixed(1) + " dB"
        }
      }

      Label {
        id: colorbarMinLabel

        text: model ? model.minDb.toFixed(0) + " dB" : ""
        color: Cpp_ThemeManager.colors["text"]
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        font: (Cpp_Misc_CommonFonts.widgetFontRevision,
               Cpp_Misc_CommonFonts.widgetFont(root.uiScale))
      }
    }
  }

  //
  // Unavailable over a remote attach: an empty spectrum reads as a broken one
  //
  Rectangle {
    z: 1000
    anchors.fill: parent
    visible: Cpp_API_Mirror.attached
    color: Cpp_ThemeManager.colors["widget_base"]

    Label {
      anchors.centerIn: parent
      wrapMode: Text.WordWrap
      horizontalAlignment: Text.AlignHCenter
      width: Math.min(parent.width - 32, 320)
      color: Cpp_ThemeManager.colors["widget_text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9)
      text: qsTr("Not available over a remote attach. This widget needs the remote's raw sample "
               + "stream, which the dashboard mirror does not carry.")
    }
  }
}
