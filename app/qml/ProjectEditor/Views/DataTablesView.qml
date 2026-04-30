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
    summary = Cpp_JSON_ProjectEditor.tablesSummary()
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
          { title: qsTr("Name"),        width: 220 },
          { title: qsTr("Description"), width: -1  },
          { title: qsTr("Entries"),     width: 100 }
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
              id: rowMouse

              hoverEnabled: true
              anchors.fill: parent
              cursorShape: modelData.isSystem ? Qt.ArrowCursor : Qt.PointingHandCursor
              onClicked: {
                if (!modelData.isSystem)
                  Cpp_JSON_ProjectEditor.selectUserTable(modelData.name)
              }
            }

            RowLayout {
              spacing: 0
              anchors.fill: parent

              Label {
                leftPadding: 8
                text: modelData.name
                elide: Text.ElideRight
                color: rowDel.textColor
                Layout.preferredWidth: 220
                Layout.alignment: Qt.AlignVCenter
                font: Cpp_Misc_CommonFonts.monoFont
              }

              Rectangle {
                implicitWidth: 1
                Layout.fillHeight: true
                color: rowDel.separatorColor
              }

              Label {
                opacity: 0.75
                leftPadding: 8
                Layout.fillWidth: true
                elide: Text.ElideRight
                color: rowDel.textColor
                text: modelData.description
                Layout.alignment: Qt.AlignVCenter
              }

              Rectangle {
                implicitWidth: 1
                Layout.fillHeight: true
                color: rowDel.separatorColor
              }

              Label {
                rightPadding: 8
                color: rowDel.textColor
                Layout.preferredWidth: 100
                text: modelData.entryCount
                Layout.alignment: Qt.AlignVCenter
                font: Cpp_Misc_CommonFonts.monoFont
                horizontalAlignment: Text.AlignRight
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
