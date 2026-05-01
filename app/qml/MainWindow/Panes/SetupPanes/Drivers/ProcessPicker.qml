/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  property int titlebarHeight: 0

  //
  // The full "Name [PID]" string selected by the user
  //
  property string selectedProcessName: ""

  //
  // Signal emitted when the user accepts a selection
  //
  signal accepted()

  width: 480
  height: 520
  minimumWidth: 380
  minimumHeight: 400 + titlebarHeight
  title: qsTr("Select Running Process")

  Component.onCompleted: {
    root.flags = Qt.Dialog
        | Qt.CustomizeWindowHint
        | Qt.WindowTitleHint
        | Qt.WindowCloseButtonHint
        | Qt.WindowStaysOnTopHint
  }

  //
  // Native window integration
  //
  onVisibleChanged: {
    if (visible) {
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
      filterField.text = ""
      filterField.forceActiveFocus()
    } else {
      Cpp_NativeWindow.removeWindow(root)
    }

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  //
  // Update window colors when theme changes
  //
  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Top section titlebar background
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["window"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  //
  // Titlebar text
  //
  Label {
    text: root.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Allow dragging the window
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        root.startSystemMove()
    }
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Rebuild the filtered model whenever the source list changes
  //
  Connections {
    target: Cpp_IO_Process
    function onRunningProcessesChanged() { root.rebuildModel() }
  }

  //
  // Rebuild filtered model whenever the filter text changes
  //
  Connections {
    target: filterField
    function onTextChanged() { root.rebuildModel() }
  }

  //
  // Filtered model
  //
  ListModel {
    id: filteredModel
  }

  function rebuildModel() {
    filteredModel.clear()
    const filter = filterField.text.toLowerCase()
    const source = Cpp_IO_Process.runningProcesses
    for (let i = 0; i < source.length; ++i) {
      const entry = source[i]
      if (filter.length === 0 || entry.toLowerCase().indexOf(filter) >= 0)
        filteredModel.append({ "name": entry })
    }
    processList.currentIndex = -1
  }

  Page {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
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

    ColumnLayout {
      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Select a running process to derive a named-pipe path suggestion.")
      }

      Item { implicitHeight: 4 }

      Label {
        text: qsTr("Filter Processes")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        TextField {
          id: filterField

          Layout.fillWidth: true
          placeholderText: qsTr("Type to filter by name…")
        }

        Button {
          text: qsTr("Refresh")
          onClicked: Cpp_IO_Process.refreshProcessList()
        }
      }

      Item { implicitHeight: 4 }

      Label {
        text: qsTr("Running Processes")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      GroupBox {
        Layout.fillWidth: true
        Layout.fillHeight: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ColumnLayout {
          spacing: 0
          anchors.margins: -7
          anchors.fill: parent

          //
          // Header row
          //
          Rectangle {
            height: 32
            Layout.fillWidth: true
            color: palette.alternateBase

            RowLayout {
              spacing: 8
              anchors.fill: parent
              anchors.leftMargin: 8
              anchors.rightMargin: 8

              Label {
                color: palette.text
                text: qsTr("Process")
                Layout.fillWidth: true
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                text: qsTr("PID")
                color: palette.text
                Layout.preferredWidth: 60
                font: Cpp_Misc_CommonFonts.boldUiFont
              }
            }
          }

          Rectangle {
            height: 1
            color: palette.mid
            Layout.fillWidth: true
          }

          ListView {
            id: processList

            clip: true
            currentIndex: -1
            model: filteredModel
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
              height: 32
              width: ListView.view.width
              color: processList.currentIndex === index
                     ? palette.highlight
                     : (index % 2 === 0 ? "transparent" : palette.alternateBase)

              RowLayout {
                spacing: 8
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  color: processList.currentIndex === index
                         ? palette.highlightedText
                         : palette.text
                  font: Cpp_Misc_CommonFonts.monoFont

                  //
                  // Show only the name part before " ["
                  //
                  text: {
                    const s = model.name
                    const bracket = s.lastIndexOf(" [")
                    return bracket >= 0 ? s.substring(0, bracket) : s
                  }
                }

                Label {
                  Layout.preferredWidth: 60
                  horizontalAlignment: Text.AlignRight
                  color: processList.currentIndex === index
                         ? palette.highlightedText
                         : palette.text
                  opacity: 0.7
                  font: Cpp_Misc_CommonFonts.monoFont

                  //
                  // Show only the PID part inside "[ ]"
                  //
                  text: {
                    const s = model.name
                    const m = s.match(/\[(\d+)\]$/)
                    return m ? m[1] : ""
                  }
                }
              }

              MouseArea {
                anchors.fill: parent
                onClicked: processList.currentIndex = index
                onDoubleClicked: {
                  processList.currentIndex = index
                  root.confirmSelection()
                }
              }
            }

            Label {
              opacity: 0.5
              color: palette.text
              anchors.centerIn: parent
              visible: filteredModel.count === 0
              horizontalAlignment: Text.AlignHCenter
              text: filterField.text.length > 0
                    ? qsTr("No processes match the filter.")
                    : qsTr("No running processes found.\nClick Refresh to update the list.")
            }
          }
        }
      }

      Item { implicitHeight: 4 }

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          opacity: 0.7
          color: palette.text
          Layout.fillWidth: true
          text: qsTr("%1 process(es)").arg(filteredModel.count)
        }

        Button {
          text: qsTr("Select")
          onClicked: root.confirmSelection()
          enabled: processList.currentIndex >= 0
        }

        Button {
          text: qsTr("Close")
          onClicked: root.close()
        }
      }
    }
  }

  function confirmSelection() {
    if (processList.currentIndex >= 0
        && processList.currentIndex < filteredModel.count) {
      root.selectedProcessName = filteredModel.get(processList.currentIndex).name
    } else {
      root.selectedProcessName = ""
    }

    root.accepted()
    root.close()
  }
}
