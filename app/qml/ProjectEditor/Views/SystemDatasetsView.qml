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
  title: qsTr("Dataset Values")
  icon: "qrc:/rcc/icons/project-editor/treeview/dataset-values.svg"

  //
  // Shared column widths and row height — header, rows, and empty-column
  // spacers stay aligned. Matches TableDelegate's 30-px row height.
  //
  readonly property int rowHeight: 30
  readonly property int colIdWidth: 80
  readonly property int colGroupWidth: 200
  readonly property int colUnitsWidth: 120
  readonly property int colActionWidth: 40

  //
  // Dataset rows obtained from ProjectEditor.systemDatasetsSummary().
  //
  property var datasets: []

  //
  // Single search string — matches any column (case-insensitive OR).
  //
  property string searchText: ""

  //
  // Filtered view — recomputed whenever the search text or dataset list changes.
  //
  property var filteredDatasets: {
    const q = String(searchText || "").toLowerCase().trim()
    if (!q)
      return datasets

    const match = (s) => String(s || "").toLowerCase().indexOf(q) !== -1
    return datasets.filter(d =>
                           match(d.uniqueId) ||
                           match(d.groupTitle) ||
                           match(d.title) ||
                           match(d.units)
                           )
  }

  function refresh() {
    datasets = Cpp_JSON_ProjectEditor.systemDatasetsSummary()
  }

  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onGroupsChanged() { Qt.callLater(root.refresh) }
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
      // Banner — matches UserTableView / DataTablesView: icon on the left,
      // title + description, match count on the right.
      //
      Rectangle {
        id: banner

        readonly property int topPad: 12
        readonly property int bottomPad: 12
        readonly property int minHeight: 84

        z: 2
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
            leftMargin: 14
            rightMargin: 14
            left: parent.left
            right: parent.right
            topMargin: banner.topPad
            bottomMargin: banner.bottomPad
            verticalCenter: parent.verticalCenter
          }

          Image {
            Layout.preferredWidth: 56
            Layout.preferredHeight: 56
            sourceSize: Qt.size(56, 56)
            Layout.alignment: Qt.AlignVCenter
            fillMode: Image.PreserveAspectFit
            source: "qrc:/rcc/icons/project-editor/summary/dataset-values.svg"
          }

          ColumnLayout {
            spacing: 4
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            Label {
              text: qsTr("Dataset Values")
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
            }

            Label {
              opacity: 0.65
              Layout.fillWidth: true
              wrapMode: Text.WordWrap
              color: Cpp_ThemeManager.colors["text"]
              text: qsTr("All datasets in the project, available for cross-referencing in transforms.")
            }
          }

          Label {
            Layout.alignment: Qt.AlignVCenter
            opacity: 0.6
            color: Cpp_ThemeManager.colors["text"]
            text: root.datasets.length === root.filteredDatasets.length ? qsTr("%1 datasets").arg(root.datasets.length)
                                                                        : qsTr("%1 of %2 datasets").arg(root.filteredDatasets.length).arg(root.datasets.length)
          }
        }
      }

      //
      // Search bar — full-width row beneath the banner, same 36-px height as
      // the secondary toolbars in UserTableView / DataTablesView.
      //
      Rectangle {
        z: 2
        Layout.fillWidth: true
        height: searchBarLayout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        RowLayout {
          id: searchBarLayout

          spacing: 8
          anchors {
            margins: 8
            topMargin: 0
            bottomMargin: 0
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Widgets.SearchField {
            id: searchField
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            text: root.searchText
            color: Cpp_ThemeManager.colors["base"]
            placeholderText: qsTr("Search")
            onTextChanged: root.searchText = text
          }
        }
      }

      //
      // Column headers — driven by the shared ProjectTableHeader component.
      //
      Widgets.ProjectTableHeader {
        z: 2
        Layout.fillWidth: true
        rowHeight: root.rowHeight
        columns: [
          { title: qsTr("ID"),      width: root.colIdWidth     },
          { title: qsTr("Group"),   width: root.colGroupWidth  },
          { title: qsTr("Dataset"), width: -1                  },
          { title: qsTr("Units"),   width: root.colUnitsWidth  },
          { title: "",              width: root.colActionWidth }
        ]
      }

      //
      // Dataset list
      //
      ListView {
        id: datasetsList

        clip: true
        spacing: 0
        interactive: true
        Layout.fillWidth: true
        Layout.fillHeight: true
        model: root.filteredDatasets
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: rowDelegate
          rowHeight: root.rowHeight

          RowLayout {
            spacing: 0
            anchors.fill: parent

            Label {
              leftPadding: 8
              text: modelData.uniqueId
              color: rowDelegate.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              Layout.preferredWidth: root.colIdWidth
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: rowDelegate.separatorColor
            }

            Label {
              leftPadding: 8
              elide: Text.ElideRight
              text: modelData.groupTitle
              color: rowDelegate.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              Layout.preferredWidth: root.colGroupWidth
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: rowDelegate.separatorColor
            }

            Label {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              elide: Text.ElideRight
              font: Cpp_Misc_CommonFonts.monoFont
              color: rowDelegate.textColor
              text: modelData.isVirtual ? modelData.title + " " + qsTr("(virtual)")
                                        : modelData.title
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: rowDelegate.separatorColor
            }

            Label {
              opacity: 0.75
              leftPadding: 8
              text: modelData.units
              elide: Text.ElideRight
              color: rowDelegate.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              Layout.preferredWidth: root.colUnitsWidth
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: rowDelegate.separatorColor
            }

            //
            // Copy-ID button
            //
            ToolButton {
              id: copyBtn

              property bool justCopied: false

              padding: 2
              flat: true
              icon.width: 16
              icon.height: 16
              ToolTip.delay: 400
              hoverEnabled: true
              Layout.preferredHeight: 26
              ToolTip.visible: hovered && !justCopied
              Layout.preferredWidth: root.colActionWidth
              ToolTip.text: qsTr("Copy ID %1 to clipboard").arg(modelData.uniqueId)
              icon.source: copyBtn.justCopied ? "qrc:/rcc/icons/buttons/apply.svg"
                                              : "qrc:/rcc/icons/buttons/copy.svg"
              icon.color: "transparent"

              background: Rectangle {
                border.width: 0
                color: "transparent"
              }

              Timer {
                id: resetCheck
                repeat: false
                interval: 1500
                onTriggered: copyBtn.justCopied = false
              }

              onClicked: {
                Cpp_Misc_Utilities.copyText(String(modelData.uniqueId))
                copyBtn.justCopied = true
                resetCheck.restart()
              }
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
            visible: root.filteredDatasets.length === 0
            text: root.datasets.length === 0
                  ? qsTr("No datasets defined in this project.")
                  : qsTr("No datasets match your search.")
          }
        }
      }
    }
  }
}
