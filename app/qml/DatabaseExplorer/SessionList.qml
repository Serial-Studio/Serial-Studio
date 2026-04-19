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
  icon: "qrc:/rcc/icons/panes/sessions.svg"

  property string searchText: ""

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

  ColumnLayout {
    spacing: 0
    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -9
    }

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

    Widgets.ProjectTableHeader {
      Layout.fillWidth: true
      columns: [
        { title: qsTr("Date"),   width: 160 },
        { title: qsTr("Frames"), width: 70  },
        { title: qsTr("Tags"),   width: -1  }
      ]
    }

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
            Layout.preferredWidth: 70
            Layout.alignment: Qt.AlignVCenter
            leftPadding: 8
            text: modelData.frame_count || "0"
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
