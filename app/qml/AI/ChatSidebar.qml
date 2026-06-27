/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
  id: root

  color: Cpp_ThemeManager.colors["window"]

  //
  // Id of the chat currently being renamed (drives the rename popup)
  //
  property string renameId: ""

  ColumnLayout {
    spacing: 8
    anchors.margins: 8
    anchors.fill: parent

    //
    // Header: title + new-chat button
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Label {
        text: qsTr("Chats")
        Layout.fillWidth: true
        elide: Label.ElideRight
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["text"]
      }

      ToolButton {
        display: AbstractButton.IconOnly
        Layout.preferredWidth: 26
        Layout.preferredHeight: 26
        ToolTip.delay: 400
        ToolTip.visible: hovered
        ToolTip.text: qsTr("New chat")
        icon.width: 14
        icon.height: 14
        icon.color: Cpp_ThemeManager.colors["text"]
        icon.source: "qrc:/icons/buttons/plus.svg"
        onClicked: Cpp_AI_Assistant.newChat()
      }
    }

    //
    // Header separator
    //
    Rectangle {
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    //
    // Chat list (most-recent first; bound to the C++ chat catalog)
    //
    ListView {
      id: list

      clip: true
      spacing: 2
      Layout.fillWidth: true
      Layout.fillHeight: true
      model: Cpp_AI_Assistant.chatList
      boundsBehavior: Flickable.StopAtBounds
      ScrollBar.vertical: ScrollBar {}

      delegate: ItemDelegate {
        id: del

        height: 46
        width: ListView.view.width

        required property var modelData
        required property int index

        readonly property bool active: modelData.id === Cpp_AI_Assistant.activeChatId

        background: Rectangle {
          radius: 6
          color: del.active
                 ? Cpp_ThemeManager.colors["highlight"]
                 : (del.hovered ? Cpp_ThemeManager.colors["alternate_base"] : "transparent")
        }

        contentItem: ColumnLayout {
          spacing: 0

          Label {
            Layout.fillWidth: true
            elide: Label.ElideRight
            font: Cpp_Misc_CommonFonts.uiFont
            text: (del.modelData.title && del.modelData.title.length > 0)
                  ? del.modelData.title : qsTr("New chat")
            color: del.active
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : Cpp_ThemeManager.colors["text"]
          }

          Label {
            opacity: 0.6
            Layout.fillWidth: true
            elide: Label.ElideRight
            text: qsTr("%1 messages").arg(del.modelData.count || 0)
            font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
            color: del.active
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : Cpp_ThemeManager.colors["text"]
          }
        }

        onClicked: Cpp_AI_Assistant.switchChat(del.modelData.id)

        //
        // Right-click opens the per-chat actions menu
        //
        TapHandler {
          acceptedButtons: Qt.RightButton
          onTapped: ctxMenu.popup()
        }

        Menu {
          id: ctxMenu

          MenuItem {
            text: qsTr("Rename...")
            onTriggered: {
              root.renameId  = del.modelData.id
              renameField.text = del.modelData.title || ""
              renamePopup.open()
              renameField.selectAll()
              renameField.forceActiveFocus()
            }
          }

          MenuItem {
            text: qsTr("Delete")
            onTriggered: Cpp_AI_Assistant.deleteChat(del.modelData.id)
          }
        }
      }
    }
  }

  //
  // Rename popup (centered over the window overlay)
  //
  Popup {
    id: renamePopup

    width: 280
    modal: true
    padding: 12
    anchors.centerIn: Overlay.overlay

    function commit() {
      const t = renameField.text.trim()
      if (root.renameId.length > 0 && t.length > 0)
        Cpp_AI_Assistant.renameChat(root.renameId, t)

      renamePopup.close()
    }

    background: Rectangle {
      radius: 8
      border.width: 1
      color: Cpp_ThemeManager.colors["base"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    ColumnLayout {
      spacing: 8
      anchors.fill: parent

      Label {
        text: qsTr("Rename chat")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["text"]
      }

      TextField {
        id: renameField

        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.uiFont
        onAccepted: renamePopup.commit()
      }

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Item { Layout.fillWidth: true }

        Button {
          text: qsTr("Cancel")
          onClicked: renamePopup.close()
        }

        Button {
          text: qsTr("Rename")
          onClicked: renamePopup.commit()
        }
      }
    }
  }
}
