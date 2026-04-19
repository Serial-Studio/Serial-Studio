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
  icon: "qrc:/rcc/icons/project-editor/treeview/shared-memory.svg"

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

      //
      // Header — explanatory banner
      //
      Rectangle {
        id: banner

        readonly property int topPad: 12
        readonly property int bottomPad: 12
        readonly property int minHeight: 84

        Layout.topMargin: -1
        Layout.fillWidth: true
        implicitHeight: Math.max(
          minHeight,
          bannerLayout.implicitHeight + topPad + bottomPad
        )
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.top: parent.top
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        RowLayout {
          id: bannerLayout

          spacing: 16
          anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: 14
            rightMargin: 14
            topMargin: banner.topPad
            bottomMargin: banner.bottomPad
          }

          Image {
            Layout.preferredWidth: 56
            Layout.preferredHeight: 56
            Layout.alignment: Qt.AlignVCenter
            fillMode: Image.PreserveAspectFit
            source: "qrc:/rcc/icons/project-editor/summary/shared-memory.svg"
            sourceSize: Qt.size(56, 56)
          }

          ColumnLayout {
            spacing: 4
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            Label {
              text: qsTr("Shared Memory")
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
            }

            Label {
              opacity: 0.65
              Layout.fillWidth: true
              wrapMode: Text.WordWrap
              color: Cpp_ThemeManager.colors["text"]
              text: qsTr("Define constants and computed values shared across all transforms.")
            }
          }
        }
      }

      //
      // Secondary toolbar — matches ActionView/UserTableView style.
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

        Flickable {
          id: toolbarFlick

          clip: true
          contentHeight: height
          boundsBehavior: Flickable.StopAtBounds
          contentWidth: toolbarLayout.implicitWidth + 16
          flickableDirection: Flickable.HorizontalFlick
          height: toolbarLayout.implicitHeight

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            topMargin: 0
            bottomMargin: 0
          }

          ScrollBar.horizontal: ScrollBar {
            height: 3
            policy: toolbarFlick.contentWidth > toolbarFlick.width
                    ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
          }

          RowLayout {
            id: toolbarLayout

            spacing: 4
            width: Math.max(implicitWidth, toolbarFlick.width)
            anchors.verticalCenter: parent.verticalCenter

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Add Shared Table")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Add shared table")
              onClicked: Cpp_JSON_ProjectModel.promptAddTable()
              icon.source: "qrc:/rcc/icons/project-editor/actions/add-table.svg"
            }

            Item { Layout.fillWidth: true }
          }
        }
      }

      //
      // Header — driven by ProjectTableHeader for visual consistency with
      // the rest of the project editor's tables.
      //
      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: [
          { title: qsTr("Name"),        width: 220 },
          { title: qsTr("Description"), width: -1  },
          { title: qsTr("Entries"),     width: 100 }
        ]
      }

      //
      // Table list (ListView flicks natively — no ScrollView wrapper to avoid
      // QQmlComponent recursion warnings)
      //
      ListView {
        id: tablesList

        clip: true
        spacing: 0
        Layout.fillWidth: true
        Layout.fillHeight: true
        model: root.summary
        interactive: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

          delegate: Widgets.ProjectTableRow {
            id: rowDel

            MouseArea {
              id: rowMouse
              anchors.fill: parent
              hoverEnabled: true
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
                Layout.preferredWidth: 220
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 8
                elide: Text.ElideRight
                text: modelData.name
                color: rowDel.textColor
                font: Cpp_Misc_CommonFonts.monoFont
              }

              Rectangle {
                implicitWidth: 1
                Layout.fillHeight: true
                color: rowDel.separatorColor
              }

              Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 8
                elide: Text.ElideRight
                opacity: 0.75
                text: modelData.description
                color: rowDel.textColor
              }

              Rectangle {
                implicitWidth: 1
                Layout.fillHeight: true
                color: rowDel.separatorColor
              }

              Label {
                Layout.preferredWidth: 100
                Layout.alignment: Qt.AlignVCenter
                rightPadding: 8
                horizontalAlignment: Text.AlignRight
                text: modelData.entryCount
                font: Cpp_Misc_CommonFonts.monoFont
                color: rowDel.textColor
              }
            }
          }

          //
          // Empty state (still shown alongside system table row)
          //
          footer: Item {
            width: ListView.view ? ListView.view.width : 0
            height: 40

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
