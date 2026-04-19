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
  icon: "qrc:/rcc/icons/project-editor/treeview/group.svg"

  property int workspaceId: Cpp_JSON_ProjectEditor.selectedWorkspaceId
  property string workspaceName: Cpp_JSON_ProjectModel.workspaceTitle(workspaceId)
  property var widgets: []

  title: workspaceName.length > 0 ? workspaceName : qsTr("Workspace")

  function refresh() {
    workspaceName = Cpp_JSON_ProjectModel.workspaceTitle(workspaceId)
    widgets = workspaceId >= 0
              ? Cpp_JSON_ProjectEditor.widgetsForWorkspace(workspaceId)
              : []
  }

  onWorkspaceIdChanged: Qt.callLater(refresh)
  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onWorkspacesChanged() { Qt.callLater(root.refresh) }
    function onGroupsChanged()     { Qt.callLater(root.refresh) }
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
      // Secondary toolbar — Rename, Delete
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
          spacing: 4
          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            topMargin: 0
            bottomMargin: 0
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Add Widget")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Add widget to workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: addWidgetDialog.open(root.workspaceId)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-dataset.svg"
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Rename")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Rename workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.promptRenameWorkspace(root.workspaceId)
            icon.source: "qrc:/rcc/icons/project-editor/actions/rename-table.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Delete")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Delete workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.confirmDeleteWorkspace(root.workspaceId)
            icon.source: "qrc:/rcc/icons/project-editor/actions/delete-table.svg"
          }
        }
      }

      //
      // Picker window for the "Add Widget" toolbar button.
      //
      AddWidgetDialog {
        id: addWidgetDialog
      }

      //
      // Widgets list — header + rows via the shared ProjectTable components.
      //
      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: Cpp_JSON_ProjectModel.customizeWorkspaces
                 ? [
                     { title: qsTr("Group"),   width: 220 },
                     { title: qsTr("Widget"),  width: -1  },
                     { title: qsTr("Type"),    width: 100 },
                     { title: "",              width: 40  }
                   ]
                 : [
                     { title: qsTr("Group"),   width: 220 },
                     { title: qsTr("Widget"),  width: -1  },
                     { title: qsTr("Type"),    width: 100 }
                   ]
      }

      ListView {
        id: list
        clip: true
        spacing: 0
        Layout.fillWidth: true
        Layout.fillHeight: true
        model: root.widgets
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: refRow

          RowLayout {
            spacing: 0
            anchors.fill: parent

            Label {
              Layout.preferredWidth: 220
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              elide: Text.ElideRight
              text: modelData.groupTitle.length > 0
                    ? modelData.groupTitle
                    : qsTr("(unknown)")
              color: refRow.textColor
              font: Cpp_Misc_CommonFonts.monoFont
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
            }

            Label {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              elide: Text.ElideRight
              text: modelData.datasetTitle.length > 0
                    ? modelData.datasetTitle
                    : qsTr("(group widget)")
              color: refRow.textColor
              font: Cpp_Misc_CommonFonts.monoFont
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
            }

            Label {
              Layout.preferredWidth: 100
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              opacity: 0.7
              text: modelData.widgetTypeName || ""
              color: refRow.textColor
              font: Cpp_Misc_CommonFonts.uiFont
              elide: Text.ElideRight
            }

            Rectangle {
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces
              implicitWidth: visible ? 1 : 0
              Layout.preferredWidth: visible ? 1 : 0
              Layout.fillHeight: true
              color: refRow.separatorColor
            }

            Item {
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces
              Layout.preferredWidth: visible ? 40 : 0
              Layout.fillHeight: true

              ToolButton {
                id: removeBtn
                anchors.centerIn: parent
                width: 40
                height: 26
                padding: 2
                flat: true
                hoverEnabled: true
                icon.width: 16
                icon.height: 16
                icon.source: "qrc:/rcc/icons/buttons/trash.svg"
                icon.color: "transparent"

                background: Rectangle {
                  color: "transparent"
                  border.width: 0
                }

                ToolTip.visible: hovered
                ToolTip.delay: 400
                ToolTip.text: qsTr("Remove from workspace")

                onClicked: Cpp_JSON_ProjectModel.removeWidgetFromWorkspace(
                  root.workspaceId,
                  modelData.widgetType,
                  modelData.groupId,
                  modelData.relativeIndex
                )
              }
            }
          }
        }

        Label {
          opacity: 0.5
          color: Cpp_ThemeManager.colors["text"]
          anchors.centerIn: parent
          visible: list.count === 0
          horizontalAlignment: Text.AlignHCenter
          text: qsTr("No widgets in this workspace.")
        }
      }
    }
  }
}
