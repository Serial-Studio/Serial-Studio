/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  icon: "qrc:/icons/project-editor/treeview/group.svg"

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  property int workspaceId: Cpp_JSON_ProjectEditor.selectedWorkspaceId
  property string workspaceName: Cpp_JSON_ProjectModel.workspaceTitle(workspaceId)
  property string workspaceIcon: Cpp_JSON_ProjectModel.workspaceIcon(workspaceId)
  property var widgets: []

  title: workspaceName.length > 0 ? workspaceName : qsTr("Workspace")

  function refresh() {
    workspaceName = Cpp_JSON_ProjectModel.workspaceTitle(workspaceId)
    workspaceIcon = Cpp_JSON_ProjectModel.workspaceIcon(workspaceId)
    widgets = workspaceId >= 0
              ? Cpp_JSON_ProjectEditor.widgetsForWorkspace(workspaceId)
              : []
  }

  onWorkspaceIdChanged: Qt.callLater(refresh)
  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onEditorWorkspacesChanged() { Qt.callLater(root.refresh) }
    function onGroupsChanged()           { Qt.callLater(root.refresh) }
  }

  //
  // Apply the icon chosen in the shared picker to this workspace.
  //
  Connections {
    target: actionIconPicker

    function onIconSelected(icon) {
      if (Cpp_JSON_ProjectEditor.currentView === ProjectEditor.WorkspaceView)
        Cpp_JSON_ProjectModel.setWorkspaceIcon(root.workspaceId, icon)
    }
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
      // Secondary toolbar: Move Up, Move Down, Change Icon, Rename, Delete
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
            topMargin: 0
            bottomMargin: 0
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Add Widget")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Add widget to workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: addWidgetDialog.open(root.workspaceId)
            icon.source: "qrc:/icons/project-editor/actions/add-dataset.svg"
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Move Up")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Move workspace up")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
                     && root.workspaceId >= 5000
            onClicked: Cpp_JSON_ProjectEditor.moveWorkspace(root.workspaceId, -1)
            icon.source: "qrc:/icons/project-editor/actions/move-up.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Move Down")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Move workspace down")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
                     && root.workspaceId >= 5000
            onClicked: Cpp_JSON_ProjectEditor.moveWorkspace(root.workspaceId, 1)
            icon.source: "qrc:/icons/project-editor/actions/move-down.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Change Icon")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Change workspace icon")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: {
              actionIconPicker.selectedIcon = Cpp_Misc_IconEngine.isInlineSvg(
                root.workspaceIcon) ? "" : root.workspaceIcon
              actionIconPicker.showNormal()
            }
            icon.source: root.workspaceIcon.length > 0
                         ? Cpp_Misc_IconEngine.resolveActionIconSource(root.workspaceIcon)
                         : "qrc:/icons/dashboard-large/workspace.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Rename")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Rename workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.promptRenameWorkspace(root.workspaceId)
            icon.source: "qrc:/icons/project-editor/actions/rename-table.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Delete")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Delete workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.confirmDeleteWorkspace(root.workspaceId)
            icon.source: "qrc:/icons/project-editor/actions/delete-table.svg"
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
      // Widgets list: header + rows via the shared ProjectTable components.
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
        model: root.widgets
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: refRow

          readonly property bool typeKnown: (modelData.widgetTypeName || "").length > 0
          readonly property bool groupResolved: modelData.groupTitle.length > 0
          readonly property bool unknownRow: !refRow.groupResolved && !refRow.typeKnown
          readonly property bool unresolvedRow: !refRow.groupResolved && refRow.typeKnown
          readonly property color rowTextColor: refRow.unknownRow
                                                ? Cpp_ThemeManager.colors["alarm"]
                                                : (refRow.unresolvedRow
                                                   ? Cpp_ThemeManager.colors["error"]
                                                   : refRow.textColor)

          RowLayout {
            spacing: 0
            anchors.fill: parent
            LayoutMirroring.enabled: root.rtl
            LayoutMirroring.childrenInherit: true

            Label {
              Layout.preferredWidth: 220
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              elide: Text.ElideRight
              text: modelData.groupTitle.length > 0
                    ? modelData.groupTitle
                    : qsTr("(unknown)")
              color: refRow.rowTextColor
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
              color: refRow.rowTextColor
              font: Cpp_Misc_CommonFonts.monoFont
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
            }

            Label {
              opacity: 0.7
              elide: Text.ElideRight
              Layout.preferredWidth: 100
              color: refRow.rowTextColor
              leftPadding: root.rtl ? 0 : 8
              rightPadding: root.rtl ? 8 : 0
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.uiFont
              text: modelData.widgetTypeName || qsTr("(unknown)")
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces
            }

            Item {
              Layout.fillHeight: true
              Layout.preferredWidth: 40
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces

              ToolButton {
                id: removeBtn

                padding: 2
                flat: true
                icon.width: 16
                icon.height: 16
                implicitWidth: 32
                hoverEnabled: true
                ToolTip.delay: 400
                implicitHeight: 26
                anchors.centerIn: parent
                ToolTip.visible: hovered
                icon.color: "transparent"
                icon.source: "qrc:/icons/buttons/trash.svg"
                ToolTip.text: qsTr("Remove widget from workspace")

                background: Rectangle {
                  border.width: 0
                  color: "transparent"
                }

                onClicked: Cpp_JSON_ProjectModel.removeWidgetFromWorkspace(
                             root.workspaceId, index)
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
