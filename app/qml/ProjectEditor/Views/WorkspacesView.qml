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

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  property var summary: []
  property int unresolvedCount: 0

  function refresh() {
    summary = Cpp_JSON_ProjectEditor.workspacesSummary()
    unresolvedCount = Cpp_JSON_ProjectEditor.unresolvedWorkspaceWidgetCount()
  }

  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onEditorWorkspacesChanged() { Qt.callLater(root.refresh) }
    function onGroupsChanged()           { Qt.callLater(root.refresh) }
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
            onClicked: {
              if (Cpp_JSON_ProjectModel.customizeWorkspaces)
                Cpp_JSON_ProjectModel.confirmResetWorkspacesToAuto()
              else
                Cpp_JSON_ProjectModel.customizeWorkspaces = true
            }
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Move Up")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Move the selected workspace up")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
                     && Cpp_JSON_ProjectEditor.selectedWorkspaceId >= 5000
            onClicked: Cpp_JSON_ProjectEditor.moveWorkspace(
                         Cpp_JSON_ProjectEditor.selectedWorkspaceId, -1)
            icon.source: "qrc:/icons/project-editor/actions/move-up.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Move Down")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Move the selected workspace down")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
                     && Cpp_JSON_ProjectEditor.selectedWorkspaceId >= 5000
            onClicked: Cpp_JSON_ProjectEditor.moveWorkspace(
                         Cpp_JSON_ProjectEditor.selectedWorkspaceId, 1)
            icon.source: "qrc:/icons/project-editor/actions/move-down.svg"
          }

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

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Cleanup")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: root.unresolvedCount > 0
                          ? qsTr("Remove %1 widget reference(s) whose target "
                                 + "group or dataset no longer exists")
                            .arg(root.unresolvedCount)
                          : qsTr("No stale widget references in any workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
                     && root.unresolvedCount > 0
            onClicked: Cpp_JSON_ProjectEditor.confirmCleanupUnresolvedWorkspaceWidgets()
            icon.source: "qrc:/icons/project-editor/actions/clear.svg"
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
            LayoutMirroring.enabled: root.rtl
            LayoutMirroring.childrenInherit: true

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

        ColumnLayout {
          spacing: 8
          anchors.centerIn: parent
          visible: list.count === 0

          Label {
            opacity: 0.5
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            text: Cpp_JSON_ProjectModel.customizeWorkspaces
                  ? qsTr("No workspaces. Add one with the toolbar above, "
                         + "or reset to the auto layout.")
                  : qsTr("Project has no eligible groups -- add a group "
                         + "with widgets to populate workspaces.")
          }

          Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Reset to Auto Layout")
            visible: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.confirmResetWorkspacesToAuto()
          }
        }
      }
    }
  }
}
