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

  title: qsTr("Sessions")
  icon: Cpp_Misc_IconRegistry.icon("panes", "sessions", 16)
  headerVisible: typeof app === "undefined" || !app.runtimeMode

  //
  // Custom properties
  //
  property string searchText: ""
  property var verdicts: Cpp_Sessions_Manager.latestVerdicts()

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
  // Maps a stored verdict string to a short badge label
  //
  function verdictBadge(sessionId) {
    const verdict = verdicts[String(sessionId)]
    if (verdict === "reproduced")
      return qsTr("Reproduced")

    if (verdict === "diverged")
      return qsTr("Diverged")

    if (verdict === "partial")
      return qsTr("Partial")

    if (verdict === "not_verifiable")
      return qsTr("Not verifiable")

    if (verdict === "error")
      return qsTr("Error")

    return "--.--"
  }

  //
  // Refresh verdict badges when the session list changes (a finished verification also
  // emits sessionsChanged, so one handler covers both)
  //
  Connections {
    target: Cpp_Sessions_Manager
    function onSessionsChanged() {
      root.verdicts = Cpp_Sessions_Manager.latestVerdicts()
    }
  }

  readonly property var filteredSessions: {
    const q = searchText.toLowerCase().trim()
    if (!q)
      return Cpp_Sessions_Manager.sessionList

    return Cpp_Sessions_Manager.sessionList.filter(function(s) {
      return (s.project_title || "").toLowerCase().indexOf(q) >= 0
             || (s.started_at || "").toLowerCase().indexOf(q) >= 0
             || (s.tag_labels || "").toLowerCase().indexOf(q) >= 0
             || (s.notes || "").toLowerCase().indexOf(q) >= 0
    })
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
    // Search field
    //
    Rectangle {
      implicitHeight: 48
      Layout.topMargin: -1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_background"]

      Rectangle {
        height: 1
        width: parent.width
        anchors.bottom: parent.bottom
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      Widgets.SearchField {
        implicitHeight: 32
        placeholderText: qsTr("Search")
        color: Cpp_ThemeManager.colors["base"]
        onTextChanged: root.searchText = text

        anchors {
          leftMargin: 6
          rightMargin: 6
          left: parent.left
          right: parent.right
          verticalCenter: parent.verticalCenter
        }
      }
    }

    //
    // Table header
    //
    Widgets.ProjectTableHeader {
      Layout.fillWidth: true
      columns: [
        { title: qsTr("Date"),     width: 160 },
        { title: qsTr("Size"),     width: 80  },
        { title: qsTr("Verified"), width: 90  },
        { title: qsTr("Tags"),     width: -1  }
      ]
    }

    //
    // Session list
    //
    ListView {
      id: sessionListView

      clip: true
      spacing: 0
      Layout.fillWidth: true
      Layout.fillHeight: true
      model: root.filteredSessions
      boundsBehavior: Flickable.StopAtBounds

      ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
      }

      delegate: Widgets.ProjectTableRow {
        id: sessionRow

        readonly property bool isCurrent:
          modelData.session_id === Cpp_Sessions_Manager.selectedSessionId

        color: isCurrent
               ? Cpp_ThemeManager.colors["highlight"]
               : Cpp_ThemeManager.colors["table_cell_bg"]

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor
          onClicked: Cpp_Sessions_Manager.selectedSessionId = modelData.session_id
        }

        RowLayout {
          spacing: 0
          anchors.fill: parent

          Label {
            Layout.preferredWidth: 160
            Layout.alignment: Qt.AlignVCenter
            leftPadding: 8
            elide: Text.ElideRight
            text: modelData.started_at || ""
            font: Cpp_Misc_CommonFonts.monoFont
            color: sessionRow.isCurrent
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : sessionRow.textColor
          }

          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            color: sessionRow.separatorColor
          }

          Label {
            Layout.preferredWidth: 80
            Layout.alignment: Qt.AlignVCenter
            leftPadding: 8
            text: root.formatSize(modelData.size_bytes)
            font: Cpp_Misc_CommonFonts.monoFont
            color: sessionRow.isCurrent
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : sessionRow.textColor
          }

          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            color: sessionRow.separatorColor
          }

          Label {
            Layout.preferredWidth: 90
            Layout.alignment: Qt.AlignVCenter
            leftPadding: 8
            opacity: 0.7
            elide: Text.ElideRight
            text: root.verdictBadge(modelData.session_id)
            font: Cpp_Misc_CommonFonts.uiFont
            color: sessionRow.isCurrent
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : sessionRow.textColor
          }

          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            color: sessionRow.separatorColor
          }

          Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            leftPadding: 8
            opacity: 0.7
            elide: Text.ElideRight
            text: (modelData.tag_labels && modelData.tag_labels.length > 0)
                  ? modelData.tag_labels
                  : "--.--"
            font: Cpp_Misc_CommonFonts.uiFont
            color: sessionRow.isCurrent
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : sessionRow.textColor
          }
        }
      }

      Label {
        opacity: 0.5
        anchors.centerIn: parent
        visible: sessionListView.count === 0
        color: Cpp_ThemeManager.colors["text"]
        horizontalAlignment: Text.AlignHCenter
        text: Cpp_Sessions_Manager.isOpen
              ? qsTr("No sessions found.")
              : qsTr("No session file open.")
      }
    }
  }
}
