/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets
import "../Commands" as Commands

Widgets.Pane {
  id: root

  title: qsTr("Session Details")
  headerVisible: !root.operatorMode
  icon: Cpp_Misc_IconRegistry.icon("panes", "details", 16)

  //
  // Custom properties
  //
  property int sessionId: Cpp_Sessions_Manager.selectedSessionId
  property var metadata: sessionId >= 0
                         ? Cpp_Sessions_Manager.sessionMetadata(sessionId)
                         : ({})
  property var streamStats: []
  property var verification: sessionId >= 0
                             ? Cpp_Sessions_Manager.latestVerification(sessionId)
                             : ({})
  property var regression: ({})
  property int regressionAffectedCount: 0

  //
  // Narrowest usable width of the pane; the explorer window derives its minimum
  // size from this. The command band scrolls, so it imposes no floor of its own.
  //
  readonly property real minimumUsableWidth: 360

  //
  // Maps a stored verdict string to user-facing text
  //
  function verdictLabel(verdict) {
    if (verdict === "reproduced")
      return qsTr("Reproduced")

    if (verdict === "diverged")
      return qsTr("Diverged")

    if (verdict === "partial")
      return qsTr("Partially verified")

    if (verdict === "not_verifiable")
      return qsTr("Not mechanically verifiable")

    if (verdict === "error")
      return qsTr("Verification error")

    return qsTr("Never verified")
  }

  //
  // Maps a regression drift verdict to user-facing text
  //
  function driftLabel(verdict) {
    if (verdict === "identical")
      return qsTr("Identical")

    if (verdict === "value-drift")
      return qsTr("Value drift")

    if (verdict === "coverage-drift")
      return qsTr("Coverage drift")

    if (verdict === "structural-drift")
      return qsTr("Structural drift")

    if (verdict === "not_verifiable")
      return qsTr("Not mechanically comparable")

    return qsTr("Check failed")
  }

  //
  // True when the explorer is opened from a deployed shortcut. Edits are
  // then restricted to the session currently being recorded.
  //
  readonly property bool operatorMode: typeof app !== "undefined"
                                       && app.runtimeMode

  //
  // Author mode: always editable. Operator mode: only the live session is editable
  //
  readonly property bool editsAllowed:
    !operatorMode
    || (Cpp_CommercialBuild
        && root.sessionId >= 0
        && root.sessionId === Cpp_Sessions_Export.currentSessionId)

  //
  // Formats a session's recorded payload size. Frame count was a misleading metric for stream
  // sources, which write blocks rather than reading rows and so always reported one frame.
  //
  function formatSize(bytes) {
    const value = bytes || 0
    if (value <= 0)
      return "--"

    if (value < 1024)
      return qsTr("%1 B").arg(value)

    if (value < 1024 * 1024)
      return qsTr("%1 KB").arg((value / 1024).toFixed(1))

    if (value < 1024 * 1024 * 1024)
      return qsTr("%1 MB").arg((value / (1024 * 1024)).toFixed(1))

    return qsTr("%1 GB").arg((value / (1024 * 1024 * 1024)).toFixed(2))
  }

  //
  // Summarises recorded stream data so a capture can be confirmed without replaying it
  //
  function streamSummary() {
    if (!root.streamStats || root.streamStats.length === 0)
      return ""

    var samples = 0
    var start = -1
    var end = -1
    for (var i = 0; i < root.streamStats.length; ++i) {
      var entry = root.streamStats[i]
      samples += entry.sampleCount
      if (start < 0 || entry.startNs < start)
        start = entry.startNs

      if (end < 0 || entry.endNs > end)
        end = entry.endNs
    }

    var seconds = (end - start) / 1000000000
    return qsTr("%1 samples over %2 s, %3 datasets")
             .arg(samples).arg(seconds.toFixed(1)).arg(root.streamStats.length)
  }

  //
  // Refresh when selected session changes
  //
  Connections {
    target: Cpp_Sessions_Manager
    function onSelectedSessionChanged() {
      root.sessionId = Cpp_Sessions_Manager.selectedSessionId
      root.metadata  = root.sessionId >= 0
                       ? Cpp_Sessions_Manager.sessionMetadata(root.sessionId)
                       : {}
      root.verification = root.sessionId >= 0
                          ? Cpp_Sessions_Manager.latestVerification(root.sessionId)
                          : {}
      root.streamStats = []
      root.regression = {}
      root.regressionAffectedCount = 0
      if (root.sessionId >= 0)
        Cpp_Sessions_Manager.requestStreamStats(root.sessionId)
    }

    function onSessionStreamStatsReady(id, stats) {
      if (id === root.sessionId)
        root.streamStats = stats
    }

    function onVerificationFinished() {
      root.verification = root.sessionId >= 0
                          ? Cpp_Sessions_Manager.latestVerification(root.sessionId)
                          : {}
    }

    function onRegressionReportChanged() {
      const report = Cpp_Sessions_Manager.lastRegressionReport
      const reportId = (report && report.sessionId !== undefined) ? report.sessionId : -1
      if (reportId >= 0 && reportId !== root.sessionId)
        return

      root.regression = report
      if (!report || !report.verdict)
        root.regressionAffectedCount = 0
    }
  }

  //
  // Report options dialog, opened by the Generate Report button
  //
  ReportOptionsDialog {
    id: _reportDialog
  }

  //
  // Side-by-side drift comparison window (spec 0047)
  //
  DriftReportDialog {
    id: _driftDialog
  }

  //
  // Main layout
  //
  ColumnLayout {
    spacing: 0
    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -9
    }

    //
    // Empty state
    //
    Label {
      opacity: 0.5
      visible: root.sessionId < 0
      Layout.alignment: Qt.AlignCenter
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Select a session to view details.")
    }

    //
    // Session commands: secondary ribbon band, pinned above the scrolling detail
    //
    Rectangle {
      id: commandBand

      z: 2
      Layout.fillWidth: true
      Layout.preferredHeight: 80
      visible: root.sessionId >= 0
      color: Cpp_ThemeManager.colors["groupbox_background"]

      Rectangle {
        height: 1
        width: parent.width
        anchors.bottom: parent.bottom
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      DatabaseCommandBindings {
        id: _detailBindings

        reportDialog: _reportDialog
      }

      Commands.CommandModel {
        id: _detailModel

        context: "database"
        bindingSets: [_detailBindings]
      }

      Widgets.CommandToolbar {
        model: _detailModel
        secondaryToolbar: true
        surface: "database-detail-toolbar"
        anchors {
          fill: parent
          leftMargin: 4
          rightMargin: 4
        }
      }
    }

    //
    // Detail view (visible when a session is selected)
    //
    Flickable {
      clip: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      visible: root.sessionId >= 0
      boundsBehavior: Flickable.StopAtBounds
      contentHeight: detailColumn.implicitHeight + 32

      ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
      }

      ColumnLayout {
        id: detailColumn

        spacing: 12
        width: parent.width

        anchors {
          margins: 16
          top: parent.top
          left: parent.left
          right: parent.right
        }

        //
        // Metadata grid
        //
        GridLayout {
          columns: 2
          rowSpacing: 6
          columnSpacing: 16
          Layout.fillWidth: true

          Label {
            opacity: 0.6
            text: qsTr("Project:")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
          }
          Label {
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["text"]
            elide: Text.ElideRight
            text: root.metadata.project_title || "--.--"
          }

          Label {
            opacity: 0.6
            text: qsTr("Started:")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
          }
          Label {
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.monoFont
            text: root.metadata.started_at || "--.--"
          }

          Label {
            opacity: 0.6
            text: qsTr("Ended:")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
          }
          Label {
            Layout.fillWidth: true
            text: root.metadata.ended_at || qsTr("(in progress)")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.monoFont
          }

          Label {
            opacity: 0.6
            text: qsTr("Size:")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
          }
          Label {
            Layout.fillWidth: true
            text: root.formatSize(root.metadata.size_bytes)
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.monoFont
          }

          Label {
            opacity: 0.6
            visible: root.streamStats.length > 0
            text: qsTr("Stream data:")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
          }
          Label {
            Layout.fillWidth: true
            visible: root.streamStats.length > 0
            text: root.streamSummary()
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.monoFont
          }
        }

        //
        // Notes section
        //
        Label {
          text: qsTr("Notes")
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        TextArea {
          id: notesArea

          Layout.fillWidth: true
          Layout.preferredHeight: 80
          wrapMode: TextEdit.Wrap
          readOnly: !root.editsAllowed
          opacity: root.editsAllowed ? 1 : 0.7
          placeholderText: root.editsAllowed
                           ? qsTr("Add session notes…")
                           : qsTr("Notes are read-only for completed sessions.")
          text: Cpp_Sessions_Manager.selectedSessionNotes
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.uiFont

          background: Rectangle {
            color: Cpp_ThemeManager.colors["base"]
            border.width: 1
            border.color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          onEditingFinished: {
            if (root.editsAllowed)
              Cpp_Sessions_Manager.selectedSessionNotes = text
          }
        }

        //
        // Tags section
        //
        Label {
          text: qsTr("Tags")
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        //
        // Session tags
        //
        Flow {
          spacing: 6
          Layout.fillWidth: true

          Repeater {
            model: Cpp_Sessions_Manager.selectedSessionTags

            delegate: Rectangle {
              width: chipRow.implicitWidth + 16
              height: 28
              radius: 4
              color: Cpp_ThemeManager.colors["highlight"]

              Row {
                id: chipRow

                spacing: 4
                anchors.centerIn: parent

                Label {
                  text: modelData.label
                  font: Cpp_Misc_CommonFonts.uiFont
                  anchors.verticalCenter: parent.verticalCenter
                  color: Cpp_ThemeManager.colors["highlighted_text"]
                }

                ToolButton {
                  flat: true
                  icon.width: 12
                  icon.height: 12
                  background: Item {}
                  visible: root.editsAllowed
                  anchors.verticalCenter: parent.verticalCenter
                  icon.source: "qrc:/icons/buttons/close.svg"
                  icon.color: Cpp_ThemeManager.colors["highlighted_text"]
                  onClicked: Cpp_Sessions_Manager.removeTagFromSession(root.sessionId, modelData.tag_id)
                }
              }
            }
          }
        }

        //
        // Add new tag
        //
        RowLayout {
          spacing: 8
          Layout.fillWidth: true
          visible: root.editsAllowed

          Widgets.LineField {
            id: tagField

            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("New tag…")
            onAccepted: addTagBtn.clicked()
          }

          Button {
            id: addTagBtn

            text: qsTr("Add")
            enabled: tagField.text.trim().length > 0
            onClicked: {
              const label = tagField.text.trim()
              if (label.length === 0)
                return

              Cpp_Sessions_Manager.addTagAndAssign(root.sessionId, label)
              tagField.text = ""
            }
          }
        }

        /* Reproducibility section (hidden)
        Label {
          text: qsTr("Reproducibility")
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        //
        // Latest verdict + provenance
        //
        ColumnLayout {
          spacing: 4
          Layout.fillWidth: true

          Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
            text: root.verdictLabel(root.verification.verdict)
          }

          Label {
            opacity: 0.6
            Layout.fillWidth: true
            elide: Text.ElideRight
            visible: !!root.verification.verified_at
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.monoFont
            text: qsTr("Checked %1 with version %2").arg(
                    root.verification.verified_at || "")
                  .arg(root.verification.app_version || "")
          }

          Label {
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            visible: root.verification.verdict === "error" && !!(root.verification.detail || {}).error
            color: Cpp_ThemeManager.colors["alarm"]
            font: Cpp_Misc_CommonFonts.uiFont
            text: (root.verification.detail || {}).error || ""
          }

          Label {
            opacity: 0.8
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            visible: root.verification.verdict === "error" && !!(root.verification.detail || {}).hint
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            text: (root.verification.detail || {}).hint || ""
          }

          Repeater {
            model: (root.verification.detail || {}).notes || []

            delegate: Label {
              opacity: 0.8
              wrapMode: Text.Wrap
              Layout.fillWidth: true
              text: modelData
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            }
          }

          Label {
            opacity: 0.6
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            text: qsTr("Re-runs the archived raw data through the current build and compares "
                       + "it against the recorded values. This proves the archive is "
                       + "reproducible; it is not a determinism guarantee, a safety function, "
                       + "or a calibration authority.")
          }
        }*/

        //
        // Project comparison (spec 0047; ephemeral, never stored with the session)
        //
        Label {
          visible: !root.operatorMode
          text: qsTr("Project Comparison")
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        //
        // Regression drift report
        //
        Rectangle {
          Layout.fillWidth: true
          Layout.preferredHeight: 180
          visible: !root.operatorMode
          border.width: 1
          color: Cpp_ThemeManager.colors["base"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          ScrollView {
            id: _comparisonView

            clip: true
            contentWidth: availableWidth
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            anchors {
              margins: 8
              fill: parent
            }

            ColumnLayout {
              spacing: 4
              visible: !root.operatorMode
              width: _comparisonView.availableWidth

              Label {
                opacity: 0.6
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                visible: !root.regression.verdict
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                text: qsTr("Compares this session's decoded values against the project currently "
                           + "open in the editor. The result reflects a transient candidate and "
                           + "is not stored with the session.")
              }

              Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                visible: !!root.regression.verdict
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.boldUiFont
                text: root.driftLabel(root.regression.verdict)
              }

              Label {
                opacity: 0.6
                Layout.fillWidth: true
                elide: Text.ElideRight
                visible: !!(root.regression.candidate || {}).title
                         || !!(root.regression.candidate || {}).sha256
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.monoFont
                text: qsTr("Candidate: %1 (%2)").arg(
                        (root.regression.candidate || {}).title || qsTr("untitled"))
                      .arg(((root.regression.candidate || {}).sha256 || "").substring(0, 12))
              }

              Label {
                opacity: 0.6
                Layout.fillWidth: true
                elide: Text.ElideRight
                visible: !!root.regression.baselineReproduction
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                text: qsTr("Archived-configuration reproduction status: %1").arg(
                        root.regression.baselineReproduction || "")
              }

              Label {
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                visible: root.regression.verdict === "error" && !!root.regression.error
                color: Cpp_ThemeManager.colors["alarm"]
                font: Cpp_Misc_CommonFonts.uiFont
                text: root.regression.error || ""
              }

              Label {
                opacity: 0.8
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                visible: root.regression.verdict === "error" && !!root.regression.hint
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                text: root.regression.hint || ""
              }

              Repeater {
                model: root.regression.notes || []

                delegate: Label {
                  opacity: 0.8
                  wrapMode: Text.Wrap
                  Layout.fillWidth: true
                  text: modelData
                  color: Cpp_ThemeManager.colors["text"]
                  font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                }
              }

              Repeater {
                model: {
                  const all = (root.regression.datasets || []).filter(function(d) {
                    return !!d.structural || (d.changed || 0) > 0
                        || (d.onlyBaseline || 0) > 0 || (d.onlyCandidate || 0) > 0
                  })
                  root.regressionAffectedCount = all.length
                  return all.slice(0, 20)
                }

                delegate: Label {
                  opacity: 0.8
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  color: Cpp_ThemeManager.colors["text"]
                  font: Cpp_Misc_CommonFonts.monoFont
                  text: {
                    if (modelData.structural === "added")
                      return qsTr("%1: only in the current project").arg(modelData.title || modelData.uniqueId)

                    if (modelData.structural === "removed")
                      return qsTr("%1: only in the recorded project").arg(modelData.title || modelData.uniqueId)

                    return qsTr("%1: %2 of %3 values changed, %4 missing, %5 extra")
                        .arg(modelData.title || modelData.uniqueId)
                        .arg(modelData.changed || 0)
                        .arg(modelData.compared || 0)
                        .arg(modelData.onlyBaseline || 0)
                        .arg(modelData.onlyCandidate || 0)
                  }
                }
              }

              Label {
                opacity: 0.6
                Layout.fillWidth: true
                visible: root.regressionAffectedCount > 20
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                text: qsTr("…and %1 more datasets are affected.")
                        .arg(root.regressionAffectedCount - 20)
              }
            }
          }
        }
      }
    }
  }
}
