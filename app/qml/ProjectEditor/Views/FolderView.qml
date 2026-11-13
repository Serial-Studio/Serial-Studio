/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../../Widgets" as Widgets

// Folder view layout: strings arrive as properties (translation context), actions leave as signals.
Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_Misc_IconRegistry.icon("widgets", "folder", 16)

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  property int folderId: 0
  property string folderName: ""
  property var contents: []
  readonly property var filteredContents: {
    const q = searchBand.query.trim()
    if (q.length === 0)
      return contents

    return contents.filter((r) => SerialStudioHelpers.searchMatches(q, String(r.title)))
  }

  property bool editable: true
  property string addItemIcon: "group"

  property string emptyText: ""
  property string renameText: ""
  property string deleteText: ""
  property string titleColumn: ""
  property string countColumn: ""
  property string addItemText: ""
  property string fallbackTitle: ""
  property string addFolderText: ""
  property string renameToolTip: ""
  property string deleteToolTip: ""
  property string addItemToolTip: ""
  property string addFolderToolTip: ""

  signal refreshRequested()
  signal addFolderClicked()
  signal addItemClicked()
  signal renameClicked()
  signal deleteClicked()
  signal entryClicked(var entry)

  title: folderName.length > 0 ? folderName : fallbackTitle

  actionComponent: EditorNavActions {}

  function refresh() {
    refreshRequested()
  }

  onFolderIdChanged: {
    searchBand.query = ""
    Qt.callLater(refresh)
  }
  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Widgets.ThemedPage {
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
      // Search band: above the secondary toolbar, aligned with the tree search
      //
      EditorSearchBand {
        id: searchBand

        resultCount: list.count
      }

      //
      // Secondary toolbar: Add Sub-folder, Add item, Rename, Delete
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
            enabled: root.editable
            text: root.addFolderText
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: root.addFolderToolTip
            onClicked: root.addFolderClicked()
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "add-folder", 24)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            enabled: root.editable
            text: root.addItemText
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: root.addItemToolTip
            onClicked: root.addItemClicked()
            icon.source: Cpp_Misc_IconRegistry.icon("editor", root.addItemIcon, 24)
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: root.renameText
            enabled: root.editable
            onClicked: root.renameClicked()
            ToolTip.text: root.renameToolTip
            Layout.alignment: Qt.AlignVCenter
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "rename-table", 24)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: root.deleteText
            enabled: root.editable
            onClicked: root.deleteClicked()
            ToolTip.text: root.deleteToolTip
            Layout.alignment: Qt.AlignVCenter
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "delete-table", 24)
          }
        }
      }

      //
      // Header + contents list (sub-folders and items inside this folder)
      //
      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: [
          { title: root.titleColumn, width: 280 },
          { title: root.countColumn, width: -1 }
        ]
      }

      ListView {
        id: list

        clip: true
        spacing: 0
        Layout.fillWidth: true
        Layout.fillHeight: true
        model: root.filteredContents
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: itemRow

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.entryClicked(modelData)
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
          visible: list.count === 0 && searchBand.query.trim().length === 0
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.uiFont
          horizontalAlignment: Text.AlignHCenter
          text: root.emptyText
        }
      }
    }
  }
}
