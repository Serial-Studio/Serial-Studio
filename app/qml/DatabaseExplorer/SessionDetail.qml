/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
  icon: "qrc:/icons/panes/details.svg"

  //
  // Custom properties
  //
  property int sessionId: Cpp_Sessions_Manager.selectedSessionId
  property var metadata: sessionId >= 0
                         ? Cpp_Sessions_Manager.sessionMetadata(sessionId)
                         : ({})

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
    }
  }

  //
  // Report options dialog, opened by the Generate Report button
  //
  ReportOptionsDialog {
    id: _reportDialog
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
            text: root.metadata.project_title || "—"
            color: Cpp_ThemeManager.colors["text"]
            elide: Text.ElideRight
          }

          Label {
            opacity: 0.6
            text: qsTr("Started:")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.boldUiFont
          }
          Label {
            Layout.fillWidth: true
            text: root.metadata.started_at || "—"
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.monoFont
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

          TextField {
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

              Cpp_Sessions_Manager.addTag(label)
              const tags = Cpp_Sessions_Manager.tagList
              for (let i = 0; i < tags.length; ++i) {
                if (tags[i].label.toLowerCase() === label.toLowerCase()) {
                  Cpp_Sessions_Manager.assignTagToSession(
                    root.sessionId, tags[i].tag_id)
                  break
                }
              }

              tagField.text = ""
            }
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
          spacing: 8
          Layout.fillWidth: true

          Button {
            icon.width: 18
            icon.height: 18
            text: qsTr("Replay")
            visible: !root.operatorMode
            enabled: (root.metadata.frame_count || 0) > 0
            icon.source: "qrc:/icons/buttons/play.svg"
            onClicked: Cpp_Sessions_Manager.replaySelectedSession()
          }

          Button {
            icon.width: 18
            icon.height: 18
            text: qsTr("Export CSV")
            icon.source: "qrc:/icons/buttons/export-csv.svg"
            onClicked: Cpp_Sessions_Manager.exportSessionToCsv(root.sessionId)
            enabled: (root.metadata.frame_count || 0) > 0 && !Cpp_Sessions_Manager.csvExportBusy
          }

          Button {
            icon.width: 18
            icon.height: 18
            text: qsTr("Generate Report")
            icon.source: "qrc:/icons/buttons/report.svg"
            onClicked: _reportDialog.openFor(root.sessionId)
            enabled: (root.metadata.frame_count || 0) > 0 && !Cpp_Sessions_Manager.pdfExportBusy
          }

          Item {
            Layout.fillWidth: true
          }

          Button {
            icon.width: 18
            icon.height: 18
            text: qsTr("Delete")
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
