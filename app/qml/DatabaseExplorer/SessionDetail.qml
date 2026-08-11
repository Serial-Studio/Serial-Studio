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
  property var verification: sessionId >= 0
                             ? Cpp_Sessions_Manager.latestVerification(sessionId)
                             : ({})
  property var regression: ({})
  property int regressionAffectedCount: 0

  //
  // Narrowest width at which the action row stays fully visible; the explorer
  // window derives its minimum size from this so buttons can never be clipped
  //
  readonly property real minimumUsableWidth: actionRow.implicitWidth + 32 + 18

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
    }

    function onVerificationFinished() {
      root.verification = root.sessionId >= 0
                          ? Cpp_Sessions_Manager.latestVerification(root.sessionId)
                          : {}
    }

    function onRegressionReportChanged() {
      root.regression = Cpp_Sessions_Manager.lastRegressionReport
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
            text: qsTr("Frames:")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
          }
          Label {
            Layout.fillWidth: true
            text: root.metadata.frame_count || "0"
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

        //
        // Reproducibility section
        //
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
        }

        //
        // Drift vs current project (spec 0047; ephemeral, never stored with the session)
        //
        Label {
          visible: !root.operatorMode
          text: qsTr("Drift vs Current Project")
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        //
        // Regression drift report
        //
        ColumnLayout {
          spacing: 4
          Layout.fillWidth: true
          visible: !root.operatorMode

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

        //
        // Spacer
        //
        Item {
          implicitHeight: 8
        }

        //
        // Action buttons
        //
        RowLayout {
          id: actionRow

          spacing: 8
          Layout.fillWidth: true

          Widgets.IconButton {
            text: qsTr("Replay")
            visible: !root.operatorMode
            icon.source: "qrc:/icons/buttons/play.svg"
            enabled: (root.metadata.frame_count || 0) > 0
            onClicked: Cpp_Sessions_Manager.replaySelectedSession()
          }

          Widgets.IconButton {
            text: qsTr("Export CSV")
            icon.source: "qrc:/icons/buttons/export-csv.svg"
            onClicked: Cpp_Sessions_Manager.exportSessionToCsv(root.sessionId)
            enabled: (root.metadata.frame_count || 0) > 0 && !Cpp_Sessions_Manager.csvExportBusy
          }

          Widgets.IconButton {
            text: qsTr("Generate Report")
            icon.source: "qrc:/icons/buttons/report.svg"
            onClicked: _reportDialog.openFor(root.sessionId)
            enabled: (root.metadata.frame_count || 0) > 0 && !Cpp_Sessions_Manager.pdfExportBusy
          }

          Widgets.IconButton {
            visible: !root.operatorMode
            icon.source: "qrc:/icons/buttons/apply.svg"
            ToolTip.visible: hovered && !root.metadata.ended_at
            ToolTip.text: qsTr("Only completed sessions can be verified")
            onClicked: Cpp_Sessions_Manager.verifySession(root.sessionId)
            enabled: !!root.metadata.ended_at && !Cpp_Sessions_Manager.verificationBusy
            text: Cpp_Sessions_Manager.verificationBusy && !Cpp_Sessions_Manager.regressionBusy
                  ? qsTr("Verifying…")
                  : qsTr("Verify")
          }

          Widgets.IconButton {
            visible: !root.operatorMode
            icon.source: "qrc:/icons/buttons/test.svg"
            ToolTip.visible: hovered && !root.metadata.ended_at
            ToolTip.text: qsTr("Only completed sessions can be checked against a project")
            onClicked: Cpp_Sessions_Manager.regressSession(root.sessionId)
            enabled: !!root.metadata.ended_at && !Cpp_Sessions_Manager.verificationBusy
            text: Cpp_Sessions_Manager.regressionBusy ? qsTr("Checking…") : qsTr("Check Project")
          }

          Widgets.IconButton {
            text: qsTr("View Report")
            icon.source: "qrc:/icons/buttons/report.svg"
            visible: !root.operatorMode && !!root.regression.verdict
            onClicked: _driftDialog.openFor(root.regression)
          }

          Item {
            Layout.fillWidth: true
          }

          Widgets.IconButton {
            text: qsTr("Delete")
            opacity: enabled ? 1 : 0.5
            visible: !root.operatorMode
            enabled: !Cpp_Sessions_Manager.locked
            icon.source: "qrc:/icons/buttons/trash.svg"
            ToolTip.visible: hovered && Cpp_Sessions_Manager.locked
            ToolTip.text: qsTr("Unlock the session file to delete sessions")
            onClicked: Cpp_Sessions_Manager.confirmDeleteSession(root.sessionId)
          }
        }
      }
    }
  }
}
