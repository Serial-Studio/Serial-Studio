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
import SerialStudio.UI as SS_Ui

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Dialog mode
  //
  property int workspaceId: -1
  property bool editMode: false
  property SS_Ui.TaskBar taskBar: null

  //
  // Internal state
  //
  property string widgetFilter: ""
  property var checkedWidgets: ({})

  //
  // Window options
  //
  staysOnTop: true
  title: editMode ? qsTr("Edit Workspace")
                  : qsTr("New Workspace")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Open dialog in "new" mode
  //
  function openNew(tb) {
    root.taskBar = tb
    root.taskBar.dismissSearch()
    root.editMode = false
    root.workspaceId = -1
    root.checkedWidgets = ({})
    root.widgetFilter = ""
    _nameField.text = ""
    _nameField.forceActiveFocus()
    root.show()
    root.raise()
  }

  //
  // Open dialog in "edit" mode -- pre-populate with workspace widgets
  //
  function openEdit(tb, wsId, currentName) {
    root.taskBar = tb
    root.editMode = true
    root.workspaceId = wsId
    root.widgetFilter = ""
    _nameField.text = currentName

    // Build checked map from existing workspace widget IDs
    var map = {}
    var ids = tb.workspaceWidgetIds(wsId)
    for (var i = 0; i < ids.length; ++i)
      map[ids[i]] = true

    root.checkedWidgets = map
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

    if (root.editMode) {
      //
      // Rename
      //
      root.taskBar.renameWorkspace(root.workspaceId, name)

      //
      // Collect checked window IDs and update workspace contents
      //
      var ids = []
      var keys = Object.keys(root.checkedWidgets)
      for (var i = 0; i < keys.length; ++i) {
        if (root.checkedWidgets[keys[i]])
          ids.push(parseInt(keys[i]))
      }

      root.taskBar.setWorkspaceWidgets(root.workspaceId, ids)
    } else {
      root.taskBar.createWorkspace(name)

      //
      // Add checked widgets to the new workspace
      //
      var newKeys = Object.keys(root.checkedWidgets)
      for (var j = 0; j < newKeys.length; ++j) {
        if (root.checkedWidgets[newKeys[j]])
          root.taskBar.addWidgetToActiveWorkspace(parseInt(newKeys[j]))
      }
    }

    root.close()
  }

  //
  // Dialog content
  //
  dialogContent: ColumnLayout {
    id: layout

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
    // Widget selection
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true

      Label {
        opacity: 0.8
        font: Cpp_Misc_CommonFonts.boldUiFont
        text: qsTr("Select widgets to include:")
      }

      Widgets.SearchField {
        Layout.fillWidth: true
        text: root.widgetFilter
        placeholderText: qsTr("Filter widgets…")
        onTextChanged: root.widgetFilter = text
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
              model: root.taskBar ? root.taskBar.allWidgets : []
              delegate: RowLayout {
                spacing: 0
                Layout.fillWidth: true
                visible: {
                  if (modelData["isWorkspace"])
                    return false

                  if (root.widgetFilter.length === 0)
                    return true

                  var f = root.widgetFilter.toLowerCase()
                  var name = modelData["widgetName"].toLowerCase()
                  var group = modelData["groupName"].toLowerCase()
                  return name.indexOf(f) >= 0 || group.indexOf(f) >= 0
                }

                CheckBox {
                  id: _checkbox

                  checked: {
                    var wid = modelData["windowId"]
                    return root.checkedWidgets[wid] === true
                  }

                  onClicked: {
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
                    onClicked: {
                      _checkbox.checked = !_checkbox.checked
                      _checkbox.clicked()
                    }
                  }
                }

                Item {
                  implicitWidth: 2
                }

                Label {
                  Layout.fillWidth: true
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.uiFont
                  text: modelData["widgetName"]
                        + (modelData["groupName"]
                           ? " (" + modelData["groupName"] + ")"
                           : "")

                  MouseArea {
                    anchors.fill: parent
                    onClicked: {
                      _checkbox.checked = !_checkbox.checked
                      _checkbox.clicked()
                    }
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
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Button {
        leftPadding: 8
        icon.width: 18
        icon.height: 18
        text: qsTr("OK")
        onClicked: root.apply()
        opacity: enabled ? 1 : 0.5
        enabled: _nameField.text.trim().length > 0
        icon.source: "qrc:/icons/buttons/apply.svg"
      }
    }
  }
}
