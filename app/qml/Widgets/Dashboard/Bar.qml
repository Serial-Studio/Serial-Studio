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
  required property string widgetId

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
                                   * Cpp_Misc_CommonFonts.widgetFontScale
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
  ColumnLayout {
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true
    }

    //
    // Bar widget
    //
    Item {
      id: barContainer

      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
      Layout.preferredWidth: !isHorizontal ? Math.min(root.width * 0.6, 300) : 30
      Layout.preferredHeight: isHorizontal ? Math.min(80, root.height * 0.5) : root.height * 0.7

      ProgressBar {
        id: progressBar

        value: root.value
        to: model.maxValue
        from: model.minValue

        anchors.centerIn: parent
        width: isHorizontal ? Math.max(100, parent.width - labelMetrics.width) : Math.min(80, parent.width * 0.5)
        height: isHorizontal ? Math.min(50, parent.height * 0.7) : Math.max(100, parent.height - labelMetrics.height * 2)

        background: Rectangle {
          radius: 3
          border.width: 2
          implicitWidth: 200
          implicitHeight: 30
          border.color: Cpp_ThemeManager.colors["widget_border"]

          gradient: Gradient {
            orientation: isHorizontal ? Gradient.Horizontal : Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.05) }
            GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_base"] }
            GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.05) }
          }

          Rectangle {
            border.width: 1
            anchors.margins: 1
            anchors.fill: parent
            color: "transparent"
            radius: parent.radius - 1
            border.color: Qt.rgba(0, 0, 0, 0.1)
          }
        }

        contentItem: Item {
          clip: true
          implicitWidth: 200
          implicitHeight: 30

          Rectangle {
            x: 2
            radius: 1
            y: isHorizontal ? 2 : (1 - progressBar.visualPosition) * (parent.height - 4) + 2
            width: isHorizontal ? Math.max(0, progressBar.visualPosition * (parent.width - 4)) : parent.width - 4
            height: isHorizontal ? parent.height - 4 : Math.max(0, progressBar.visualPosition * (parent.height - 4))

            gradient: Gradient {
              orientation: isHorizontal ? Gradient.Horizontal : Gradient.Vertical
              GradientStop { position: 0.0; color: Qt.lighter(fillColor, 1.2) }
              GradientStop { position: 0.5; color: fillColor }
              GradientStop { position: 1.0; color: Qt.darker(fillColor, 1.1) }
            }

            Rectangle {
              width: 1
              anchors.top: parent.top
              anchors.left: parent.left
              color: Qt.rgba(1, 1, 1, 0.3)
              anchors.bottom: parent.bottom
            }
          }
        }

        Repeater {
          model: tickCount
          delegate: Item {
            required property int index
            readonly property real frac: index / (tickCount - 1)
            readonly property real tickValue: root.model.minValue + frac * (root.model.maxValue - root.model.minValue)

            Rectangle {
              width: isHorizontal ? 2 : 8
              height: isHorizontal ? 8 : 2
              color: Cpp_ThemeManager.colors["widget_border"]
              x: isHorizontal ? (frac * progressBar.width - width / 2) : progressBar.width
              y: isHorizontal ? progressBar.height : ((1 - frac) * progressBar.height - height / 2)
            }

            Text {
              font.pixelSize: fontSize
              text: formatValue(parent.tickValue)
              color: Cpp_ThemeManager.colors["widget_text"]
              font.family: Cpp_Misc_CommonFonts.widgetFontFamily
              x: isHorizontal ? (frac * progressBar.width - width / 2) : progressBar.width + 10
              y: isHorizontal ? progressBar.height + 10 : ((1 - frac) * progressBar.height - height / 2)
            }
          }
        }

        Rectangle {
          x: {
            if (isHorizontal) {
              const alarmFrac = (root.model.alarmLow - root.model.minValue) / (root.model.maxValue - root.model.minValue)
              return alarmFrac * progressBar.width - width / 2
            } else {
              return progressBar.width
            }
          }
          y: {
            if (isHorizontal) {
              return progressBar.height
            } else {
              const alarmFrac = (root.model.alarmLow - root.model.minValue) / (root.model.maxValue - root.model.minValue)
              return (1 - alarmFrac) * progressBar.height - height / 2
            }
          }
          radius: 1
          width: isHorizontal ? 3 : 12
          height: isHorizontal ? 12 : 3
          visible: root.model.alarmsDefined
          color: Cpp_ThemeManager.colors["alarm"]
        }

        Rectangle {
          x: {
            if (isHorizontal) {
              const alarmFrac = (root.model.alarmHigh - root.model.minValue) / (root.model.maxValue - root.model.minValue)
              return alarmFrac * progressBar.width - width / 2
            } else {
              return progressBar.width
            }
          }
          y: {
            if (isHorizontal) {
              return progressBar.height
            } else {
              const alarmFrac = (root.model.alarmHigh - root.model.minValue) / (root.model.maxValue - root.model.minValue)
              return (1 - alarmFrac) * progressBar.height - height / 2
            }
          }
          radius: 1
          width: isHorizontal ? 3 : 12
          height: isHorizontal ? 12 : 3
          visible: root.model.alarmsDefined
          color: Cpp_ThemeManager.colors["alarm"]
        }
      }

      //
      // Mouse area for cursor tooltip
      //
      MouseArea {
        id: cursorTracker

        hoverEnabled: true
        anchors.fill: progressBar
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: true

        property real cursorValue: model.minValue

        onPositionChanged: (mouse) => {
          if (isHorizontal) {
            var fraction = mouse.x / width
            cursorValue = model.minValue + fraction * (model.maxValue - model.minValue)
          } else {
            var fraction = 1.0 - (mouse.y / height)
            cursorValue = model.minValue + fraction * (model.maxValue - model.minValue)
          }
        }

        //
        // Measurement line (full height when horizontal, full width when vertical)
        //
        Rectangle {
          radius: 1
          antialiasing: true
          width: isHorizontal ? 2 : cursorTracker.width
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: isHorizontal ? cursorTracker.mouseX - 1 : 0
          y: isHorizontal ? 0 : cursorTracker.mouseY - 1
          height: isHorizontal ? cursorTracker.height : 2
          color: Cpp_ThemeManager.colors["polar_indicator"]
        }

        //
        // Cursor crosshair (vertical arms)
        //
        Rectangle {
          width: 1
          height: 12
          x: cursorTracker.mouseX - width / 2
          y: cursorTracker.mouseY - height - 4
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          color: Cpp_ThemeManager.colors["polar_indicator"]
        }

        Rectangle {
          width: 1
          height: 12
          y: cursorTracker.mouseY + 4
          x: cursorTracker.mouseX - width / 2
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          color: Cpp_ThemeManager.colors["polar_indicator"]
        }

        //
        // Cursor crosshair (horizontal arms)
        //
        Rectangle {
          width: 12
          height: 1
          x: cursorTracker.mouseX - width - 4
          y: cursorTracker.mouseY - height / 2
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          color: Cpp_ThemeManager.colors["polar_indicator"]
        }

        Rectangle {
          width: 12
          height: 1
          x: cursorTracker.mouseX + 4
          y: cursorTracker.mouseY - height / 2
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          color: Cpp_ThemeManager.colors["polar_indicator"]
        }

        //
        // Cursor value label (tooltip style)
        //
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
            text: formatValue(cursorTracker.cursorValue) + " " + model.units
          }
        }
      }
    }

    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true
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
      Layout.alignment: Qt.AlignHCenter
      Layout.minimumWidth: implicitWidth
      maximumWidth: Math.min(root.width * 0.8, 200)
    }

    Item {
      implicitHeight: 4
    }
  }
}
