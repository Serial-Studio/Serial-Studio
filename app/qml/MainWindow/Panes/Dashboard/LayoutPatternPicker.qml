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
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.impl

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

//
// Layout gallery for the taskbar's auto-layout button. The thumbnails are not artwork: each asks
// the tiler for the geometry it would really produce, so one cannot disagree with what it does.
//
Popup {
  id: root

  //
  // The window manager the picker drives
  //
  required property SS_Ui.WindowManager windowManager

  //
  // Pattern ids, matching UI::Layouts::Pattern; the empty id is Grid
  //
  readonly property var patterns: [
    { "id": "",             "index": 0, "name": qsTr("Grid")           },
    { "id": "master-stack", "index": 1, "name": qsTr("Master + Stack") },
    { "id": "master-grid",  "index": 2, "name": qsTr("Master + Grid")  },
    { "id": "row",          "index": 3, "name": qsTr("Row")            },
    { "id": "column",       "index": 4, "name": qsTr("Column")         },
    { "id": "spiral",       "index": 5, "name": qsTr("Spiral")         }
  ]

  //
  // Live choice of whatever the taskbar is showing: the window manager owns it, so a group tab
  // carries one exactly like a user workspace does
  //
  readonly property int activeRatio: root.windowManager.layoutRatio
  readonly property bool tiling: root.windowManager.autoLayoutEnabled
  readonly property string activePattern: root.windowManager.layoutPattern
  readonly property bool glide: root.opened && !Cpp_Misc_GraphicsBackend.reduceMotion

  //
  // Reduces a ratio in sixteenths to its simplest fraction, so the stops read 1/4 .. 3/4
  //
  function ratioLabel(sixteenths) {
    let a = sixteenths
    let b = 16
    while (b) {
      const t = b
      b = a % b
      a = t
    }

    return (sixteenths / a) + "/" + (16 / a)
  }

  //
  // Preview re-tiling motion, shared by the four geometry behaviors of a preview cell
  //
  component Glide: NumberAnimation {
    duration: 220
    easing.type: Easing.OutCubic
  }

  function apply(patternId) {
    root.windowManager.selectLayoutPattern(patternId, root.activeRatio)
    root.close()
  }

  margins: 8
  padding: 12
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  transformOrigin: Cpp_Misc_Translator.rtl ? Popup.BottomLeft : Popup.BottomRight

  enter: Widgets.PopupEnter {}
  exit: Widgets.PopupExit {}

  background: Rectangle {
    radius: 10
    border.width: 1
    color: Cpp_ThemeManager.colors["start_menu_background"]
    border.color: Cpp_ThemeManager.colors["start_menu_border"]
  }

  contentItem: ColumnLayout {
    spacing: 10

    Label {
      opacity: 0.6
      text: qsTr("Layout")
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
    }

    //
    // Pattern thumbnails, three per row
    //
    Grid {
      columns: 3
      spacing: 8
      Layout.alignment: Qt.AlignHCenter

      Repeater {
        model: root.patterns

        delegate: Rectangle {
          id: _tile

          required property var modelData

          readonly property bool current: root.tiling
                                          && root.activePattern === _tile.modelData["id"]

          width: 104
          radius: 8
          height: 92
          border.width: 1
          color: _tile.current
                 ? Qt.alpha(Cpp_ThemeManager.colors["highlight"], 0.12)
                 : (_tileArea.containsMouse
                    ? Qt.alpha(Cpp_ThemeManager.colors["text"], 0.06)
                    : "transparent")
          border.color: _tile.current
                        ? Cpp_ThemeManager.colors["highlight"]
                        : Cpp_ThemeManager.colors["start_menu_border"]

          scale: Cpp_Misc_GraphicsBackend.reduceMotion
                 ? 1
                 : (_tileArea.pressed ? 0.97 : (_tileArea.containsMouse ? 1.03 : 1))

          Behavior on color { ColorAnimation { duration: 90 } }
          Behavior on border.color { ColorAnimation { duration: 140 } }
          Behavior on scale { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }

          ColumnLayout {
            spacing: 6
            anchors.margins: 8
            anchors.fill: parent

            //
            // Miniature of the real tiling, drawn on its own canvas so the widgets read as a
            // dashboard rather than as floating chips
            //
            Rectangle {
              id: _canvas

              clip: true
              radius: 4
              Layout.fillWidth: true
              Layout.fillHeight: true
              color: Cpp_ThemeManager.colors["dashboard_background"]

              //
              // Counted, not used as the model: a new list rebuilds the delegates, and a rebuilt
              // delegate cannot glide to the geometry a new split ratio gives it
              //
              readonly property var rects: root.windowManager.patternPreview(
                                             _tile.modelData["index"], 5, _canvas.width,
                                             _canvas.height, root.activeRatio)

              Repeater {
                model: _canvas.rects.length

                delegate: Rectangle {
                  id: _cell

                  required property int index

                  readonly property var rect: _cell.index < _canvas.rects.length
                                              ? _canvas.rects[_cell.index]
                                              : null

                  border.width: 1
                  x: _cell.rect !== null ? _cell.rect["x"] : 0
                  y: _cell.rect !== null ? _cell.rect["y"] : 0
                  width: _cell.rect !== null ? _cell.rect["width"] : 0
                  height: _cell.rect !== null ? _cell.rect["height"] : 0
                  color: Cpp_ThemeManager.colors["widget_window"]
                  border.color: Cpp_ThemeManager.colors["widget_border"]

                  Behavior on x {
                    enabled: root.glide
                    Glide {}
                  }

                  Behavior on y {
                    enabled: root.glide
                    Glide {}
                  }

                  Behavior on width {
                    enabled: root.glide
                    Glide {}
                  }

                  Behavior on height {
                    enabled: root.glide
                    Glide {}
                  }
                }
              }
            }

            Label {
              elide: Text.ElideRight
              Layout.fillWidth: true
              text: _tile.modelData["name"]
              horizontalAlignment: Text.AlignHCenter
              font: Cpp_Misc_CommonFonts.customUiFont(0.9, _tile.current)
              color: _tile.current ? Cpp_ThemeManager.colors["highlight"]
                                   : Cpp_ThemeManager.colors["text"]
            }
          }

          MouseArea {
            id: _tileArea

            hoverEnabled: true
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.apply(_tile.modelData["id"])
          }
        }
      }
    }

    //
    // Split ratio, offered only for the patterns that have a primary region
    //
    RowLayout {
      spacing: 10
      Layout.fillWidth: true
      visible: root.tiling
               && root.windowManager.patternHasPrimary(
                    Math.max(0, root.patterns.findIndex((p) => p["id"] === root.activePattern)))

      Label {
        opacity: 0.6
        text: qsTr("Split")
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.SegmentedControl {
        implicitHeight: 26
        currentIndex: root.windowManager.layoutRatioStops().indexOf(root.activeRatio)
        model: root.windowManager.layoutRatioStops().map(
                 (stop) => ({ "text": root.ratioLabel(stop) }))
        onActivated: (index) => {
          root.windowManager.selectLayoutPattern(
            root.activePattern, root.windowManager.layoutRatioStops()[index])
        }
      }
    }

    //
    // Divider: below it the dashboard stops tiling itself
    //
    Rectangle {
      height: 1
      opacity: 0.6
      Layout.fillWidth: true
      Layout.topMargin: 2
      color: Cpp_ThemeManager.colors["start_menu_border"]
    }

    //
    // Manual placement is the other answer to "how is this dashboard arranged", so it belongs
    // in the same menu - as a row, because it is a mode rather than an arrangement
    //
    Rectangle {
      radius: 6
      height: 34
      Layout.fillWidth: true
      color: !root.tiling
             ? Qt.alpha(Cpp_ThemeManager.colors["highlight"], 0.12)
             : (_manualArea.containsMouse
                ? Qt.alpha(Cpp_ThemeManager.colors["text"], 0.06)
                : "transparent")

      Behavior on color { ColorAnimation { duration: 90 } }

      RowLayout {
        spacing: 8
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        IconImage {
          sourceSize: Qt.size(18, 18)
          Layout.preferredWidth: 18
          Layout.preferredHeight: 18
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/icons/buttons/manual-placement.svg"
          color: !root.tiling ? Cpp_ThemeManager.colors["highlight"]
                              : Cpp_ThemeManager.colors["text"]
        }

        Label {
          Layout.fillWidth: true
          text: qsTr("Manual placement")
          Layout.alignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, !root.tiling)
          color: !root.tiling ? Cpp_ThemeManager.colors["highlight"]
                              : Cpp_ThemeManager.colors["text"]
        }

        IconImage {
          visible: !root.tiling
          sourceSize: Qt.size(14, 14)
          Layout.preferredWidth: 14
          Layout.preferredHeight: 14
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/icons/buttons/apply.svg"
          color: Cpp_ThemeManager.colors["highlight"]
        }
      }

      MouseArea {
        id: _manualArea

        hoverEnabled: true
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
          root.windowManager.autoLayoutEnabled = false
          root.close()
        }
      }
    }
  }
}
