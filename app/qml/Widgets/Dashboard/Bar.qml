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
import "ValueFormat.js" as ValueFormat

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property BarModel model
  required property string widgetId

  //
  // Spring follower emulates a real spring-driven movement: it tracks moving targets
  // continuously instead of restarting a fixed-duration animation on every sample.
  //
  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {
    SpringAnimation {
      spring: 4.5
      damping: 0.4
      epsilon: 0.001
    }
  }

  //
  // Helper properties
  //
  readonly property bool isHorizontal: root.width > 1.5 * root.height
  readonly property bool showLabels: isHorizontal ? root.width >= 200 : root.height >= 160
  readonly property real fontSize: Math.max(10, Math.min(28, Math.min(root.width, root.height) / 22))
                                   * Cpp_Misc_CommonFonts.widgetFontScale
  readonly property real digitalFontSize: Math.max(11, Math.min(36, Math.min(root.width, root.height) / 16))
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
      return perTick > labelMetrics.width + 2
    }
    const perTick = (root.height - 16) / (tickCount - 1)
    return perTick > labelMetrics.height + 2
  }

  //
  // Odd minor-tick count adapted to the space per major gap, so a half tick
  // always lands on the midpoint between two majors.
  //
  readonly property int subTicksPerMajor: {
    if (tickCount < 2)
      return 0

    const span = progressBar.innerLen / (tickCount - 1)
    const fit = Math.floor(span / 6) - 1
    const odd = fit % 2 === 0 ? fit - 1 : fit
    return Math.max(1, Math.min(7, odd))
  }

  //
  // Shared instrument-widget formatters (ValueFormat.js), bound to this model
  //
  function formatValue(val) { return ValueFormat.formatValue(model, val) }
  function formatTickValue(val) { return ValueFormat.formatTickValue(model, val) }
  function getPaddedText(val) { return ValueFormat.getPaddedText(model, val) }
  function getPaddedFormattedText(val) { return ValueFormat.getPaddedFormattedText(model, val) }

  //
  // Measures the widest endpoint label as actually displayed (trimmed). Only
  // min/max are sampled: looping the ticks would cycle through tickCount.
  //
  TextMetrics {
    id: labelMetrics

    font.pixelSize: fontSize
    font.family: Cpp_Misc_CommonFonts.widgetFontFamily
    text: {
      const a = formatTickValue(model.minValue)
      const b = formatTickValue(model.maxValue)
      return (a.length >= b.length) ? a : b
    }
  }

  //
  // SwipeView: page 0 = analog bar, page 1 = digital readout.
  // Active page is persisted per-widget via Cpp_JSON_ProjectModel.
  //
  SwipeView {
    id: swipeView

    clip: true
    interactive: true
    anchors.fill: parent
    anchors.bottomMargin: pageIndicator.height + 4

    //
    // PAGE 0: Analog bar
    //
    Item {
      id: analogPage

      clip: true
      visible: opacity > 0
      opacity: SwipeView.isCurrentItem ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 150 } }

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
            shadowBlur: 0.55
            shadowEnabled: true
            shadowOpacity: 0.13
            shadowColor: "#000000"
            shadowVerticalOffset: 1
            source: progressBarShadowSrc
            anchors.fill: progressBarShadowSrc
            visible: Cpp_Misc_GraphicsBackend.effectsEnabled
            enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
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

            readonly property real wellInset: 4
            readonly property real fillFrac: root.normalizedValue
            readonly property real innerLen: (isHorizontal ? width : height) - wellInset * 2
            readonly property real majorTickLen: Math.max(6, Math.min(26, innerBreadth * 0.32))
            readonly property real innerBreadth: (isHorizontal ? height : width) - wellInset * 2

            //
            // Position along the well's inner length for a normalised scale fraction.
            //
            function posFor(frac) {
              return isHorizontal ? wellInset + frac * innerLen
                                  : wellInset + (1 - frac) * innerLen
            }

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
              // Coloured fill that grows from low end of the range
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
                  GradientStop { position: 0.0; color: Qt.lighter(root.color, 1.20) }
                  GradientStop { position: 0.5; color: root.color }
                  GradientStop { position: 1.0; color: Qt.darker(root.color, 1.10) }
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
            // Alarm-band zone strips on the glass, along the reading edge (like the
            // coloured temperature zones printed on a thermometer tube).
            //
            Repeater {
              model: root.model.alarmBands
              delegate: Rectangle {
                required property var modelData
                opacity: 0.65
                antialiasing: true
                readonly property real bandLen: Math.max(0, (modelData.fracMax - modelData.fracMin) * progressBar.innerLen)
                readonly property real bandThickness: Math.max(4, Math.min(7, progressBar.innerBreadth * 0.14))
                color: modelData.customColor && modelData.customColor.length > 0
                       ? modelData.customColor
                       : Cpp_ThemeManager.alarmColorForSeverity(modelData.severity)

                x: isHorizontal
                   ? progressBar.posFor(modelData.fracMin)
                   : progressBar.width - progressBar.wellInset - bandThickness
                y: isHorizontal
                   ? progressBar.height - progressBar.wellInset - bandThickness
                   : progressBar.posFor(modelData.fracMax)
                width: isHorizontal ? bandLen : bandThickness
                height: isHorizontal ? bandThickness : bandLen
              }
            }

            //
            // Major graduation marks on the glass + labels outside the reading edge
            //
            Repeater {
              model: tickCount
              delegate: Item {
                required property int index
                readonly property real frac: index / (tickCount - 1)
                readonly property real tickValue: root.model.minValue + frac * (root.model.maxValue - root.model.minValue)

                Rectangle {
                  opacity: 0.9
                  antialiasing: true
                  width: isHorizontal ? 1.5 : progressBar.majorTickLen
                  height: isHorizontal ? progressBar.majorTickLen : 1.5
                  color: Cpp_ThemeManager.colors["widget_border"]
                  x: isHorizontal
                     ? progressBar.posFor(parent.frac) - width / 2
                     : progressBar.width - progressBar.wellInset - width
                  y: isHorizontal
                     ? progressBar.height - progressBar.wellInset - height
                     : progressBar.posFor(parent.frac) - height / 2
                }

                Text {
                  visible: showLabels && root.labelsFit
                  font.pixelSize: fontSize
                  text: formatTickValue(parent.tickValue)
                  color: Cpp_ThemeManager.colors["widget_text"]
                  font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                  x: isHorizontal
                     ? progressBar.posFor(parent.frac) - width / 2
                     : progressBar.width + 6
                  y: isHorizontal
                     ? progressBar.height + 6
                     : progressBar.posFor(parent.frac) - height / 2
                }
              }
            }

            //
            // Minor graduation marks between majors (adaptive odd count, emphasized half tick)
            //
            Repeater {
              model: Math.max(0, tickCount - 1) * subTicksPerMajor
              delegate: Rectangle {
                required property int index
                readonly property int subIndex: (index % subTicksPerMajor) + 1
                readonly property int majorIndex: Math.floor(index / subTicksPerMajor)
                readonly property bool halfTick: 2 * subIndex === subTicksPerMajor + 1
                readonly property real tickLen: progressBar.majorTickLen * (halfTick ? 0.66 : 0.42)
                readonly property real frac: (majorIndex + subIndex / (subTicksPerMajor + 1)) / (tickCount - 1)

                opacity: halfTick ? 0.8 : 0.65
                antialiasing: true
                width: isHorizontal ? (halfTick ? 1.5 : 1) : tickLen
                height: isHorizontal ? tickLen : (halfTick ? 1.5 : 1)
                color: Cpp_ThemeManager.colors["widget_border"]
                x: isHorizontal
                   ? progressBar.posFor(frac) - width / 2
                   : progressBar.width - progressBar.wellInset - width
                y: isHorizontal
                   ? progressBar.height - progressBar.wellInset - height
                   : progressBar.posFor(frac) - height / 2
              }
            }

            //
            // Bezel inner shadow: edge falloff just inside the well so the bezel reads
            // as overhanging the recessed face (rectangular analog of the Gauge ring).
            //
            Item {
              id: wellShadow

              anchors.fill: innerWell

              readonly property color shadowColor: Qt.rgba(0, 0, 0, 0.15)
              readonly property real falloff: Math.min(Math.max(4, progressBar.innerBreadth * 0.08),
                                                       progressBar.innerBreadth * 0.20)

              Rectangle {
                height: wellShadow.falloff
                anchors {
                  top: parent.top
                  left: parent.left
                  right: parent.right
                }
                gradient: Gradient {
                  GradientStop { position: 0.0; color: wellShadow.shadowColor }
                  GradientStop { position: 1.0; color: "transparent" }
                }
              }

              Rectangle {
                height: wellShadow.falloff
                anchors {
                  left: parent.left
                  right: parent.right
                  bottom: parent.bottom
                }
                gradient: Gradient {
                  GradientStop { position: 0.0; color: "transparent" }
                  GradientStop { position: 1.0; color: wellShadow.shadowColor }
                }
              }

              Rectangle {
                width: wellShadow.falloff
                anchors {
                  top: parent.top
                  left: parent.left
                  bottom: parent.bottom
                }
                gradient: Gradient {
                  orientation: Gradient.Horizontal
                  GradientStop { position: 0.0; color: wellShadow.shadowColor }
                  GradientStop { position: 1.0; color: "transparent" }
                }
              }

              Rectangle {
                width: wellShadow.falloff
                anchors {
                  top: parent.top
                  right: parent.right
                  bottom: parent.bottom
                }
                gradient: Gradient {
                  orientation: Gradient.Horizontal
                  GradientStop { position: 0.0; color: "transparent" }
                  GradientStop { position: 1.0; color: wellShadow.shadowColor }
                }
              }
            }

            //
            // Inner-well ring redrawn opaque above the juice, bands and marks so the glass
            // edge stays crisp and bands never show through the border.
            //
            Rectangle {
              radius: 3
              border.width: 1
              antialiasing: true
              color: "transparent"
              anchors.fill: innerWell
              border.color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.10)
            }
          }

        }

        //
        // Label row: channel name + flashing value box (matches Gauge and Meter)
        //
        Item {
          id: labelRow

          Layout.row: 1
          Layout.column: 0
          Layout.fillWidth: true
          visible: root.height >= 110
          Layout.preferredHeight: visible ? Math.round(digitalFontSize * 2.0 + 12) : 0

          //
          // Hide the title and let the value box span the full row when either the
          // title would elide or the value text would overflow its half-row slot.
          //
          readonly property real rowInnerWidth: width - 12
          readonly property real halfWidth: (rowInnerWidth - 4) / 2
          readonly property bool titleFits: titleTextMetrics.width <= halfWidth - 8
          readonly property bool valueFits: valueBoxMetrics.width + 18 <= halfWidth
          readonly property bool showTitle: root.model.title.length > 0
                                            && titleFits && valueFits

          TextMetrics {
            id: titleTextMetrics

            font.bold: true
            text: root.model.title
            font.pixelSize: digitalFontSize
            font.family: Cpp_Misc_CommonFonts.widgetFontFamily
          }

          RowLayout {
            spacing: 4
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6

            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true
              visible: labelRow.showTitle

              Text {
                id: titleText

                font.bold: true
                elide: Text.ElideRight
                anchors.fill: parent
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                text: root.model.title
                font.pixelSize: digitalFontSize
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: Cpp_ThemeManager.colors["widget_text"]
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
              }
            }

            //
            // Value readout
            //
            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true

              Rectangle {
                id: valueBox

                radius: 3
                border.width: 1
                antialiasing: true
                anchors.centerIn: parent
                height: valueText.implicitHeight + 8
                width: Math.min(parent.width, valueBoxMetrics.width + 18)
                border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.35)
                color: model.alarmTriggered
                       ? (valueBox.alarmFlashOn
                          ? Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity)
                          : Cpp_ThemeManager.colors["console_base"])
                       : Cpp_ThemeManager.colors["console_base"]

                Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }

                property bool alarmFlashOn: false

                TextMetrics {
                  id: valueBoxMetrics

                  font.bold: true
                  font.pixelSize: digitalFontSize * 1.05
                  font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                  text: {
                    const a = root.formatValue(root.model.minValue)
                    const b = root.formatValue(root.model.maxValue)
                    const longer = a.length >= b.length ? a : b
                    return longer + (root.model.units.length > 0 ? " " + root.model.units : "")
                  }
                }

                SequentialAnimation {
                  loops: Animation.Infinite
                  running: model.alarmTriggered
                  PropertyAction { target: valueBox; property: "alarmFlashOn"; value: true }
                  PauseAnimation { duration: 450 }
                  PropertyAction { target: valueBox; property: "alarmFlashOn"; value: false }
                  PauseAnimation { duration: 450 }
                }

                Text {
                  id: valueText

                  font.bold: true
                  anchors.centerIn: parent
                  font.pixelSize: digitalFontSize * 1.05
                  font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                  color: model.alarmTriggered
                         ? (valueBox.alarmFlashOn
                            ? "#ffffff"
                            : Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity))
                         : Cpp_ThemeManager.colors["console_text"]
                  text: root.getPaddedFormattedText(model.value)

                  Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
                }
              }
            }
          }
        }

        Item {
          implicitHeight: 4
        }
      }
    }

    //
    // PAGE 1: Big digital readout
    //
    Item {
      id: digitalPage

      clip: true
      visible: opacity > 0
      opacity: SwipeView.isCurrentItem ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 150 } }

      TextMetrics {
        id: bigValueMetrics

        font.bold: true
        font.pixelSize: 100
        font.family: Cpp_Misc_CommonFonts.monoFont.family
        text: {
          const a = root.formatValue(root.model.minValue)
          const b = root.formatValue(root.model.maxValue)
          const longer = a.length >= b.length ? a : b
          return longer + (root.model.units.length > 0 ? " " + root.model.units : "")
        }
      }

      readonly property real availableW: Math.max(0, width  - 32)
      readonly property real availableH: Math.max(0, height - 64)
      readonly property real bigValueFontPx: {
        const metricsW = Math.max(8, bigValueMetrics.width)
        const widthFit  = (digitalPage.availableW * 0.85 / metricsW) * bigValueMetrics.font.pixelSize
        const heightFit = digitalPage.availableH * 0.50
        return Math.max(20, Math.min(160, widthFit, heightFit))
      }

      Rectangle {
        id: digitalBox

        radius: 4
        antialiasing: true
        anchors.centerIn: parent
        border.width: 1
        border.color: root.model.alarmTriggered
                      ? Qt.darker(Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity), 1.20)
                      : Qt.darker(root.color, 1.30)
        width: Math.min(parent.width - 16, digitalColumn.implicitWidth + 32)
        height: Math.min(parent.height - 16, digitalColumn.implicitHeight + 24)
        color: digitalBox.targetColor

        Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }

        property bool alarmFlashOn: false
        readonly property color targetColor: root.model.alarmTriggered && digitalBox.alarmFlashOn
                                              ? Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity)
                                              : Cpp_ThemeManager.colors["console_base"]
        SequentialAnimation {
          loops: Animation.Infinite
          running: root.model.alarmTriggered
          PropertyAction { target: digitalBox; property: "alarmFlashOn"; value: true }
          PauseAnimation { duration: 450 }
          PropertyAction { target: digitalBox; property: "alarmFlashOn"; value: false }
          PauseAnimation { duration: 450 }
        }

        Column {
          id: digitalColumn

          spacing: 6
          anchors.centerIn: parent

          Text {
            id: bigValueText

            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, digitalPage.width - 32)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            text: getPaddedText(root.model.value)
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.bold: true
            font.pixelSize: digitalPage.bigValueFontPx
            color: root.model.alarmTriggered
                   ? (digitalBox.alarmFlashOn ? "#ffffff" : Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity))
                   : root.color
            Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
          }

          Text {
            opacity: 0.80
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, digitalPage.width - 32)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            text: root.model.title
            visible: root.model.title.length > 0
            color: root.model.alarmTriggered
                   ? (digitalBox.alarmFlashOn ? "#ffffff" : Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity))
                   : root.color
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.pixelSize: bigValueText.font.pixelSize * 0.30
            Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
          }
        }
      }
    }
  }

  //
  // Page indicator
  //
  PageIndicator {
    id: pageIndicator

    interactive: true
    count: swipeView.count
    anchors.bottomMargin: 4
    anchors.bottom: parent.bottom
    currentIndex: swipeView.currentIndex
    anchors.horizontalCenter: parent.horizontalCenter

    delegate: Rectangle {
      required property int index
      implicitWidth: 8
      implicitHeight: 8
      radius: width / 2
      antialiasing: true
      color: Cpp_ThemeManager.colors["widget_text"]
      opacity: index === pageIndicator.currentIndex ? 0.95 : 0.40
      Behavior on opacity { NumberAnimation { duration: 120 } }
    }

    onCurrentIndexChanged: {
      if (swipeView.currentIndex !== currentIndex)
        swipeView.currentIndex = currentIndex
    }
  }

  //
  // Suppresses the page auto-save while restore assigns the persisted index
  //
  property bool restoringPage: false

  //
  // Restore per-widget page from project settings, then persist on change.
  //
  Component.onCompleted: {
    root.restoringPage = true
    const s = Cpp_JSON_ProjectModel.widgetSettings(root.widgetId)
    if (s["page"] !== undefined)
      swipeView.currentIndex = parseInt(s["page"])

    root.restoringPage = false
  }
  Connections {
    target: swipeView
    function onCurrentIndexChanged() {
      if (root.restoringPage)
        return

      Cpp_JSON_ProjectModel.saveWidgetSetting(
            root.widgetId, "page", swipeView.currentIndex)
    }
  }
}
