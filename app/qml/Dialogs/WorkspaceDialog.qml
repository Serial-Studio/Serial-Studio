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

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Dialog mode
  //
  property int workspaceId: -1
  property bool renameMode: false
  property SS_Ui.TaskBar taskBar: null

  //
  // Internal state
  //
  property var checkedWidgets: ({})

  //
  // Window options
  //
  staysOnTop: true
  title: renameMode ? qsTr("Rename Workspace")
                    : qsTr("New Workspace")

  //
  // Open dialog in "new" mode
  //
  function openNew(tb) {
    root.taskBar = tb
    root.taskBar.dismissSearch()
    root.renameMode = false
    root.workspaceId = -1
    root.checkedWidgets = ({})
    _nameField.text = ""
    _nameField.forceActiveFocus()
    root.show()
    root.raise()
  }

  //
  // Open dialog in "rename" mode
  //
  function openRename(tb, wsId, currentName) {
    root.taskBar = tb
    root.renameMode = true
    root.workspaceId = wsId
    root.checkedWidgets = ({})
    _nameField.text = currentName
    _nameField.selectAll()
    _nameField.forceActiveFocus()
    root.show()
    root.raise()
  }

  //
  // Apply changes
  //
  function apply() {
    var name = _nameField.text.trim()
    if (name.length === 0 || !root.taskBar)
      return

    if (root.renameMode) {
      root.taskBar.renameWorkspace(root.workspaceId, name)
    } else {
      root.taskBar.createWorkspace(name)

      // Add checked widgets to the new workspace
      var keys = Object.keys(root.checkedWidgets)
      for (var i = 0; i < keys.length; ++i) {
        if (root.checkedWidgets[keys[i]])
          root.taskBar.addWidgetToActiveWorkspace(parseInt(keys[i]))
      }
    }

    root.close()
  }

  //
  // Dialog content
  //
  dialogContent: ColumnLayout {
    spacing: 8

    //
    // Workspace name
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.minimumWidth: 480

      Label {
        text: qsTr("Name:")
        font: Cpp_Misc_CommonFonts.boldUiFont
      }

      TextField {
        id: _nameField

        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.uiFont
        placeholderText: qsTr("My Workspace")
      }
    }

    //
    // Widget selection (only for new workspace mode)
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true
      visible: !root.renameMode

      Label {
        opacity: 0.8
        text: qsTr("Select widgets to include:")
        font: Cpp_Misc_CommonFonts.boldUiFont
      }

      Rectangle {
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 200
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ScrollView {
          clip: true
          anchors.margins: 4
          anchors.fill: parent
          ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

          ColumnLayout {
            spacing: 2
            width: parent.width

            Repeater {
              model: root.taskBar ? root.taskBar.searchResults : []
              delegate: RowLayout {
                spacing: 0
                Layout.fillWidth: true
                visible: !modelData["isWorkspace"]

                CheckBox {
                  id: _checkbox
                  onCheckedChanged: {
                    var wid = modelData["windowId"]
                    var map = root.checkedWidgets
                    map[wid] = checked
                    root.checkedWidgets = map
                  }
                }

                Item {
                  implicitWidth: 2
                }

                Image {
                  sourceSize: Qt.size(16, 16)
                  source: modelData["widgetIcon"]
                  Layout.alignment: Qt.AlignVCenter

                  MouseArea {
                    anchors.fill: parent
                    onClicked: _checkbox.checked = !_checkbox.checked
                  }
                }

                Item {
                  implicitWidth: 2
                }

                Label {
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.uiFont
                  Layout.alignment: Qt.AlignVCenter
                  text: modelData["widgetName"]
                        + (modelData["groupName"]
                           ? " (" + modelData["groupName"] + ")"
                           : "")

                  MouseArea {
                    anchors.fill: parent
                    onClicked: _checkbox.checked = !_checkbox.checked
                  }
                }
              }
            }
          }
        }
      }
    }

    //
    // Buttons
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Item {
        Layout.fillWidth: true
      }

      Button {
        leftPadding: 8
        icon.width: 18
        icon.height: 18
        text: qsTr("Cancel")
        onClicked: root.close()
        icon.source: "qrc:/rcc/icons/buttons/close.svg"
      }

      Button {
        leftPadding: 8
        icon.width: 18
        icon.height: 18
        text: qsTr("OK")
        onClicked: root.apply()
        opacity: enabled ? 1 : 0.5
        enabled: _nameField.text.trim().length > 0
        icon.source: "qrc:/rcc/icons/buttons/apply.svg"
      }
    }
  }
}
