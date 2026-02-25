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
      Layout.preferredWidth: !isHorizontal ? Math.min(root.width * 0.6, 300) : 30
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

      ProgressBar {
        id: progressBar
        from: model.minValue
        to: model.maxValue
        value: root.value

        anchors.centerIn: parent
        width: isHorizontal ? Math.max(100, parent.width - labelMetrics.width) : Math.min(80, parent.width * 0.5)
        height: isHorizontal ? Math.min(50, parent.height * 0.7) : Math.max(100, parent.height - labelMetrics.height * 2)

        background: Rectangle {
          implicitWidth: 200
          implicitHeight: 30
          radius: 3
          border.width: 2
          border.color: Cpp_ThemeManager.colors["widget_border"]

          gradient: Gradient {
            orientation: isHorizontal ? Gradient.Horizontal : Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.05) }
            GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_base"] }
            GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.05) }
          }

          Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: parent.radius - 1
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(0, 0, 0, 0.1)
          }
        }

        contentItem: Item {
          implicitWidth: 200
          implicitHeight: 30
          clip: true

          Rectangle {
            x: 2
            y: isHorizontal ? 2 : (1 - progressBar.visualPosition) * (parent.height - 4) + 2
            width: isHorizontal ? Math.max(0, progressBar.visualPosition * (parent.width - 4)) : parent.width - 4
            height: isHorizontal ? parent.height - 4 : Math.max(0, progressBar.visualPosition * (parent.height - 4))
            radius: 1

            gradient: Gradient {
              orientation: isHorizontal ? Gradient.Horizontal : Gradient.Vertical
              GradientStop { position: 0.0; color: Qt.lighter(fillColor, 1.2) }
              GradientStop { position: 0.5; color: fillColor }
              GradientStop { position: 1.0; color: Qt.darker(fillColor, 1.1) }
            }

            Rectangle {
              anchors.left: parent.left
              anchors.top: parent.top
              anchors.bottom: parent.bottom
              width: 1
              color: Qt.rgba(1, 1, 1, 0.3)
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
              font.family: Cpp_Misc_CommonFonts.widgetFontFamily
              color: Cpp_ThemeManager.colors["widget_text"]
            }
          }
        }

        Rectangle {
          visible: root.model.alarmsDefined
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
          width: isHorizontal ? 3 : 12
          height: isHorizontal ? 12 : 3
          color: Cpp_ThemeManager.colors["alarm"]
          radius: 1
        }

        Rectangle {
          visible: root.model.alarmsDefined
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
          width: isHorizontal ? 3 : 12
          height: isHorizontal ? 12 : 3
          color: Cpp_ThemeManager.colors["alarm"]
          radius: 1
        }
      }

      //
      // Mouse area for cursor tooltip
      //
      MouseArea {
        id: cursorTracker
        anchors.fill: progressBar
        hoverEnabled: true
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
          width: isHorizontal ? 2 : cursorTracker.width
          height: isHorizontal ? cursorTracker.height : 2
          radius: 1
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          antialiasing: true
          x: isHorizontal ? cursorTracker.mouseX - 1 : 0
          y: isHorizontal ? 0 : cursorTracker.mouseY - 1
        }

        //
        // Cursor crosshair (vertical arms)
        //
        Rectangle {
          width: 1
          height: 12
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: cursorTracker.mouseX - width / 2
          y: cursorTracker.mouseY - height - 4
        }

        Rectangle {
          width: 1
          height: 12
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: cursorTracker.mouseX - width / 2
          y: cursorTracker.mouseY + 4
        }

        //
        // Cursor crosshair (horizontal arms)
        //
        Rectangle {
          width: 12
          height: 1
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: cursorTracker.mouseX - width - 4
          y: cursorTracker.mouseY - height / 2
        }

        Rectangle {
          width: 12
          height: 1
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: cursorTracker.mouseX + 4
          y: cursorTracker.mouseY - height / 2
        }

        //
        // Cursor value label (tooltip style)
        //
        Rectangle {
          visible: cursorTracker.containsMouse
          x: Math.min(cursorTracker.mouseX + 16, cursorTracker.width - width - 4)
          y: Math.max(4, Math.min(cursorTracker.mouseY + 16, cursorTracker.height - height - 4))
          width: tooltipLabel.width + 8
          height: tooltipLabel.height + 4
          color: Cpp_ThemeManager.colors["tooltip_base"]
          radius: 3
          border.width: 1
          border.color: Cpp_ThemeManager.colors["tooltip_text"]

          Label {
            id: tooltipLabel
            anchors.centerIn: parent
            text: formatValue(cursorTracker.cursorValue) + " " + model.units
            color: Cpp_ThemeManager.colors["tooltip_text"]
            font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.7))
            elide: Text.ElideRight
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
