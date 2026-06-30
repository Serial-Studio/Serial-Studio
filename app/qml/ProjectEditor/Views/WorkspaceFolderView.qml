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
  icon: "qrc:/icons/project-editor/treeview/folder.svg"

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  property int folderId: Cpp_JSON_ProjectEditor.selectedFolderId
  property string folderName: Cpp_JSON_ProjectModel.workspaceFolderTitle(folderId)
  property var contents: []

  title: folderName.length > 0 ? folderName : qsTr("Folder")

  function refresh() {
    folderName = Cpp_JSON_ProjectModel.workspaceFolderTitle(folderId)
    contents = Cpp_JSON_ProjectEditor.workspaceFolderContents(folderId)
  }

  onFolderIdChanged: Qt.callLater(refresh)
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
      // Secondary toolbar: Add Sub-folder, Add Workspace, Rename, Delete
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
            text: qsTr("Add Sub-folder")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            ToolTip.text: qsTr("Add a sub-folder inside this folder")
            onClicked: Cpp_JSON_ProjectModel.promptAddWorkspaceFolder(root.folderId)
            icon.source: "qrc:/icons/project-editor/actions/add-folder.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Add Workspace")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            ToolTip.text: qsTr("Add a workspace inside this folder")
            onClicked: Cpp_JSON_ProjectModel.promptAddWorkspaceInFolder(root.folderId)
            icon.source: "qrc:/icons/project-editor/toolbar/add-group.svg"
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Rename")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Rename folder")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.promptRenameWorkspaceFolder(root.folderId)
            icon.source: "qrc:/icons/project-editor/actions/rename-table.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Delete")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Delete folder")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.confirmDeleteWorkspaceFolder(root.folderId)
            icon.source: "qrc:/icons/project-editor/actions/delete-table.svg"
          }
        }
      }

      //
      // Header + contents list (folders + workspaces inside this folder)
      //
      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: [
          { title: qsTr("Title"),    width: 280 },
          { title: qsTr("Contents"), width: -1  }
        ]
      }

      ListView {
        id: list

        clip: true
        spacing: 0
        model: root.contents
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: itemRow

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              if (modelData.isFolder)
                Cpp_JSON_ProjectEditor.selectWorkspaceFolder(modelData.id)
              else
                Cpp_JSON_ProjectEditor.selectWorkspace(modelData.id)
            }
          }

          RowLayout {
            spacing: 0
            anchors.fill: parent
            LayoutMirroring.enabled: root.rtl
            LayoutMirroring.childrenInherit: true

            Item {
              Layout.fillHeight: true
              Layout.preferredWidth: 280

              RowLayout {
                spacing: 6
                anchors.fill: parent
                anchors.leftMargin: 8

                Image {
                  source: modelData.icon
                  sourceSize: Qt.size(16, 16)
                  Layout.alignment: Qt.AlignVCenter
                }

                Label {
                  text: modelData.title
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                  color: itemRow.textColor
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.uiFont
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: itemRow.separatorColor
            }

            Label {
              leftPadding: 8
              text: modelData.count
              Layout.fillWidth: true
              color: itemRow.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
            }
          }
        }

        Label {
          opacity: 0.5
          width: parent.width * 0.8
          wrapMode: Label.WordWrap
          anchors.centerIn: parent
          visible: list.count === 0
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.uiFont
          horizontalAlignment: Text.AlignHCenter
          text: qsTr("This folder is empty. Use the toolbar to add a workspace or sub-folder.")
        }
      }
    }
  }
}
