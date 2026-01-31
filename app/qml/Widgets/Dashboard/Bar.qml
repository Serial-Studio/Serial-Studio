/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
  required property BarModel model

  property real value: 0
  Behavior on value {NumberAnimation{duration: 100}}

  //
  // Repaint widget when needed
  //
  Connections {
    target: root.model
    function onUpdated() {
      root.value = model.value
    }
  }

  //
  // Helper properties
  //
  readonly property bool isHorizontal: root.width > 1.5 * root.height
  readonly property color fillColor: model.alarmTriggered ? Cpp_ThemeManager.colors["alarm"] : root.color
  readonly property real fontSize: Math.max(8, Math.min(11, Math.min(root.width, root.height) / 30))
  readonly property int tickCount: {
    if (isHorizontal) {
      const availableWidth = root.width - 100
      const labelWidth = labelMetrics.width
      return Math.max(3, Math.min(7, Math.floor(availableWidth / (labelWidth * 2))))
    } else {
      const availableHeight = root.height - 100
      const labelHeight = labelMetrics.height
      return Math.max(3, Math.min(7, Math.floor(availableHeight / (labelHeight * 3))))
    }
  }

  function formatValue(val) {
    return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
  }

  TextMetrics {
    id: labelMetrics
    font.pixelSize: fontSize
    font.family: Cpp_Misc_CommonFonts.monoFont.family
    text: {
      const a = formatValue(model.minValue)
      const b = formatValue(model.maxValue)
      return (a.length >= b.length) ? a : b
    }
  }

  //
  // Main layout
  //
  ColumnLayout {
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    Item {
      Layout.fillHeight: true
      Layout.fillWidth: true
    }

    //
    // Bar widget
    //
    Item {
      id: barContainer
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.preferredHeight: isHorizontal ? Math.min(80, root.height * 0.5) : root.height * 0.7
      Layout.preferredWidth: !isHorizontal ? Math.min(root.width * 0.6, 300) : undefined
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

      ProgressBar {
        id: progressBar
        from: model.minValue
        to: model.maxValue
        value: root.value

        anchors.centerIn: parent
        width: isHorizontal ? parent.width - labelMetrics.width : Math.min(80, parent.width * 0.5)
        height: isHorizontal ? Math.min(50, parent.height * 0.7) : parent.height - labelMetrics.height * 2

        background: Rectangle {
          implicitWidth: 200
          implicitHeight: 30
          color: Cpp_ThemeManager.colors["widget_base"]
          border.width: 2
          border.color: Cpp_ThemeManager.colors["widget_border"]
          radius: 3
        }

        contentItem: Item {
          implicitWidth: 200
          implicitHeight: 30

          Rectangle {
            width: isHorizontal ? progressBar.visualPosition * parent.width : parent.width
            height: isHorizontal ? parent.height : (1 - progressBar.visualPosition) * parent.height
            y: isHorizontal ? 0 : progressBar.visualPosition * parent.height
            radius: 2
            color: fillColor
            Behavior on color {ColorAnimation{duration: 200}}
          }
        }

        Repeater {
          model: tickCount
          delegate: Item {
            required property int index
            readonly property real frac: index / (tickCount - 1)
            readonly property real tickValue: model.minValue + frac * (model.maxValue - model.minValue)

            Rectangle {
              x: isHorizontal ? (frac * progressBar.width - width / 2) : progressBar.width
              y: isHorizontal ? progressBar.height : ((1 - frac) * progressBar.height - height / 2)
              width: isHorizontal ? 2 : 8
              height: isHorizontal ? 8 : 2
              color: Cpp_ThemeManager.colors["widget_border"]
            }

            Text {
              x: isHorizontal ? (frac * progressBar.width - width / 2) : progressBar.width + 10
              y: isHorizontal ? progressBar.height + 10 : ((1 - frac) * progressBar.height - height / 2)
              text: formatValue(parent.tickValue)
              font.pixelSize: fontSize
              font.family: Cpp_Misc_CommonFonts.monoFont.family
              color: Cpp_ThemeManager.colors["widget_text"]
            }
          }
        }
      }
    }

    Item {
      Layout.fillHeight: true
      Layout.fillWidth: true
    }

    //
    // Range/scale + current value display
    //
    VisualRange {
      id: range
      value: model.value
      units: model.units
      rangeVisible: false
      maxValue: model.maxValue
      minValue: model.minValue
      alarm: model.alarmTriggered
      maximumWidth: Math.min(root.width * 0.8, 200)
      Layout.alignment: Qt.AlignHCenter
      Layout.minimumWidth: implicitWidth
    }

    Item {
      implicitHeight: 4
    }
  }
}
