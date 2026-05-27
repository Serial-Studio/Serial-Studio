/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
import ".." as Widgets

Item {
  id: root

  implicitWidth: 380
  implicitHeight: 320

  //
  // Widget data inputs (unused; Stopwatch has no project-bound model)
  //
  property var color
  property var model
  property var windowRoot
  property string widgetId

  //
  // Timer state: accumulated ms across run/pause segments + the wall-clock
  // start of the current active segment.
  //
  property real accumMs: 0
  property real displayMs: 0
  property real runStartMs: 0
  property bool running: false

  //
  // Pure read: never mutates accumMs / runStartMs.
  //
  function currentMs() {
    if (!running)
      return accumMs

    return accumMs + (Date.now() - runStartMs)
  }

  //
  // 30 Hz tick: only running while the stopwatch is active.
  //
  Timer {
    id: tickTimer

    repeat: true
    interval: 33
    running: root.running
    onTriggered: root.displayMs = root.currentMs()
  }

  //
  // Start / stop toggle.
  //
  function toggleRun() {
    if (running) {
      accumMs   = currentMs()
      running   = false
      displayMs = accumMs
      return
    }

    runStartMs = Date.now()
    running    = true
    displayMs  = accumMs
  }

  //
  // Records the current total + delta from the previous lap.
  //
  function recordLap() {
    const total = currentMs()
    if (total <= 0)
      return

    const prevTotal = laps.count > 0 ? laps.get(0).totalMs : 0
    laps.insert(0, {
      n: laps.count + 1,
      lapMs: Math.max(0, total - prevTotal),
      totalMs: total
    })
  }

  //
  // Stops, zeros state, clears laps.
  //
  function resetAll() {
    running    = false
    accumMs    = 0
    runStartMs = 0
    displayMs  = 0
    laps.clear()
  }

  //
  // Formats milliseconds as HH:MM:SS.mmm
  //
  function formatMs(ms) {
    const totalMs   = Math.max(0, Math.floor(ms))
    const millis    = totalMs % 1000
    const totalSec  = Math.floor(totalMs / 1000)
    const seconds   = totalSec % 60
    const totalMin  = Math.floor(totalSec / 60)
    const minutes   = totalMin % 60
    const hours     = Math.floor(totalMin / 60)
    const pad       = (n, w) => String(n).padStart(w, "0")
    return pad(hours, 2) + ":" + pad(minutes, 2) + ":" + pad(seconds, 2)
         + "." + pad(millis, 3)
  }

  //
  // Persisted laps for the current session (cleared on Reset)
  //
  ListModel {
    id: laps
  }

  //
  // Main layout
  //
  ColumnLayout {
    spacing: 8
    anchors.margins: 12
    anchors.fill: parent

    //
    // Digital readout: mirrors Gauge.qml value box (raised text, console palette)
    //
    Rectangle {
      id: readoutBox

      radius: 4
      border.width: 1
      antialiasing: true
      Layout.fillWidth: true
      Layout.preferredHeight: readoutText.implicitHeight + 18
      color: Cpp_ThemeManager.colors["console_base"]
      border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.35)

      Text {
        id: readoutText

        anchors.centerIn: parent
        text: root.formatMs(root.displayMs)
        font: Cpp_Misc_CommonFonts.customMonoFont(2.20, true)
        color: root.running
               ? Cpp_ThemeManager.colors["console_text"]
               : Qt.lighter(Cpp_ThemeManager.colors["console_text"], 0.95)
      }
    }

    //
    // Button row: Start/Stop, Lap, Reset
    //
    RowLayout {
      spacing: 6
      Layout.fillWidth: true

      Button {
        id: startStopBtn

        Layout.fillWidth: true
        Layout.preferredHeight: 32
        font: Cpp_Misc_CommonFonts.boldUiFont
        icon.width: 14
        icon.height: 14
        icon.color: root.running
                    ? Cpp_ThemeManager.colors["alarm"]
                    : Cpp_ThemeManager.colors["highlight"]
        icon.source: root.running
                     ? "qrc:/icons/buttons/stopwatch-stop.svg"
                     : "qrc:/icons/buttons/stopwatch-start.svg"
        text: root.running ? qsTr("Stop") : qsTr("Start")
        onClicked: root.toggleRun()
      }

      Button {
        id: lapBtn

        Layout.fillWidth: true
        Layout.preferredHeight: 32
        font: Cpp_Misc_CommonFonts.uiFont
        text: qsTr("Lap")
        enabled: root.running || root.displayMs > 0
        opacity: enabled ? 1.0 : 0.45
        icon.width: 14
        icon.height: 14
        icon.source: "qrc:/icons/buttons/stopwatch-lap.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        onClicked: root.recordLap()
      }

      Button {
        id: resetBtn

        Layout.fillWidth: true
        Layout.preferredHeight: 32
        font: Cpp_Misc_CommonFonts.uiFont
        text: qsTr("Reset")
        enabled: root.displayMs > 0 || laps.count > 0
        opacity: enabled ? 1.0 : 0.45
        icon.width: 14
        icon.height: 14
        icon.source: "qrc:/icons/buttons/stopwatch-reset.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        onClicked: root.resetAll()
      }
    }

    //
    // Laps table
    //
    Rectangle {
      clip: true
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      color: Cpp_ThemeManager.colors["table_cell_bg"]
      border.color: Cpp_ThemeManager.colors["groupbox_hard_border"]

      //
      // Header row that sits above the ListView
      //
      Rectangle {
        id: header

        height: 24
        anchors {
          margins: 1
          top: parent.top
          left: parent.left
          right: parent.right
        }

        color: Cpp_ThemeManager.colors["alternate_base"]

        RowLayout {
          spacing: 0
          anchors.fill: parent
          anchors.leftMargin: 10
          anchors.rightMargin: 10

          Label {
            Layout.preferredWidth: 48
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["text"]
            verticalAlignment: Text.AlignVCenter
            text: qsTr("#")
          }

          Label {
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["text"]
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Lap")
          }

          Label {
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["text"]
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            text: qsTr("Total")
          }
        }

        Rectangle {
          height: 1
          anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
          }

          color: Cpp_ThemeManager.colors["groupbox_border"]
        }
      }

      ListView {
        id: lapList

        clip: true
        spacing: 0
        model: laps
        boundsBehavior: Flickable.StopAtBounds

        anchors {
          margins: 1
          left: parent.left
          top: header.bottom
          right: parent.right
          bottom: parent.bottom
        }

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Rectangle {
          id: lapRow

          required property int index
          required property var model

          width: lapList.width
          height: 24
          color: (index % 2 === 0)
                 ? Cpp_ThemeManager.colors["table_cell_bg"]
                 : Cpp_ThemeManager.colors["alternate_base"]

          RowLayout {
            spacing: 0
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            Label {
              Layout.preferredWidth: 48
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              verticalAlignment: Text.AlignVCenter
              text: "#" + lapRow.model.n
            }

            Label {
              Layout.fillWidth: true
              font: Cpp_Misc_CommonFonts.customMonoFont(1.0, false)
              color: Cpp_ThemeManager.colors["text"]
              verticalAlignment: Text.AlignVCenter
              text: root.formatMs(lapRow.model.lapMs)
            }

            Label {
              Layout.fillWidth: true
              font: Cpp_Misc_CommonFonts.customMonoFont(1.0, false)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              verticalAlignment: Text.AlignVCenter
              horizontalAlignment: Text.AlignRight
              text: root.formatMs(lapRow.model.totalMs)
            }
          }
        }

        //
        // Empty-state placeholder when no laps have been recorded yet
        //
        ColumnLayout {
          spacing: 8
          anchors.centerIn: parent
          visible: lapList.count === 0
          width: Math.min(parent.width - 48, 280)

          Label {
            Layout.fillWidth: true
            text: qsTr("No laps recorded")
            color: Cpp_ThemeManager.colors["placeholder_text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1.0, false)
            horizontalAlignment: Text.AlignHCenter
          }

          Label {
            Layout.fillWidth: true
            text: qsTr("Press Lap while the stopwatch is running")
            color: Cpp_ThemeManager.colors["placeholder_text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
          }
        }
      }
    }
  }
}
