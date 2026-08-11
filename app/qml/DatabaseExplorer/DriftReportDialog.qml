/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  staysOnTop: true
  preferredWidth: 860
  preferredHeight: 580
  title: qsTr("Comparison Report")

  //
  // Dialog state
  //
  property var report: ({})
  readonly property var affected: {
    const all = (root.report.datasets || []).filter(function(d) {
      return !!d.structural || (d.changed || 0) > 0
          || (d.onlyBaseline || 0) > 0 || (d.onlyCandidate || 0) > 0
    })
    return all
  }

  //
  // Maps the drift verdict to a plain-language headline
  //
  function headline(verdict) {
    if (verdict === "identical")
      return qsTr("The current project decodes this session exactly like the recorded one.")

    if (verdict === "value-drift")
      return qsTr("The current project produces different values from the same recording.")

    if (verdict === "coverage-drift")
      return qsTr("The current project finds a different number of readings in the recording.")

    if (verdict === "structural-drift")
      return qsTr("The current project has a different set of datasets than the recording.")

    if (verdict === "not_verifiable")
      return qsTr("This session cannot be compared mechanically.")

    return qsTr("The comparison could not be completed.")
  }

  //
  // Formats a capture timestamp (ns since recording start) as m:ss.mmm into the recording
  //
  function friendlyTime(ns) {
    if (ns === undefined || ns === null || ns < 0)
      return "-"

    const totalMs = Math.floor(ns / 1e6)
    const minutes = Math.floor(totalMs / 60000)
    const seconds = Math.floor((totalMs % 60000) / 1000)
    const millis  = totalMs % 1000
    return minutes + ":" + String(seconds).padStart(2, "0") + "."
        + String(millis).padStart(3, "0")
  }

  //
  // Loads a regression report and shows the window
  //
  function openFor(regressionReport) {
    root.report = regressionReport || {}
    root.show()
    root.raise()
  }

  //
  // Main layout
  //
  ColumnLayout {
    spacing: 8
    anchors.margins: 16
    anchors.fill: parent

    //
    // Verdict headline + candidate identification
    //
    Label {
      Layout.fillWidth: true
      wrapMode: Text.Wrap
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(1.15, true)
      text: root.headline(root.report.verdict)
    }

    Label {
      opacity: 0.6
      Layout.fillWidth: true
      elide: Text.ElideRight
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.uiFont
      visible: !!(root.report.candidate || {}).title || !!(root.report.candidate || {}).sha256
      text: qsTr("Compared against: %1").arg(
              (root.report.candidate || {}).title || qsTr("current project"))
    }

    //
    // Plain-language notes from the comparison
    //
    Repeater {
      model: root.report.notes || []

      delegate: Label {
        opacity: 0.8
        wrapMode: Text.Wrap
        Layout.fillWidth: true
        text: modelData
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
      }
    }

    //
    // Divider
    //
    Rectangle {
      height: 1
      Layout.fillWidth: true
      Layout.topMargin: 4
      Layout.bottomMargin: 4
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    //
    // Nothing-affected state
    //
    Label {
      opacity: 0.6
      visible: root.affected.length === 0
      Layout.alignment: Qt.AlignCenter
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.uiFont
      text: qsTr("No differences to show.")
    }

    //
    // Per-dataset side-by-side comparison
    //
    ListView {
      id: datasetList

      clip: true
      spacing: 12
      model: root.affected
      Layout.fillWidth: true
      Layout.fillHeight: true
      visible: root.affected.length > 0
      boundsBehavior: Flickable.StopAtBounds

      ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
      }

      delegate: ColumnLayout {
        spacing: 2
        width: datasetList.width - 16

        Label {
          Layout.fillWidth: true
          elide: Text.ElideRight
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.boldUiFont
          text: modelData.title || modelData.uniqueId
        }

        Label {
          opacity: 0.7
          Layout.fillWidth: true
          wrapMode: Text.Wrap
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          text: {
            if (modelData.structural === "added")
              return qsTr("Only exists in the current project.")

            if (modelData.structural === "removed")
              return qsTr("Only exists in the recorded project.")

            return qsTr("%1 of %2 values changed, %3 missing, %4 extra.")
                .arg(modelData.changed || 0)
                .arg(modelData.compared || 0)
                .arg(modelData.onlyBaseline || 0)
                .arg(modelData.onlyCandidate || 0)
          }
        }

        GridLayout {
          columns: 3
          rowSpacing: 2
          columnSpacing: 24
          Layout.fillWidth: true
          Layout.leftMargin: 12
          visible: (modelData.divergences || []).length > 0

          Label {
            opacity: 0.5
            text: qsTr("Time")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
          }

          Label {
            opacity: 0.5
            text: qsTr("Recorded project")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
          }

          Label {
            opacity: 0.5
            text: qsTr("Current project")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
          }

          Repeater {
            model: {
              const rows = []
              const samples = modelData.divergences || []
              for (let i = 0; i < samples.length; ++i) {
                const s      = samples[i]
                const parse  = s.stage === "parse"
                rows.push(root.friendlyTime(s.captureTimestampNs))
                rows.push(parse ? s.baselineRaw : s.baselineFinal)
                rows.push(parse ? s.candidateRaw : s.candidateFinal)
              }
              return rows
            }

            delegate: Label {
              elide: Text.ElideRight
              Layout.maximumWidth: 300
              text: String(modelData)
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.monoFont
            }
          }
        }

        Label {
          opacity: 0.5
          Layout.fillWidth: true
          Layout.leftMargin: 12
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
          visible: (modelData.changed || 0) > (modelData.divergences || []).length
                   && (modelData.divergences || []).length > 0
          text: qsTr("...and %1 more changed values.")
                  .arg((modelData.changed || 0) - (modelData.divergences || []).length)
        }
      }
    }

    //
    // Close button
    //
    RowLayout {
      Layout.fillWidth: true

      Item {
        Layout.fillWidth: true
      }

      Button {
        text: qsTr("Close")
        onClicked: root.close()
      }
    }
  }
}
