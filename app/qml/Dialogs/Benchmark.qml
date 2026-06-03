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
import QtQuick.Controls.impl

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  title: qsTr("Benchmark")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Selected workload (indices into the runner's option lists)
  //
  property int framesIndex: 1
  property int secondsIndex: 1
  readonly property var results: runner.results
  readonly property bool running: runner.running
  readonly property var runner: Cpp_Benchmark_Runner

  //
  // Reset the results table when the dialog is dismissed (same as the Clear button)
  //
  onVisibleChanged: {
    if (!visible)
      runner.clearResults()
  }

  //
  // Number formatting helpers for the results table
  //
  function fmtInt(value) {
    return Math.round(value).toLocaleString(Qt.locale())
  }

  function fmtFps(value) {
    return qsTr("%1 frames/s").arg(Math.round(value).toLocaleString(Qt.locale()))
  }

  function fmtSeconds(value) {
    return qsTr("%1 s").arg(Number(value).toLocaleString(Qt.locale(), 'f', 2))
  }

  function resultText(row) {
    if (!row.gated)
      return qsTr("n/a")

    return row.passed ? qsTr("Pass") : qsTr("Fail")
  }

  function resultColor(row) {
    if (!row.gated)
      return Cpp_ThemeManager.colors["text"]

    return row.passed ? Cpp_ThemeManager.colors["alarm_ok"]
                      : Cpp_ThemeManager.colors["error"]
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12

    //
    // Title block: Apple HIG "Title / Subtitle" pairing
    //
    Label {
      Layout.fillWidth: true
      text: qsTr("Hotpath Benchmark")
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(1.6, true)
    }

    Label {
      opacity: 0.8
      Layout.fillWidth: true
      Layout.maximumWidth: 560
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Measures how fast this computer can extract, parse, and visualize " +
                 "frames through Serial Studio's data pipeline.")
    }

    //
    // Warning callout: the run blocks the main thread
    //
    Rectangle {
      radius: 6
      border.width: 1
      Layout.topMargin: 2
      Layout.fillWidth: true
      Layout.minimumWidth: 560
      implicitHeight: warningLayout.implicitHeight + 20
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      RowLayout {
        id: warningLayout

        spacing: 12
        anchors.margins: 10
        anchors.fill: parent

        Image {
          Layout.preferredWidth: 48
          Layout.preferredHeight: 48
          sourceSize: Qt.size(48, 48)
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/icons/notifications/warning.svg"
        }

        ColumnLayout {
          spacing: 2
          Layout.fillWidth: true

          Label {
            Layout.fillWidth: true
            text: qsTr("The interface will freeze while running")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1, true)
          }

          Label {
            opacity: 0.8
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.uiFont
            text: qsTr("Each phase runs flat-out on the main thread, so the window stops " +
                       "responding until it finishes. Your current project is reloaded " +
                       "automatically when the benchmark ends.")
          }
        }
      }
    }

    //
    // Workload selectors + run button
    //
    GridLayout {
      columns: 2
      rowSpacing: 8
      columnSpacing: 12
      Layout.topMargin: 4
      Layout.fillWidth: true

      Label {
        text: qsTr("Frames per phase:")
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.uiFont
      }

      ComboBox {
        Layout.fillWidth: true
        enabled: !root.running
        currentIndex: root.framesIndex
        model: root.runner.frameOptions
        onActivated: root.framesIndex = currentIndex
      }

      Label {
        text: qsTr("Minimum duration:")
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.uiFont
      }

      ComboBox {
        Layout.fillWidth: true
        enabled: !root.running
        currentIndex: root.secondsIndex
        model: root.runner.secondsOptions
        onActivated: root.secondsIndex = currentIndex
      }
    }

    //
    // Progress area (only while running)
    //
    ColumnLayout {
      spacing: 8
      Layout.topMargin: 4
      visible: root.running
      Layout.fillWidth: true

      RowLayout {
        spacing: 12
        Layout.fillWidth: true

        BusyIndicator {
          implicitWidth: 24
          implicitHeight: 24
          running: root.running
        }

        Label {
          Layout.fillWidth: true
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(1, true)
          text: root.runner.currentPhase.length > 0
                ? qsTr("Running %1...").arg(root.runner.currentPhase)
                : qsTr("Preparing...")
        }
      }

      ProgressBar {
        to: 1
        from: 0
        Layout.fillWidth: true
        value: root.runner.progress
      }
    }

    //
    // Results table
    //
    Rectangle {
      radius: 6
      border.width: 1
      Layout.topMargin: 4
      Layout.fillWidth: true
      Layout.minimumWidth: 560
      visible: root.results.length > 0
      implicitHeight: resultsColumn.implicitHeight + 2
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      ColumnLayout {
        id: resultsColumn

        spacing: 0
        anchors.margins: 1
        anchors.fill: parent

        //
        // Header row
        //
        RowLayout {
          spacing: 0
          Layout.fillWidth: true

          Label {
            topPadding: 8
            leftPadding: 10
            bottomPadding: 8
            text: qsTr("Pipeline")
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
          }

          Label {
            topPadding: 8
            bottomPadding: 8
            rightPadding: 10
            text: qsTr("Throughput")
            Layout.preferredWidth: 150
            horizontalAlignment: Text.AlignRight
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
          }

          Label {
            topPadding: 8
            bottomPadding: 8
            rightPadding: 10
            text: qsTr("Time")
            Layout.preferredWidth: 70
            horizontalAlignment: Text.AlignRight
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
          }

          Label {
            topPadding: 8
            bottomPadding: 8
            rightPadding: 10
            text: qsTr("Result")
            Layout.preferredWidth: 60
            horizontalAlignment: Text.AlignRight
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
          }
        }

        Rectangle {
          Layout.fillWidth: true
          implicitHeight: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        //
        // One row per completed phase
        //
        Repeater {
          model: root.results

          delegate: RowLayout {
            id: resultRow

            required property var modelData

            spacing: 0
            Layout.fillWidth: true

            Label {
              Layout.fillWidth: true
              leftPadding: 10
              topPadding: 6
              bottomPadding: 6
              elide: Text.ElideRight
              text: resultRow.modelData.label
              color: Cpp_ThemeManager.colors["text"]
            }

            Label {
              Layout.preferredWidth: 150
              rightPadding: 10
              topPadding: 6
              bottomPadding: 6
              horizontalAlignment: Text.AlignRight
              color: Cpp_ThemeManager.colors["text"]
              text: root.fmtFps(resultRow.modelData.fps)
              font: Cpp_Misc_CommonFonts.customMonoFont(0.9, false)
            }

            Label {
              Layout.preferredWidth: 70
              rightPadding: 10
              topPadding: 6
              bottomPadding: 6
              horizontalAlignment: Text.AlignRight
              color: Cpp_ThemeManager.colors["text"]
              text: root.fmtSeconds(resultRow.modelData.seconds)
              font: Cpp_Misc_CommonFonts.customMonoFont(0.9, false)
            }

            Label {
              topPadding: 6
              rightPadding: 10
              bottomPadding: 6
              Layout.preferredWidth: 60
              horizontalAlignment: Text.AlignRight
              text: root.resultText(resultRow.modelData)
              color: root.resultColor(resultRow.modelData)
              font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
            }
          }
        }
      }
    }

    //
    // Footnote (only with results): the gated targets
    //
    Label {
      opacity: 0.7
      Layout.fillWidth: true
      Layout.maximumWidth: 560
      wrapMode: Text.WordWrap
      visible: root.results.length > 0
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
      text: qsTr("Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, " +
                 "JavaScript 128 K). The export and dashboard phases are informational.")
    }

    //
    // Buttons: Apple HIG order (secondary left, primary at far right)
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 6
      Layout.fillWidth: true

      Widgets.IconButton {
        text: qsTr("Clear")
        onClicked: root.runner.clearResults()
        icon.source: "qrc:/icons/buttons/clear.svg"
        enabled: !root.running && root.results.length > 0
      }

      Item { Layout.fillWidth: true }

      Widgets.IconButton {
        text: qsTr("Close")
        enabled: !root.running
        onClicked: root.close()
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        rightPadding: 8
        enabled: !root.running
        icon.source: "qrc:/icons/buttons/apply.svg"
        text: root.running ? qsTr("Running...") : qsTr("Run Benchmark")
        onClicked: root.runner.start(root.framesIndex, root.secondsIndex)
      }
    }
  }
}
