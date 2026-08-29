/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root

  Layout.fillWidth: true
  Layout.fillHeight: true
  implicitHeight: plottingLayout.implicitHeight + 16

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
    color: Cpp_ThemeManager.colors["groupbox_background"]
  }

  GridLayout {
    id: plottingLayout

    columns: 2
    rowSpacing: 4
    columnSpacing: 8
    anchors.margins: 8
    anchors.fill: parent

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.topMargin: 2
      Layout.columnSpan: 2
      text: qsTr("Data Plotting")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } Rectangle {
      implicitHeight: 1
      Layout.columnSpan: 2
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    } Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    }

    Label {
      text: qsTr("Time Range")
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _timeRange

      readonly property var presets: [0.001, 0.002, 0.005, 0.01, 0.02, 0.05,
                                      0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 30, 60, 120, 300]

      function nearestIndex(seconds) {
        var best = 0
        for (var i = 0; i < presets.length; ++i)
          if (Math.abs(presets[i] - seconds) < Math.abs(presets[best] - seconds))
            best = i

        return best
      }

      function formatSeconds(s) {
        return s < 1 ? (Math.round(s * 1000) + " ms") : (parseFloat(s.toFixed(3)) + " s")
      }

      from: 0
      to: presets.length - 1
      editable: true
      Layout.fillWidth: true
      Component.onCompleted: value = nearestIndex(Cpp_UI_Dashboard.plotTimeRange)
      textFromValue: function(value, locale) { return _timeRange.formatSeconds(presets[value]) }
      valueFromText: function(text, locale) {
        var t = String(text).toLowerCase()
        var num = parseFloat(t.replace(/[^0-9.]/g, ""))
        if (isNaN(num))
          return _timeRange.value

        var secs = (t.indexOf("ms") >= 0) ? num / 1000 : num
        return _timeRange.nearestIndex(secs)
      }
      onValueModified: {
        if (presets[value] !== Cpp_UI_Dashboard.plotTimeRange)
          Cpp_UI_Dashboard.plotTimeRange = presets[value]
      }

      Connections {
        target: Cpp_UI_Dashboard
        function onPlotTimeRangeChanged() {
          const idx = _timeRange.nearestIndex(Cpp_UI_Dashboard.plotTimeRange)
          if (_timeRange.value !== idx)
            _timeRange.value = idx
        }
      }
    }

    Label {
      text: qsTr("Point Count")
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _pointCount

      from: 10
      to: 100000
      stepSize: 10
      editable: true
      Layout.fillWidth: true
      value: Cpp_UI_Dashboard.points
      onValueChanged: {
        if (value !== Cpp_UI_Dashboard.points)
          Cpp_UI_Dashboard.points = value
      }

      Connections {
        target: Cpp_UI_Dashboard
        function onPointsChanged() {
          _pointCount.value = Cpp_UI_Dashboard.points
        }
      }
    }

    Label {
      text: qsTr("UI Refresh Rate (Hz)")
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _refreshRate

      from: 1
      to: 240
      editable: true
      Layout.fillWidth: true
      value: Cpp_Misc_TimerEvents.fps
      onValueChanged: {
        if (value !== Cpp_Misc_TimerEvents.fps)
          Cpp_Misc_TimerEvents.fps = value
      }

      Connections {
        target: Cpp_Misc_TimerEvents
        function onFpsChanged() {
          _refreshRate.value = Cpp_Misc_TimerEvents.fps
        }
      }
    }

    Item { Layout.fillHeight: true }
    Item { Layout.fillHeight: true }
  }
}
