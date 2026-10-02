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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ".." as Widgets

// TX command library popover (spec 0091): fills the send field; sending stays explicit
Popup {
  id: root

  signal fillRequested(string text)

  readonly property var lib: Cpp_Console_Handler.sendLibrary
  readonly property var recentItems: {
    const h = lib.history
    const out = []
    for (let i = h.length - 1; i >= 0 && out.length < 8; --i)
      out.push(h[i])

    return out
  }

  function shortText(text) {
    return text.length > 36 ? text.substring(0, 36) + "..." : text
  }

  component RowButton: Widgets.IconButton {
    flat: true
    iconSize: 14
    implicitWidth: 26
    implicitHeight: 26
    Layout.maximumWidth: 26
    opacity: enabled ? 1 : 0.4
    Layout.alignment: Qt.AlignVCenter
  }

  component SectionHeader: RowLayout {
    required property string title

    spacing: 8
    Layout.topMargin: 8
    Layout.fillWidth: true
    Layout.bottomMargin: 2

    Label {
      opacity: 0.7
      text: parent.title
      font: Cpp_Misc_CommonFonts.boldUiFont
    }

    Rectangle {
      height: 1
      opacity: 0.5
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }
  }

  width: 420
  padding: 8
  margins: 8
  transformOrigin: Popup.BottomLeft
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

  enter: Widgets.PopupEnter {}
  exit: Widgets.PopupExit {}

  background: Rectangle {
    radius: 4
    opacity: 0.95
    color: Cpp_ThemeManager.colors["window"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  contentItem: ScrollView {
    id: scroll

    clip: true
    contentWidth: availableWidth
    implicitHeight: Math.min(column.implicitHeight, 360)

    ColumnLayout {
      id: column

      spacing: 2
      width: scroll.availableWidth

      Label {
        padding: 8
        opacity: 0.6
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Sent commands and pinned presets will appear here")
        visible: root.recentItems.length === 0 && root.lib.pins.length === 0
      }

      SectionHeader {
        title: qsTr("Recent")
        visible: root.recentItems.length > 0
      }

      Repeater {
        model: root.recentItems

        RowLayout {
          spacing: 2
          Layout.fillWidth: true

          ItemDelegate {
            implicitHeight: 26
            Layout.fillWidth: true
            text: root.shortText(modelData)
            font: Cpp_Misc_CommonFonts.monoFont
            ToolTip.text: qsTr("Fill the send field with this command")
            onClicked: {
              root.fillRequested(modelData)
              root.close()
            }
          }

          RowButton {
            icon.source: "qrc:/icons/buttons/plus.svg"
            ToolTip.text: qsTr("Pin this command with the current send settings")
            onClicked: root.lib.addPin("", modelData)
          }

          RowButton {
            visible: root.lib.canPromote
            icon.source: "qrc:/icons/buttons/save.svg"
            ToolTip.text: qsTr("Save as project action")
            onClicked: root.lib.promoteCurrent(modelData)
          }
        }
      }

      SectionHeader {
        title: qsTr("Pinned")
        visible: root.lib.pins.length > 0
      }

      Repeater {
        model: root.lib.pins

        RowLayout {
          id: pinRow

          required property var modelData
          required property int index

          property bool renaming: false

          spacing: 2
          Layout.fillWidth: true

          ItemDelegate {
            implicitHeight: 26
            Layout.fillWidth: true
            visible: !pinRow.renaming
            text: root.shortText(pinRow.modelData.title)
            ToolTip.text: qsTr("Fill the send field and restore this pin's send settings")
            onClicked: {
              root.fillRequested(root.lib.recallPin(pinRow.index))
              root.close()
            }
          }

          TextField {
            id: renameField

            implicitHeight: 26
            Layout.fillWidth: true
            visible: pinRow.renaming
            placeholderText: qsTr("Pin name") + "..."
            onVisibleChanged: {
              if (visible) {
                text = pinRow.modelData.title
                selectAll()
                forceActiveFocus()
              }
            }
            onAccepted: {
              root.lib.renamePin(pinRow.index, text)
              pinRow.renaming = false
            }
            Keys.onEscapePressed: pinRow.renaming = false
          }

          RowButton {
            rotation: 180
            enabled: pinRow.index > 0
            ToolTip.text: qsTr("Move up")
            icon.source: "qrc:/icons/buttons/dropdown.svg"
            onClicked: root.lib.movePin(pinRow.index, pinRow.index - 1)
          }

          RowButton {
            ToolTip.text: qsTr("Move down")
            icon.source: "qrc:/icons/buttons/dropdown.svg"
            enabled: pinRow.index < root.lib.pins.length - 1
            onClicked: root.lib.movePin(pinRow.index, pinRow.index + 1)
          }

          RowButton {
            checked: pinRow.renaming
            ToolTip.text: qsTr("Rename pin")
            icon.source: "qrc:/icons/buttons/rename.svg"
            onClicked: pinRow.renaming = !pinRow.renaming
          }

          RowButton {
            visible: root.lib.canPromote
            icon.source: "qrc:/icons/buttons/save.svg"
            ToolTip.text: qsTr("Save as project action")
            onClicked: root.lib.promotePin(pinRow.index)
          }

          RowButton {
            ToolTip.text: qsTr("Delete pin")
            icon.source: "qrc:/icons/buttons/trash.svg"
            onClicked: root.lib.removePin(pinRow.index)
          }
        }
      }

      SectionHeader {
        title: qsTr("Project Actions")
        visible: root.lib.canPromote && Cpp_UI_Dashboard.actionCount > 0
      }

      Repeater {
        model: root.lib.canPromote ? Cpp_UI_Dashboard.actions : null

        ItemDelegate {
          required property var model

          text: model.text
          implicitHeight: 28
          Layout.fillWidth: true
          icon.source: model.icon
          ToolTip.text: qsTr("Trigger this project action")
          enabled: Cpp_IO_Manager.isConnected && !Cpp_IO_Manager.paused
          onClicked: {
            Cpp_UI_Dashboard.activateAction(model.id, true)
            root.close()
          }
        }
      }
    }
  }
}
