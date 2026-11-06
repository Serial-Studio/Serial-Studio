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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Effects
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
  required property LEDPanelModel model

  //
  // Panel-lamp look, selected by the theme (spec 0113); every other theme keeps the disc
  //
  readonly property bool lampLook: Cpp_ThemeManager.widgetLooks["led"] === "lamp"

  //
  // A lamp row is taller than a disc row: the bezel needs the room
  //
  readonly property int cellHeight: root.lampLook ? 42 : 32

  //
  // Create scrollable grid view with LED states
  //
  Flickable {
    id: ledFlick

    contentWidth: width
    anchors.fill: parent
    anchors.topMargin: 8
    anchors.leftMargin: 8
    anchors.bottomMargin: 8
    contentHeight: Math.max(grid.implicitHeight, height)

    ScrollBar.vertical: ScrollBar {
      id: scroll

      policy: ledFlick.contentHeight > ledFlick.height ? ScrollBar.AlwaysOn
                                                       : ScrollBar.AlwaysOff
    }

    GridLayout {
      id: grid

      columns: {
        const itemHeight = root.cellHeight + 4
        const availableHeight = root.height - 16
        const itemsPerColumn = Math.floor(availableHeight / itemHeight)
        return (root.model.count <= itemsPerColumn) ? 1 : 2
      }
      rowSpacing: 4
      columnSpacing: 4
      width: parent.width - 8
      anchors.centerIn: parent
      anchors.horizontalCenterOffset: -4

      Repeater {
        model: root.model.count
        delegate: Rectangle {
          id: cell

          //
          // Shrink fonts, paddings and the LED dot as the cell narrows so
          // the labels always fit inside the rectangle
          //
          readonly property real uiScale: Cpp_Misc_CommonFonts.autoScale(width, 180)

          border.width: 1
          Layout.fillWidth: true
          radius: root.lampLook ? 4 : 0
          Layout.minimumHeight: root.cellHeight
          Layout.maximumHeight: root.cellHeight
          Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
          color: root.lampLook ? Cpp_ThemeManager.colors["base"]
                               : Cpp_ThemeManager.colors["widget_base"]
          border.color: root.lampLook ? Cpp_ThemeManager.colors["groupbox_border"]
                                      : Cpp_ThemeManager.colors["widget_border"]

          //
          // Lamp bezel under the lens; its halo is what a lit lamp shows with effects off
          //
          Loader {
            height: width
            active: root.lampLook
            width: Math.round(led.width * 1.45)
            x: layout.x + led.x + (led.width - width) / 2
            y: layout.y + led.y + (led.height - height) / 2

            sourceComponent: PanelBezel {
              halo: led.showLit
              haloColor: led.ledColor
            }
          }

          RowLayout {
            id: layout

            anchors.margins: 4
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: Math.round(12 * cell.uiScale)
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: Math.round(24 * cell.uiScale)
            anchors.rightMargin: Math.round(12 * cell.uiScale)

            Rectangle {
              id: led

              readonly property bool lit: root.model.states[index]
              readonly property color ledColor: root.model.colors[index]
              readonly property bool flashing: led.lit && root.model.blinks[index]
              readonly property bool showLit: led.lit && (!led.flashing || led.flashOn)
              readonly property color litColor: {
                if (!root.lampLook)
                  return led.ledColor

                return Qt.hsva(Math.max(0, led.ledColor.hsvHue), led.ledColor.hsvSaturation,
                               Math.max(0.96, led.ledColor.hsvValue), 1)
              }
              readonly property color offColor: {
                if (!root.lampLook)
                  return Qt.darker(led.ledColor)

                return Qt.hsva(Math.max(0, led.ledColor.hsvHue),
                               led.ledColor.hsvSaturation * 0.7,
                               Math.min(1, led.ledColor.hsvValue * 0.55 + 0.12), 1)
              }

              property bool flashOn: true

              border.width: 1
              implicitWidth: Math.round((root.lampLook ? 20 : 18) * cell.uiScale)
              implicitHeight: Math.round((root.lampLook ? 20 : 18) * cell.uiScale)
              radius: implicitWidth / 2
              Layout.alignment: Qt.AlignVCenter
              border.color: root.lampLook ? Cpp_ThemeManager.colors["window_border"]
                                          : Cpp_ThemeManager.colors["widget_border"]
              color: led.showLit ? led.litColor : led.offColor

              Behavior on color {ColorAnimation{}}

              SequentialAnimation {
                running: led.flashing
                loops: Animation.Infinite
                PropertyAction { target: led; property: "flashOn"; value: true }
                PauseAnimation { duration: 450 }
                PropertyAction { target: led; property: "flashOn"; value: false }
                PauseAnimation { duration: 450 }
              }
            }

            Label {
              id: ledTitle

              elide: Qt.ElideRight
              Layout.fillWidth: true
              text: root.model.titles[index]
              Layout.alignment: Qt.AlignVCenter
              horizontalAlignment: Label.AlignLeft
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(cell.uiScale))

              Behavior on color {ColorAnimation{}}

              HoverHandler {
                id: ledTitleHover

                enabled: ledTitle.truncated
              }
              ToolTip.delay: 600
              ToolTip.text: ledTitle.text
              ToolTip.visible: ledTitleHover.hovered && ledTitle.truncated
            }

            //
            // Status label: shrinkable (fillWidth + capped maximumWidth) so
            // narrow cells elide the text instead of overflowing the cell
            //
            Label {
              id: ledStatus

              elide: Qt.ElideRight
              Layout.fillWidth: true
              visible: text.length > 0
              text: root.model.labels[index]
              Layout.alignment: Qt.AlignVCenter
              horizontalAlignment: Label.AlignRight
              Layout.maximumWidth: Math.min(implicitWidth, layout.width * 0.45)
              color: led.lit ? led.ledColor : Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(cell.uiScale))

              Behavior on color {ColorAnimation{}}

              HoverHandler {
                id: ledStatusHover

                enabled: ledStatus.truncated
              }
              ToolTip.delay: 600
              ToolTip.text: ledStatus.text
              ToolTip.visible: ledStatusHover.hovered && ledStatus.truncated
            }

            //
            // Pop-out buttons for the other widgets showing this dataset
            //
            DatasetWidgetButtons {
              windowRoot: root.windowRoot
              widgets: root.model.widgets[index]
              Layout.alignment: Qt.AlignVCenter
              buttonSize: Math.max(22, Math.round(24 * cell.uiScale))
            }
          }

          //
          // Lamp facets and highlight over the lens, under the bloom so the glow covers them
          //
          Loader {
            width: led.width
            height: led.height
            x: layout.x + led.x
            y: layout.y + led.y
            active: root.lampLook

            sourceComponent: PanelLensFacets {
              lit: led.showLit
            }
          }

          MultiEffect {
            id: ledBloomWide

            source: led
            width: led.width
            height: led.height
            x: layout.x + led.x
            y: layout.y + led.y

            blurEnabled: true
            blur: led.showLit ? 1 : 0
            blurMax: led.showLit ? 64 : 0
            brightness: led.showLit ? 0.4 : 0
            saturation: led.showLit ? 0.2 : 0
            visible: Cpp_Misc_GraphicsBackend.effectsEnabled
            enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
          }

          MultiEffect {
            id: ledBloomTight

            source: led
            width: led.width
            height: led.height
            x: layout.x + led.x
            y: layout.y + led.y

            blurEnabled: true
            blur: led.showLit ? 0.3 : 0
            blurMax: led.showLit ? 64 : 0
            brightness: led.showLit ? 0.4 : 0
            saturation: led.showLit ? 0.2 : 0
            visible: Cpp_Misc_GraphicsBackend.effectsEnabled
            enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
          }

          //
          // Emissive bloom (spec 0089): halos captured with a margin so the blur spill
          // survives, riding the core's flash/steady tier so the lamp pulses coherently.
          //
          HdrBoost {
            target: ledBloomWide
            captureMargin: 64
            width: led.width + 128
            height: led.height + 128
            x: layout.x + led.x - 64
            y: layout.y + led.y - 64
            active: led.showLit && Cpp_Misc_GraphicsBackend.effectsEnabled
            boost: led.flashing
                   ? Cpp_Misc_GraphicsBackend.hdrFlashIntensity
                   : Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
          }

          HdrBoost {
            target: ledBloomTight
            captureMargin: 64
            width: led.width + 128
            height: led.height + 128
            x: layout.x + led.x - 64
            y: layout.y + led.y - 64
            active: led.showLit && Cpp_Misc_GraphicsBackend.effectsEnabled
            boost: led.flashing
                   ? Cpp_Misc_GraphicsBackend.hdrFlashIntensity
                   : Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
          }

          HdrBoost {
            target: led
            width: led.width
            height: led.height
            x: layout.x + led.x
            y: layout.y + led.y
            active: led.showLit
            boost: led.flashing
                   ? Cpp_Misc_GraphicsBackend.hdrFlashIntensity
                   : Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
          }
        }
      }
    }
  }
}
