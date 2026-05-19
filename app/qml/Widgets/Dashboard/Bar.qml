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
import QtQuick.Effects
import QtQuick.Layouts

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
  required property string widgetId

  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {NumberAnimation{duration: 120; easing.type: Easing.OutCubic}}

  //
  // Helper properties
  //
  readonly property bool isHorizontal: root.width > 1.5 * root.height
  readonly property bool showLabels: isHorizontal ? root.width >= 200 : root.height >= 160
  readonly property color fillColor: model.alarmTriggered ? Cpp_ThemeManager.colors["alarm"] : root.color
  readonly property real fontSize: Math.max(10, Math.min(14, Math.min(root.width, root.height) / 22))
                                   * Cpp_Misc_CommonFonts.widgetFontScale
  readonly property int tickCount: {
    if (model.displayTickCount > 0) return model.displayTickCount
    if (isHorizontal) {
      const availableWidth = Math.max(20, root.width - labelMetrics.width * 4)
      return Math.max(3, Math.min(9, Math.floor(availableWidth / (labelMetrics.width * 2))))
    } else {
      const availableHeight = Math.max(20, root.height - labelMetrics.height * 6)
      return Math.max(3, Math.min(9, Math.floor(availableHeight / (labelMetrics.height * 3))))
    }
  }
  readonly property bool labelsFit: {
    if (tickCount < 2) return true
    if (isHorizontal) {
      const perTick = (root.width - 16) / (tickCount - 1)
      return perTick > labelMetrics.width + 8
    }
    const perTick = (root.height - 16) / (tickCount - 1)
    return perTick > labelMetrics.height + 4
  }

  function formatValue(val) {
    const fmt = model.displayFormat
    if (!fmt || fmt === "" || fmt === "auto")
      return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)

    if (fmt === "0d") return Math.round(val).toString()
    if (fmt === "1d") return val.toFixed(1)
    if (fmt === "2d") return val.toFixed(2)
    if (fmt === "3d") return val.toFixed(3)
    if (fmt === "sci") return val.toExponential(2)
    const fm = fmt.match(/^%[\d\.]*\.(\d+)f$/)
    if (fm) return val.toFixed(parseInt(fm[1]))
    const fe = fmt.match(/^%[\d\.]*\.(\d+)e$/)
    if (fe) return val.toExponential(parseInt(fe[1]))
    return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
  }

  TextMetrics {
    id: labelMetrics

    font.pixelSize: fontSize
    font.family: Cpp_Misc_CommonFonts.widgetFontFamily
    text: {
      const a = formatValue(model.minValue)
      const b = formatValue(model.maxValue)
      return (a.length >= b.length) ? a : b
    }
  }

  //
  // Main layout
  //
  GridLayout {
    id: barLayout

    rows: 2
    columns: 1
    rowSpacing: 4
    anchors.margins: 8
    anchors.fill: parent

    //
    // Bar widget
    //
    Item {
      id: barContainer

      Layout.row: 0
      Layout.column: 0
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

      //
      // Hidden source for the soft drop shadow (single MultiEffect pass)
      //
      Rectangle {
        id: progressBarShadowSrc

        visible: false
        anchors.centerIn: progressBar
        width: progressBar.width
        height: progressBar.height
        radius: 4
        color: Cpp_ThemeManager.colors["widget_base"]
      }
      MultiEffect {
        shadowBlur: 0.45
        shadowEnabled: true
        shadowOpacity: 0.25
        shadowColor: "#000000"
        shadowVerticalOffset: 2
        source: progressBarShadowSrc
        anchors.fill: progressBarShadowSrc
      }

      Item {
        id: progressBar

        anchors.centerIn: parent
        width: isHorizontal
               ? Math.max(40, parent.width - (showLabels ? labelMetrics.width + 12 : 8))
               : Math.max(28, Math.min(parent.width * 0.55, 220))
        height: isHorizontal
               ? Math.max(28, Math.min(parent.height * 0.65, 220))
               : Math.max(40, parent.height - (showLabels ? labelMetrics.height * 2 + 8 : 8))

        readonly property real fillFrac: root.normalizedValue

        //
        // Bezel frame: light at top edge, darker at bottom
        //
        Rectangle {
          id: bezel

          radius: 4
          border.width: 2
          anchors.fill: parent
          antialiasing: true
          border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.2)
          gradient: Gradient {
            orientation: isHorizontal ? Gradient.Vertical : Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.15) }
            GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_base"] }
            GradientStop { position: 1.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.10) }
          }
        }

        //
        // Inner well (cream/lighter interior, sits inside the frame)
        //
        Rectangle {
          id: innerWell

          radius: 3
          clip: true
          border.width: 1
          anchors.margins: 3
          antialiasing: true
          anchors.fill: parent
          border.color: Qt.rgba(0, 0, 0, 0.18)
          gradient: Gradient {
            orientation: isHorizontal ? Gradient.Vertical : Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.20) }
            GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.06) }
          }

          //
          // Coloured fill -- grows from low end of the range
          //
          Rectangle {
            id: fillRect

            radius: 2
            antialiasing: true
            readonly property real innerW: innerWell.width - innerWell.border.width * 2
            readonly property real innerH: innerWell.height - innerWell.border.width * 2
            x: innerWell.border.width
            y: isHorizontal
               ? innerWell.border.width
               : innerWell.border.width + (1 - progressBar.fillFrac) * fillRect.innerH
            width: isHorizontal
                   ? Math.max(0, progressBar.fillFrac * fillRect.innerW)
                   : fillRect.innerW
            height: isHorizontal
                    ? fillRect.innerH
                    : Math.max(0, progressBar.fillFrac * fillRect.innerH)

            gradient: Gradient {
              orientation: isHorizontal ? Gradient.Horizontal : Gradient.Vertical
              GradientStop { position: 0.0; color: Qt.lighter(root.fillColor, 1.20) }
              GradientStop { position: 0.5; color: root.fillColor }
              GradientStop { position: 1.0; color: Qt.darker(root.fillColor, 1.10) }
            }

            Rectangle {
              x: 0
              width: 1
              visible: !isHorizontal
              anchors.top: parent.top
              anchors.bottom: parent.bottom
              color: Qt.rgba(1, 1, 1, 0.30)
            }
            Rectangle {
              y: 0
              height: 1
              visible: isHorizontal
              anchors.left: parent.left
              anchors.right: parent.right
              color: Qt.rgba(1, 1, 1, 0.30)
            }
          }
        }

        //
        // Major ticks + labels
        //
        Repeater {
          model: tickCount
          delegate: Item {
            required property int index
            readonly property real frac: index / (tickCount - 1)
            readonly property real tickValue: root.model.minValue + frac * (root.model.maxValue - root.model.minValue)

            Rectangle {
              width: isHorizontal ? 1.5 : 8
              height: isHorizontal ? 8 : 1.5
              radius: 0.75
              color: Cpp_ThemeManager.colors["widget_border"]
              x: isHorizontal ? (parent.frac * progressBar.width - width / 2) : progressBar.width
              y: isHorizontal ? progressBar.height : ((1 - parent.frac) * progressBar.height - height / 2)
            }

            Text {
              visible: showLabels && root.labelsFit
              font.pixelSize: fontSize
              text: formatValue(parent.tickValue)
              color: Cpp_ThemeManager.colors["widget_text"]
              font.family: Cpp_Misc_CommonFonts.widgetFontFamily
              x: isHorizontal ? (parent.frac * progressBar.width - width / 2) : progressBar.width + 10
              y: isHorizontal ? progressBar.height + 10 : ((1 - parent.frac) * progressBar.height - height / 2)
            }
          }
        }

        //
        // Minor ticks between majors (4 between each pair)
        //
        Repeater {
          model: (tickCount - 1) * 4
          delegate: Rectangle {
            required property int index
            readonly property int subIndex: (index % 4) + 1
            readonly property int majorIndex: Math.floor(index / 4)
            readonly property real frac: (majorIndex + subIndex / 5) / (tickCount - 1)

            width: isHorizontal ? 1 : 4
            height: isHorizontal ? 4 : 1
            opacity: 0.65
            color: Cpp_ThemeManager.colors["widget_border"]
            x: isHorizontal ? (frac * progressBar.width - width / 2) : progressBar.width
            y: isHorizontal ? progressBar.height : ((1 - frac) * progressBar.height - height / 2)
          }
        }

        Rectangle {
          x: isHorizontal ? root.model.normalizedAlarmLow * progressBar.width - width / 2
                          : progressBar.width
          y: isHorizontal ? progressBar.height
                          : (1 - root.model.normalizedAlarmLow) * progressBar.height - height / 2
          radius: 1
          width: isHorizontal ? 3 : 12
          height: isHorizontal ? 12 : 3
          visible: root.model.alarmsDefined
                   && root.model.alarmLow > root.model.minValue
                   && root.model.alarmLow < root.model.maxValue
          color: Cpp_ThemeManager.colors["alarm"]
        }

        Rectangle {
          x: isHorizontal ? root.model.normalizedAlarmHigh * progressBar.width - width / 2
                          : progressBar.width
          y: isHorizontal ? progressBar.height
                          : (1 - root.model.normalizedAlarmHigh) * progressBar.height - height / 2
          radius: 1
          width: isHorizontal ? 3 : 12
          height: isHorizontal ? 12 : 3
          visible: root.model.alarmsDefined
                   && root.model.alarmHigh > root.model.minValue
                   && root.model.alarmHigh < root.model.maxValue
          color: Cpp_ThemeManager.colors["alarm"]
        }
      }

    }

    //
    // Range/scale + current value display
    //
    VisualRange {
      id: range

      Layout.row: 1
      Layout.column: 0
      value: model.value
      units: model.units
      rangeVisible: false
      maxValue: model.maxValue
      minValue: model.minValue
      alarm: model.alarmTriggered
      Layout.alignment: Qt.AlignHCenter
      Layout.minimumWidth: implicitWidth
      maximumWidth: Math.min(root.width * 0.85, 320)
      Layout.preferredHeight: visible ? implicitHeight : 0
      visible: model.showValueDisplay && root.height >= 110
    }

    Item {
      implicitHeight: 4
    }
  }
}
