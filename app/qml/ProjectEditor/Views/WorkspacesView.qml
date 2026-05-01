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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  title: qsTr("Workspaces")
  icon: "qrc:/icons/project-editor/treeview/datagrid.svg"

  property var summary: []

  function refresh() {
    summary = Cpp_JSON_ProjectEditor.workspacesSummary()
  }

  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onEditorWorkspacesChanged() { Qt.callLater(root.refresh) }
  }

  Page {
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -10
    }

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      //
      // Secondary toolbar
      //
      Rectangle {
        id: toolbar

        z: 2
        Layout.fillWidth: true
        height: toolbarLayout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        RowLayout {
          id: toolbarLayout

          spacing: 8
          anchors {
            margins: 8
            topMargin: 0
            bottomMargin: 0
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          //
          // Off: synthetic layout read-only. On: editable, persisted workspaces.
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Customize")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Edit workspaces manually")
            checked: Cpp_JSON_ProjectModel.customizeWorkspaces
            icon.source: "qrc:/icons/project-editor/actions/customize.svg"
            onClicked: Cpp_JSON_ProjectModel.customizeWorkspaces =
                       !Cpp_JSON_ProjectModel.customizeWorkspaces
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Add Workspace")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Add workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.promptAddWorkspace()
            icon.source: "qrc:/icons/project-editor/toolbar/add-group.svg"
          }
        }
      }

      //
      // Header + list
      //
      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: [
          { title: qsTr("Title"),   width: 280 },
          { title: qsTr("Widgets"), width: -1  }
        ]
      }

      ListView {
        id: list

        clip: true
        spacing: 0
        model: root.summary
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: wsRow

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: Cpp_JSON_ProjectEditor.selectWorkspace(modelData.id)
          }

          RowLayout {
            spacing: 0
            anchors.fill: parent

            Label {
              leftPadding: 8
              text: modelData.title
              elide: Text.ElideRight
              color: wsRow.textColor
              Layout.preferredWidth: 280
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.uiFont
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: wsRow.separatorColor
            }

            Label {
              leftPadding: 8
              Layout.fillWidth: true
              color: wsRow.textColor
              text: modelData.widgetCount
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
            }
          }
        }

        Label {
          opacity: 0.5
          color: Cpp_ThemeManager.colors["text"]
          anchors.centerIn: parent
          visible: list.count === 0
          horizontalAlignment: Text.AlignHCenter
          text: qsTr("No workspaces.")
        }
      }
    }
  }
}
