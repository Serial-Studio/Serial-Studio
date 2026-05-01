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

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  title: qsTr("Dataset Values")
  icon: "qrc:/icons/project-editor/treeview/dataset-values.svg"

  //
  // Shared column widths and row height -- header, rows, and empty-column
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
  // Single search string -- matches any column (case-insensitive OR).
  //
  property string searchText: ""

  //
  // Pad uniqueIds to the largest entry's digit count (min 5); display only.
  //
  readonly property int idPadWidth: {
    let max = 0
    for (let i = 0; i < datasets.length; ++i) {
      const v = Number(datasets[i].uniqueId)
      if (v > max) max = v
    }
    return Math.max(5, String(max).length)
  }

  function paddedId(id) {
    return String(id).padStart(idPadWidth, '0')
  }

  //
  // Filtered view -- recomputed whenever the search text or dataset list changes.
  //
  property var filteredDatasets: {
    const q = String(searchText || "").toLowerCase().trim()
    if (!q)
      return datasets

    const match = (s) => String(s || "").toLowerCase().indexOf(q) !== -1
    return datasets.filter(d =>
                           match(d.uniqueId) ||
                           match(paddedId(d.uniqueId)) ||
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

      Rectangle {
        z: 2
        Layout.fillWidth: true
        implicitHeight: 48
        Layout.topMargin: -1
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Widgets.SearchField {
          id: searchField

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          text: root.searchText
          color: Cpp_ThemeManager.colors["base"]
          placeholderText: qsTr("Search")
          onTextChanged: root.searchText = text

          anchors {
            leftMargin: 6
            rightMargin: 6
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }
        }
      }

      //
      // Column headers -- driven by the shared ProjectTableHeader component.
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
              color: rowDelegate.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              Layout.preferredWidth: root.colIdWidth
              text: root.paddedId(modelData.uniqueId)
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
            // Copy-access-code button
            //
            ToolButton {
              id: copyBtn

              readonly property string accessCode: "datasetGetFinal(" + modelData.uniqueId + ")"

              padding: 2
              flat: true
              icon.width: 16
              icon.height: 16
              ToolTip.delay: 400
              hoverEnabled: true
              ToolTip.visible: hovered
              icon.color: "transparent"
              Layout.preferredHeight: 26
              Layout.preferredWidth: root.colActionWidth
              icon.source: "qrc:/icons/buttons/copy.svg"
              ToolTip.text: qsTr("Copy access code %1 to clipboard").arg(copyBtn.accessCode)

              background: Rectangle {
                border.width: 0
                color: "transparent"
              }

              onClicked: {
                Cpp_Misc_Utilities.copyText(copyBtn.accessCode)
                copyToast.show()
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

    //
    // Floating "Dataset access code copied" toast.
    //
    Rectangle {
      id: copyToast

      z: 1000
      opacity: 0
      radius: 4
      anchors.bottom: parent.bottom
      anchors.bottomMargin: 24
      anchors.horizontalCenter: parent.horizontalCenter
      width: copyToastLabel.implicitWidth + 24
      height: copyToastLabel.implicitHeight + 12
      color: Cpp_ThemeManager.colors["highlight"]
      visible: opacity > 0

      function show() {
        copyToast.opacity = 1
        copyToastTimer.restart()
      }

      RowLayout {
        id: copyToastLabel

        spacing: 8
        anchors.centerIn: parent

        ToolButton {
          flat: true
          padding: 0
          enabled: false
          icon.width: 14
          icon.height: 14
          Layout.preferredWidth: 14
          Layout.preferredHeight: 14
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/icons/buttons/apply.svg"
          icon.color: Cpp_ThemeManager.colors["highlighted_text"]
          background: Item {}
        }

        Label {
          Layout.alignment: Qt.AlignVCenter
          text: qsTr("Dataset access code copied")
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["highlighted_text"]
        }
      }

      Timer {
        id: copyToastTimer

        repeat: false
        interval: 1500
        onTriggered: copyToast.opacity = 0
      }

      Behavior on opacity {
        NumberAnimation { duration: 150 }
      }
    }
  }
}
