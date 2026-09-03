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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets
import "AssistantPanel" as AssistantPanel

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  fixedSize: false
  title: qsTr("Assistant")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Allow resizing and maximizing; the chat needs vertical room.
  //
  width: 1040
  height: 680
  minimumWidth: 900
  minimumHeight: 560
  maximumWidth: 10000
  maximumHeight: 10000

  //
  // Wire Cpp_AI_Assistant requestKeyManager to the dialog
  //
  Connections {
    target: Cpp_AI_Assistant
    function onRequestKeyManager() { keyManager.activate() }
  }

  //
  // Controls visibility of the left chat-list sidebar
  //
  property bool sidebarVisible: true

  //
  // Dialog content (SmartDialog handles native window integration,
  // titlebar drag, and theme palette automatically)
  //
  dialogContent: SplitView {
    orientation: Qt.Horizontal

    //
    // Transparent handle acts as a spacer between the sidebar and the chat.
    //
    handle: Item {
      implicitWidth: root.sidebarVisible ? 12 : 0
    }

    ChatSidebar {
      SplitView.fillHeight: true
      SplitView.minimumWidth: 200
      visible: root.sidebarVisible
      SplitView.preferredWidth: 240
    }

    ColumnLayout {
      id: layout

      spacing: 12
      SplitView.fillWidth: true
      SplitView.fillHeight: true

        //
        // Top bar: chat title on the left, provider/model/settings on the right.
        //
        AssistantPanel.AssistantTopBar {
          id: topBarSection

          sidebarVisible: root.sidebarVisible
          onSidebarToggled: root.sidebarVisible = !root.sidebarVisible
          onMemoryManagerRequested: memoryManager.activate()
        }

        //
        // Conversation area: centered welcome card when empty, framed list once
        // the user has sent a first message.
        //
        AssistantPanel.AssistantConversation {
          id: conversationSection

          onComposerClearRequested: composerSection.clear()
        }

        //
        // Persistent context-degradation banner. Advisory chrome only: it never
        // disables or delays the composer below it.
        //
        Rectangle {
          id: degradedBanner

          radius: 8
          border.width: 1
          Layout.fillWidth: true
          visible: Cpp_AI_Assistant.contextDegraded
          implicitHeight: visible ? degradedRow.implicitHeight + 16 : 0
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["alarm"]

          RowLayout {
            id: degradedRow

            spacing: 8
            anchors.margins: 8
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            ColumnLayout {
              spacing: 2
              Layout.fillWidth: true

              Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: qsTr("Context may be degraded")
                color: Cpp_ThemeManager.colors["alarm"]
                font: Cpp_Misc_CommonFonts.customUiFont(1, true)
              }

              Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                textFormat: Text.PlainText
                font: Cpp_Misc_CommonFonts.uiFont
                color: Cpp_ThemeManager.colors["text"]
                text: Cpp_AI_Assistant.degradationDetail
              }
            }

            Button {
              text: qsTr("Start fresh chat")
              onClicked: Cpp_AI_Assistant.newChatFromHandoff(Cpp_AI_Assistant.activeChatId)
            }
          }
        }

        //
        // Memory-proposal chip: the assistant asked to remember something; nothing
        // persists unless the user clicks Remember.
        //
        Rectangle {
          id: memoryProposalChip

          radius: 8
          visible: false
          border.width: 1
          Layout.fillWidth: true
          implicitHeight: visible ? proposalRow.implicitHeight + 16 : 0
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["highlight"]

          property string category: ""
          property string factText: ""

          Connections {
            target: Cpp_AI_Assistant
            function onMemoryProposed(category, text) {
              memoryProposalChip.category = category
              memoryProposalChip.factText = text
              memoryProposalChip.visible = true
            }
            function onActiveChatChanged() {
              memoryProposalChip.visible = false
            }
          }

          RowLayout {
            id: proposalRow

            spacing: 8
            anchors.margins: 8
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            Label {
              Layout.fillWidth: true
              wrapMode: Text.WordWrap
              textFormat: Text.PlainText
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["text"]
              text: qsTr("Remember (%1)?").arg(memoryProposalChip.category)
                    + " \"" + memoryProposalChip.factText + "\""
            }

            Button {
              text: qsTr("Remember")
              onClicked: {
                if (Cpp_AI_Assistant.addMemory(memoryProposalChip.category,
                                               memoryProposalChip.factText))
                  memoryProposalChip.visible = false
              }
            }

            Button {
              text: qsTr("Dismiss")
              onClicked: memoryProposalChip.visible = false
            }
          }
        }

        //
        // Handoff chip: advisory-only marker that this chat was seeded with the
        // previous chat's context, so a successful handoff is visible.
        //
        Rectangle {
          id: handoffSeedChip

          radius: 8
          border.width: 1
          Layout.fillWidth: true
          visible: Cpp_AI_Assistant.activeChatSeeded
          implicitHeight: visible ? handoffSeedLabel.implicitHeight + 16 : 0
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["highlight"]

          Label {
            id: handoffSeedLabel

            anchors.margins: 8
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Continuing from your previous chat. Its recent context was carried over.")
          }
        }

        //
        // Indeterminate progress stripe shown while the assistant is working
        //
        Item {
          id: workingStripe

          clip: true
          Layout.fillWidth: true
          Layout.preferredHeight: 2
          visible: Cpp_AI_Assistant.busy

          Rectangle {
            id: stripeBg

            anchors.fill: parent
            color: Cpp_ThemeManager.colors["groupbox_border"]
            opacity: 0.4
          }

          Rectangle {
            id: stripeChunk

            y: 0
            height: parent.height
            width: parent.width * 0.35
            color: Cpp_ThemeManager.colors["highlight"]
            radius: height / 2

            SequentialAnimation on x {
              loops: Animation.Infinite
              running: workingStripe.visible
              NumberAnimation {
                duration: 1100
                to: workingStripe.width
                from: -stripeChunk.width
                easing.type: Easing.InOutQuad
              }
            }
          }
        }

        //
        // Composer: a single rounded surface with the text field on the left
        // and trailing icon buttons on the right (Clear + Send/Cancel).
        //
        AssistantPanel.AssistantComposer {
          id: composerSection
        }

    }
  }

  //
  // Embedded key manager (loaded on demand)
  //
  DialogLoader {
    id: keyManager

    source: "qrc:/serial-studio.com/gui/qml/AI/KeyManagerDialog.qml"
  }

  //
  // Embedded memory manager (loaded on demand)
  //
  DialogLoader {
    id: memoryManager

    source: "qrc:/serial-studio.com/gui/qml/AI/MemoryManagerDialog.qml"
  }
}
