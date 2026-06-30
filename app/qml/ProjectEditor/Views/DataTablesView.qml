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
  title: qsTr("Shared Memory")
  icon: "qrc:/icons/project-editor/treeview/shared-memory.svg"

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  //
  // Holds the current table summary rows. Refreshed each time the view becomes
  // visible so user-defined tables and system dataset counts stay in sync.
  //
  property var summary: []

  //
  // Refresh the summary whenever the project structure changes, or when the
  // view is shown.
  //
  function refresh() {
    var rows = []
    var sys = Cpp_JSON_ProjectEditor.tablesSummary()
    if (sys.length > 0) {
      rows.push({
                  "isSystem": true,
                  "isFolder": false,
                  "title": sys[0].name,
                  "icon": "qrc:/icons/project-editor/treeview/dataset-values.svg",
                  "count": sys[0].entryCount
                })
    }

    var fc = Cpp_JSON_ProjectEditor.tableFolderContents(-1)
    for (var i = 0; i < fc.length; ++i) {
      fc[i].isSystem = false
      rows.push(fc[i])
    }

    summary = rows
  }

  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onGroupsChanged() { Qt.callLater(root.refresh) }
    function onTablesChanged() { Qt.callLater(root.refresh) }
  }

  //
  // Content
  //
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

        Flickable {
          id: toolbarFlick

          clip: true
          contentHeight: height
          height: toolbarLayout.implicitHeight
          boundsBehavior: Flickable.StopAtBounds
          flickableDirection: Flickable.HorizontalFlick
          contentWidth: toolbarLayout.implicitWidth + 16

          anchors {
            margins: 8
            topMargin: 0
            bottomMargin: 0
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          ScrollBar.horizontal: ScrollBar {
            height: 3
            policy: toolbarFlick.contentWidth > toolbarFlick.width
                    ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
          }

          RowLayout {
            id: toolbarLayout

            spacing: 4
            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(implicitWidth, toolbarFlick.width)

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Add Folder")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Add a top-level folder")
              onClicked: Cpp_JSON_ProjectModel.promptAddTableFolder(-1)
              icon.source: "qrc:/icons/project-editor/actions/add-folder.svg"
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Add Shared Table")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Add shared table")
              onClicked: Cpp_JSON_ProjectModel.promptAddTable()
              icon.source: "qrc:/icons/project-editor/actions/add-table.svg"
            }

            Item { Layout.fillWidth: true }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Help")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              onClicked: app.showHelpCenter("data-tables")
              icon.source: "qrc:/icons/code-editor/help.svg"
              ToolTip.text: qsTr("Open help documentation for shared memory")
            }
          }
        }
      }

      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: [
          { title: qsTr("Title"),     width: 280 },
          { title: qsTr("Registers"), width: -1  }
        ]
      }

      ListView {
        id: tablesList

        clip: true
        spacing: 0
        interactive: true
        model: root.summary
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: rowDel

          MouseArea {
            anchors.fill: parent
            cursorShape: modelData.isSystem ? Qt.ArrowCursor : Qt.PointingHandCursor
            onClicked: {
              if (modelData.isSystem)
                return

              if (modelData.isFolder)
                Cpp_JSON_ProjectEditor.selectTableFolder(modelData.id)
              else
                Cpp_JSON_ProjectEditor.selectUserTable(modelData.path)
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
                  color: rowDel.textColor
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.uiFont
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: rowDel.separatorColor
            }

            Label {
              leftPadding: 8
              text: modelData.count
              Layout.fillWidth: true
              color: rowDel.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
            }
          }
        }

        footer: Item {
          height: 40
          width: ListView.view ? ListView.view.width : 0

          Label {
            anchors.centerIn: parent
            opacity: 0.5
            color: Cpp_ThemeManager.colors["text"]
            visible: root.summary.length <= 1
            text: qsTr("No shared tables.")
          }
        }
      }
    }
  }
}
