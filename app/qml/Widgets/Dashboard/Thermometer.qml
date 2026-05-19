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
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root

  required property color color
  required property var windowRoot
  required property ThermometerModel model
  required property string widgetId

  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {NumberAnimation{duration: 140; easing.type: Easing.OutCubic}}

  readonly property bool showLabels: root.width >= 110 && root.height >= 120
  readonly property color fillColor: model.alarmTriggered ? Cpp_ThemeManager.colors["alarm"] : root.color
  readonly property real fontSize: Math.max(8, Math.min(11, Math.min(root.width, root.height) / 30))
                                   * Cpp_Misc_CommonFonts.widgetFontScale

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

  GridLayout {
    id: tubeLayout

    rowSpacing: 4
    columnSpacing: 8
    anchors.margins: 8
    anchors.fill: parent

    rows: tubeLayout.wide ? 1 : 2
    columns: tubeLayout.wide ? 2 : 1
    readonly property bool wide: root.width > root.height * 1.4

    Item {
      id: tubeArea

      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter
      Layout.row: tubeLayout.wide ? 0 : 0
      Layout.column: tubeLayout.wide ? 1 : 0

      //
      // Geometry -- tube on the left, tick scale on its right
      //
      readonly property real tickStem: showLabels ? 6 : 4
      readonly property real tickGap: showLabels ? 4 : 2
      readonly property real labelWidth: showLabels ? labelMetrics.width + 2 : 0
      readonly property real tubeW: Math.max(14, Math.min(80, Math.min(tubeArea.width * 0.30, tubeArea.height * 0.20)))
      readonly property real groupW: tubeArea.tubeW
                                     + (showLabels ? (tubeArea.tickGap + tubeArea.tickStem + 4 + tubeArea.labelWidth) : 0)
      readonly property real tubeX: Math.max(4, (tubeArea.width - tubeArea.groupW) / 2)
      readonly property real tubeY: 4
      readonly property real tubeH: Math.max(40, tubeArea.height - tubeArea.tubeY * 2)
      readonly property int tickCount: {
        if (root.model.displayTickCount > 0) return root.model.displayTickCount
        return Math.max(3, Math.min(9, Math.floor(tubeArea.tubeH / Math.max(22, labelMetrics.height * 2.4))))
      }
      readonly property bool labelsFit: {
        if (tubeArea.tickCount < 2) return true
        const perTick = tubeArea.tubeH / (tubeArea.tickCount - 1)
        return perTick > labelMetrics.height + 2
      }

      //
      // Outer chrome ring -- silver gradient rendered via MultiEffect with shadow
      //
      readonly property real chromeW: 3

      Rectangle {
        id: tubeChrome

        visible: false
        antialiasing: true
        x: tubeArea.tubeX - tubeArea.chromeW
        y: tubeArea.tubeY - tubeArea.chromeW
        width: tubeArea.tubeW + tubeArea.chromeW * 2
        height: tubeArea.tubeH + tubeArea.chromeW * 2
        radius: width / 2
        border.width: 1
        border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.40)
        gradient: Gradient {
          GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.30) }
          GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_base"] }
          GradientStop { position: 1.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.16) }
        }
      }
      MultiEffect {
        shadowBlur: 0.50
        source: tubeChrome
        shadowEnabled: true
        shadowOpacity: 0.28
        shadowColor: "#000000"
        shadowVerticalOffset: 2
        anchors.fill: tubeChrome
      }

      //
      // Glass tube -- cream interior with thin dark wall, sits inside the chrome ring
      //
      Rectangle {
        id: tube

        border.width: 1
        x: tubeArea.tubeX
        y: tubeArea.tubeY
        radius: width / 2
        antialiasing: true
        width: tubeArea.tubeW
        height: tubeArea.tubeH
        border.color: Qt.rgba(0, 0, 0, 0.22)
        gradient: Gradient {
          orientation: Gradient.Horizontal
          GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.24) }
          GradientStop { position: 0.5; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.16) }
          GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.08) }
        }
      }

      //
      // Reflective highlight along the tube's left side
      //
      Rectangle {
        opacity: 0.55
        radius: width / 2
        antialiasing: true
        width: tube.width * 0.18
        x: tube.x + tube.width * 0.16
        y: tube.y + tube.width * 0.55
        height: tube.height - tube.width * 1.10
        gradient: Gradient {
          GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.85) }
          GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0) }
        }
      }

      //
      // Mercury column -- capsule that grows from the bottom of the tube
      //
      Rectangle {
        id: mercuryFill

        antialiasing: true
        x: tube.x + tube.border.width
        width: tube.width - tube.border.width * 2
        height: Math.max(0, mercuryFill.frac * mercuryFill.innerH)
        y: tube.y + tube.border.width + (1 - mercuryFill.frac) * mercuryFill.innerH

        readonly property real corner: width / 2
        readonly property real frac: root.normalizedValue
        readonly property real innerH: tube.height - tube.border.width * 2

        bottomLeftRadius: mercuryFill.corner
        bottomRightRadius: mercuryFill.corner
        topLeftRadius: mercuryFill.frac > 0.93 ? mercuryFill.corner : 0
        topRightRadius: mercuryFill.frac > 0.93 ? mercuryFill.corner : 0

        gradient: Gradient {
          orientation: Gradient.Horizontal
          GradientStop { position: 0.0; color: Qt.lighter(fillColor, 1.30) }
          GradientStop { position: 0.5; color: fillColor }
          GradientStop { position: 1.0; color: Qt.darker(fillColor, 1.15) }
        }
      }

      //
      // Major ticks + labels on the right side
      //
      Repeater {
        model: tubeArea.tickCount
        delegate: Item {
          required property int index
          readonly property real frac: index / (tubeArea.tickCount - 1)
          readonly property real tickValue: root.model.minValue + frac * (root.model.maxValue - root.model.minValue)
          readonly property real yPos: tube.y + tube.border.width + (1 - frac) * (tube.height - tube.border.width * 2)

          Rectangle {
            width: tubeArea.tickStem
            height: 1.5
            radius: 0.75
            color: Cpp_ThemeManager.colors["widget_border"]
            x: tube.x + tube.width + tubeArea.tickGap
            y: parent.yPos - height / 2
          }

          Text {
            visible: showLabels && tubeArea.labelsFit
            font.pixelSize: fontSize
            text: formatValue(parent.tickValue)
            color: Cpp_ThemeManager.colors["widget_text"]
            font.family: Cpp_Misc_CommonFonts.widgetFontFamily
            verticalAlignment: Text.AlignVCenter
            x: tube.x + tube.width + tubeArea.tickGap + tubeArea.tickStem + 3
            y: parent.yPos - height / 2
          }
        }
      }

      //
      // Minor ticks between majors
      //
      Repeater {
        model: (tubeArea.tickCount - 1) * 4
        delegate: Rectangle {
          required property int index
          readonly property int subIndex: (index % 4) + 1
          readonly property int majorIndex: Math.floor(index / 4)
          readonly property real frac: (majorIndex + subIndex / 5) / (tubeArea.tickCount - 1)
          readonly property real yPos: tube.y + tube.border.width + (1 - frac) * (tube.height - tube.border.width * 2)

          width: tubeArea.tickStem * 0.55
          height: 1
          color: Cpp_ThemeManager.colors["widget_border"]
          opacity: 0.65
          x: tube.x + tube.width + tubeArea.tickGap
          y: yPos - height / 2
        }
      }

      //
      // Alarm low / high markers (left side of tube)
      //
      Repeater {
        model: [
          { frac: root.model.normalizedAlarmLow,  active: root.model.alarmsDefined && root.model.alarmLow  > root.model.minValue && root.model.alarmLow  < root.model.maxValue },
          { frac: root.model.normalizedAlarmHigh, active: root.model.alarmsDefined && root.model.alarmHigh > root.model.minValue && root.model.alarmHigh < root.model.maxValue }
        ]
        delegate: Rectangle {
          required property var modelData
          visible: modelData.active
          readonly property real yPos: tube.y + tube.border.width + (1 - modelData.frac) * (tube.height - tube.border.width * 2)

          width: 8
          height: 2.5
          radius: 1
          color: Cpp_ThemeManager.colors["alarm"]
          x: tube.x - width - 2
          y: yPos - height / 2
        }
      }

      //
      // Cursor tooltip over the tube
      //
      MouseArea {
        id: cursorTracker

        x: tube.x
        y: tube.y
        width: tube.width
        hoverEnabled: true
        height: tube.height
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: true

        property real cursorValue: model.minValue

        onPositionChanged: (mouse) => {
          const f = 1.0 - (mouse.y / height)
          cursorTracker.cursorValue = model.minValue + f * (model.maxValue - model.minValue)
        }

        Rectangle {
          radius: 1
          antialiasing: true
          width: cursorTracker.width
          height: 1.4
          y: cursorTracker.mouseY - height / 2
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          color: Cpp_ThemeManager.colors["polar_indicator"]
        }

        Rectangle {
          radius: 3
          border.width: 1
          width: tooltipLabel.width + 8
          height: tooltipLabel.height + 4
          visible: cursorTracker.containsMouse
          color: Cpp_ThemeManager.colors["tooltip_base"]
          border.color: Cpp_ThemeManager.colors["tooltip_text"]
          x: Math.min(cursorTracker.mouseX + 16, cursorTracker.width - width - 4)
          y: Math.max(4, Math.min(cursorTracker.mouseY + 16, cursorTracker.height - height - 4))

          Label {
            id: tooltipLabel

            elide: Text.ElideRight
            anchors.centerIn: parent
            color: Cpp_ThemeManager.colors["tooltip_text"]
            font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.7))
            text: formatValue(cursorTracker.cursorValue) + (model.units.length > 0 ? " " + model.units : "")
          }
        }
      }
    }

    VisualRange {
      id: range

      visible: model.showValueDisplay
               && (tubeLayout.wide ? root.width >= 220 : root.height >= 110)
      value: model.value
      units: model.units
      rangeVisible: false
      maxValue: model.maxValue
      minValue: model.minValue
      alarm: model.alarmTriggered
      Layout.row: tubeLayout.wide ? 0 : 1
      Layout.column: tubeLayout.wide ? 0 : 0
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
      Layout.minimumWidth: implicitWidth
      Layout.preferredWidth: tubeLayout.wide ? Math.min(root.width * 0.32, 280) : implicitWidth
      Layout.preferredHeight: visible ? implicitHeight : 0
      Layout.leftMargin: tubeLayout.wide ? 12 : 0
      maximumWidth: tubeLayout.wide ? Math.min(root.width * 0.32 - 12, 268)
                                    : Math.min(root.width * 0.85, 320)
    }
  }
}
