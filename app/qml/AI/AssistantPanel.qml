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
  width: 820
  height: 680
  minimumWidth: 720
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
  // True when the conversation has no messages yet
  //
  readonly property bool conversationEmpty:
    !Cpp_AI_Assistant.conversation
    || Cpp_AI_Assistant.conversation.messages.length === 0

  //
  // Dialog content (SmartDialog handles native window integration,
  // titlebar drag, and theme palette automatically)
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12

      //
      // Conversation area: centered welcome card when empty, framed list once
      // the user has sent a first message.
      //
      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        //
        // Empty state: centered card with icon + welcome + suggestion chips
        //
        Rectangle {
          id: welcomeCard

          radius: 12
          visible: root.conversationEmpty
          width: Math.min(parent.width - 32, 680)
          implicitHeight: welcomeColumn.implicitHeight + 36
          anchors.horizontalCenter: parent.horizontalCenter
          anchors.verticalCenter: parent.verticalCenter
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.width: 1
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          //
          // True when the active provider has a key configured (recomputed via signals)
          //
          property bool hasActiveKey:
            Cpp_AI_Assistant.hasKey(Cpp_AI_Assistant.currentProvider)

          Connections {
            target: Cpp_AI_Assistant
            function onKeysChanged() {
              welcomeCard.hasActiveKey = Cpp_AI_Assistant.hasKey(
                Cpp_AI_Assistant.currentProvider)
            }
            function onCurrentProviderChanged() {
              welcomeCard.hasActiveKey = Cpp_AI_Assistant.hasKey(
                Cpp_AI_Assistant.currentProvider)
            }
          }

          //
          // Conversation starters grouped by category; shuffleSuggestions()
          // picks one per category each time the welcome card becomes visible.
          //
          readonly property var suggestionPool: ({
            "discovery": [
              qsTr("Help me discover Serial Studio's features"),
              qsTr("What can this app do for my telemetry?"),
              qsTr("Walk me through what this project already contains"),
              qsTr("List the sources in this project")
            ],
            "concepts": [
              qsTr("What is a session database, and why would I use one?"),
              qsTr("CSV vs MDF4 export - what is the difference?"),
              qsTr("What is a frame parser, and when do I need one?"),
              qsTr("When should I use Lua vs JavaScript for the parser?"),
              qsTr("Plot, Bar, and Gauge - when to use each?"),
              qsTr("What is the difference between a transform and a frame parser?")
            ],
            "build": [
              qsTr("Add a UART source for an Arduino"),
              qsTr("Set up an IMU project from scratch"),
              qsTr("Configure an MQTT subscriber"),
              qsTr("Add a CAN bus source"),
              qsTr("Set up a Modbus poller"),
              qsTr("Add a network (TCP/UDP) source"),
              qsTr("Write a CSV frame parser for me"),
              qsTr("Help me parse a JSON frame"),
              qsTr("Add an EMA smoothing transform to a dataset"),
              qsTr("Decode hexadecimal frames"),
              qsTr("Calibrate a sensor with a linear transform")
            ],
            "visualize": [
              qsTr("Suggest dashboard widgets for my data"),
              qsTr("Build an executive overview workspace"),
              qsTr("Add a painter widget for a custom visualization"),
              qsTr("Show Plot, FFT, and Waterfall for one dataset"),
              qsTr("Group my datasets into useful workspaces")
            ]
          })

          property var displayedSuggestions: []

          function shuffleSuggestions() {
            var picks = []
            var pool = welcomeCard.suggestionPool
            for (var category in pool) {
              var list = pool[category]
              if (!list || list.length === 0)
                continue

              picks.push(list[Math.floor(Math.random() * list.length)])
            }
            displayedSuggestions = picks
          }

          Component.onCompleted: shuffleSuggestions()
          onVisibleChanged: {
            if (visible)
              shuffleSuggestions()
          }

          ColumnLayout {
            id: welcomeColumn

            spacing: 14
            anchors.margins: 24
            anchors.fill: parent

            //
            // Icon
            //
            Image {
              opacity: 0.85
              sourceSize.width: 56
              sourceSize.height: 56
              Layout.preferredWidth: 56
              Layout.preferredHeight: 56
              Layout.alignment: Qt.AlignHCenter
              fillMode: Image.PreserveAspectFit
              source: "qrc:/icons/toolbar/ai.svg"
            }

            Label {
              Layout.alignment: Qt.AlignHCenter
              font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
              color: Cpp_ThemeManager.colors["text"]
              text: welcomeCard.hasActiveKey
                    ? qsTr("How can I help with your project?")
                    : qsTr("Set up your API key to get started")
            }

            Label {
              Layout.alignment: Qt.AlignHCenter
              Layout.fillWidth: true
              horizontalAlignment: Text.AlignHCenter
              wrapMode: Text.WordWrap
              opacity: 0.7
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["text"]
              text: welcomeCard.hasActiveKey
                    ? qsTr("Describe what you would like to build, and I "
                        + "will configure the sources, groups, datasets, "
                        + "frame parsers, and transforms for you.")
                    : qsTr("To start chatting, paste an API key for the "
                        + "selected provider. Keys are encrypted on this "
                        + "machine and never leave your computer except "
                        + "to talk to the provider you choose.")
            }

            //
            // No-key path: prominent CTA + "Get a key" link
            //
            ColumnLayout {
              spacing: 10
              Layout.topMargin: 8
              Layout.alignment: Qt.AlignHCenter
              visible: !welcomeCard.hasActiveKey

              Widgets.IconButton {
                spacing: 8
                iconSize: 16
                leftPadding: 16
                rightPadding: 16
                Layout.preferredHeight: 36
                text: qsTr("Open API Key Setup")
                font: Cpp_Misc_CommonFonts.uiFont
                Layout.alignment: Qt.AlignHCenter
                icon.source: "qrc:/icons/buttons/wrench.svg"
                onClicked: Cpp_AI_Assistant.openKeyManager()
              }

              ToolButton {
                Layout.alignment: Qt.AlignHCenter
                display: AbstractButton.TextBesideIcon
                text: qsTr("Get a key from %1").arg(
                  Cpp_AI_Assistant.providerNames[Cpp_AI_Assistant.currentProvider] || "")
                font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
                icon.width: 14
                icon.height: 14
                icon.color: Cpp_ThemeManager.colors["text"]
                icon.source: "qrc:/icons/buttons/website.svg"
                onClicked: Qt.openUrlExternally(
                  Cpp_AI_Assistant.keyVendorUrl(Cpp_AI_Assistant.currentProvider))
              }
            }

            //
            // Has-key path: suggestion chips arranged as a 2x2 grid so the
            // layout stays balanced regardless of prompt length.
            //
            GridLayout {
              columns: 2
              rowSpacing: 8
              columnSpacing: 8
              Layout.topMargin: 4
              Layout.alignment: Qt.AlignHCenter
              visible: welcomeCard.hasActiveKey

              Repeater {
                model: welcomeCard.displayedSuggestions

                delegate: AbstractButton {
                  id: chip

                  padding: 0
                  contentItem: Item {}
                  Layout.fillWidth: true
                  Layout.preferredHeight: chipText.implicitHeight + 12
                  background: Rectangle {
                    radius: 14
                    color: chip.hovered
                           ? Cpp_ThemeManager.colors["alternate_base"]
                           : Cpp_ThemeManager.colors["base"]
                    border.width: 1
                    border.color: Cpp_ThemeManager.colors["groupbox_border"]

                    Label {
                      id: chipText

                      anchors.fill: parent
                      anchors.leftMargin: 11
                      anchors.rightMargin: 11
                      horizontalAlignment: Text.AlignHCenter
                      verticalAlignment: Text.AlignVCenter
                      elide: Text.ElideRight
                      text: modelData
                      font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
                      color: Cpp_ThemeManager.colors["text"]
                    }
                  }
                  onClicked: {
                    if (Cpp_AI_Assistant.busy)
                      return

                    composer.clear()
                    Cpp_AI_Assistant.sendMessage(modelData)
                  }
                }
              }
            }
          }
        }

        //
        // Active state: framed message transcript. Fades via opacity (never visible: false)
        // so the WebEngine transcript pre-loads and keeps its layout slot.
        //
        Rectangle {
          id: chatFrame

          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          enabled: !root.conversationEmpty
          opacity: root.conversationEmpty ? 0 : 1
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          //
          // Edge fade height; matches the visual rhythm in Hardware.qml.
          //
          readonly property int kFadeHeight: 24
          readonly property color kBgColor: Cpp_ThemeManager.colors["groupbox_background"]

          Loader {
            id: messageLoader

            anchors.margins: 8
            asynchronous: false
            anchors.fill: parent

            Component.onCompleted: {
              messageLoader.setSource(
                Cpp_HasWebEngine
                  ? "qrc:/serial-studio.com/gui/qml/AI/MessageWebView.qml"
                  : "qrc:/serial-studio.com/gui/qml/AI/MessageList.qml",
                Cpp_HasWebEngine
                  ? {}
                  : { "conversation": Cpp_AI_Assistant.conversation })
            }

            onLoaded: {
              if (Cpp_HasWebEngine && messageLoader.item
                  && messageLoader.item.chipClicked) {
                messageLoader.item.chipClicked.connect(function(text) {
                  if (Cpp_AI_Assistant.busy)
                    return

                  composer.clear()
                  Cpp_AI_Assistant.sendMessage(text)
                })
              }
            }
          }
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
      Rectangle {
        id: composerFrame

        radius: 22
        border.width: 1
        Layout.fillWidth: true
        Layout.preferredHeight: 44
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: composer.activeFocus
                     ? Cpp_ThemeManager.colors["highlight"]
                     : Cpp_ThemeManager.colors["groupbox_border"]

        Behavior on border.color { ColorAnimation { duration: 120 } }

        RowLayout {
          spacing: 4
          anchors.fill: parent
          anchors.leftMargin: 16
          anchors.rightMargin: 6

          Widgets.LineField {
            id: composer

            Layout.fillWidth: true
            Layout.fillHeight: true
            background: Item {}
            verticalAlignment: TextInput.AlignVCenter
            enabled: !Cpp_AI_Assistant.busy
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["text"]
            placeholderText: qsTr("Ask Serial Studio anything…")
            placeholderTextColor: Qt.darker(Cpp_ThemeManager.colors["text"], 1.5)
            onAccepted: {
              if (!Cpp_AI_Assistant.busy && composer.text.length > 0) {
                Cpp_AI_Assistant.sendMessage(composer.text)
                composer.clear()
              }
            }
          }

          //
          // Trailing Clear (icon-only, dimmed when inactive)
          //
          ToolButton {
            id: clearButton

            display: AbstractButton.IconOnly
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Clear conversation")
            ToolTip.visible: hovered
            ToolTip.delay: 400
            enabled: !Cpp_AI_Assistant.busy
                     && Cpp_AI_Assistant.conversation
                     && Cpp_AI_Assistant.conversation.messages.length > 0
            opacity: enabled ? 0.85 : 0.35
            icon.width: 16
            icon.height: 16
            icon.color: Cpp_ThemeManager.colors["text"]
            icon.source: "qrc:/icons/buttons/trash.svg"
            onClicked: Cpp_AI_Assistant.clearConversation()

            Behavior on opacity { NumberAnimation { duration: 120 } }
          }

          //
          // Send / Cancel button
          //
          Widgets.IconButton {
            id: sendButton

            padding: 0
            iconSize: 14
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            display: AbstractButton.IconOnly
            Layout.alignment: Qt.AlignVCenter

            readonly property bool canActivate:
              Cpp_AI_Assistant.busy
              || (!Cpp_AI_Assistant.busy && composer.text.length > 0)

            hoverEnabled: true
            enabled: canActivate

            icon.source: Cpp_AI_Assistant.busy
                         ? "qrc:/icons/buttons/cancel.svg"
                         : "qrc:/icons/buttons/send.svg"
            icon.color: canActivate
                        ? Cpp_ThemeManager.colors["highlighted_text"]
                        : Cpp_ThemeManager.colors["button_text"]

            ToolTip.text: Cpp_AI_Assistant.busy
                          ? qsTr("Stop generating")
                          : qsTr("Send message (Enter)")
            ToolTip.visible: hovered
            ToolTip.delay: 400

            background: Rectangle {
              anchors.fill: parent
              radius: width / 2
              color: sendButton.canActivate
                     ? Cpp_ThemeManager.colors["highlight"]
                     : Cpp_ThemeManager.colors["mid"]
              opacity: sendButton.canActivate ? 1.0 : 0.4

              Behavior on color { ColorAnimation { duration: 150 } }
              Behavior on opacity { NumberAnimation { duration: 150 } }
            }

            onClicked: {
              if (Cpp_AI_Assistant.busy) {
                Cpp_AI_Assistant.cancel()
              } else if (composer.text.length > 0) {
                Cpp_AI_Assistant.sendMessage(composer.text)
                composer.clear()
              }
            }
          }
        }
      }

      //
      // Footer: flat provider + model pickers, key manager icon, status pill
      //
      RowLayout {
        spacing: 6
        Layout.fillWidth: true

        Widgets.Combo {
          id: providerCombo

          flat: true
          ToolTip.delay: 400
          ToolTip.visible: hovered
          Layout.preferredHeight: 26
          ToolTip.text: qsTr("Provider")
          model: Cpp_AI_Assistant.providerNames
          currentIndex: Cpp_AI_Assistant.currentProvider
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          onActivated: {
            if (currentIndex === Cpp_AI_Assistant.currentProvider)
              return

            const target = currentIndex
            Cpp_AI_Assistant.requestProviderSwitch(target)
            currentIndex = Cpp_AI_Assistant.currentProvider
            modelCombo.refresh()
          }

          Connections {
            target: Cpp_AI_Assistant
            function onCurrentProviderChanged() {
              providerCombo.currentIndex = Cpp_AI_Assistant.currentProvider
            }
          }
        }

        Widgets.Combo {
          id: modelCombo

          property var modelIds: []

          flat: true
          valueRole: "id"
          textRole: "name"
          ToolTip.delay: 400
          ToolTip.visible: hovered
          Layout.preferredHeight: 26
          ToolTip.text: qsTr("Model selection")
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          onActivated: {
            const id = modelIds[currentIndex]
            if (id)
              Cpp_AI_Assistant.setModel(Cpp_AI_Assistant.currentProvider, id)
          }

          function refresh() {
            const provider = Cpp_AI_Assistant.currentProvider
            const ids = Cpp_AI_Assistant.availableModels(provider)
            const items = []
            for (let i = 0; i < ids.length; ++i) {
              items.push({
                id: ids[i],
                name: Cpp_AI_Assistant.modelDisplayName(provider, ids[i])
              })
            }
            modelIds = ids
            model = items

            const cur = Cpp_AI_Assistant.currentModel(provider)
            const idx = ids.indexOf(cur)
            if (idx >= 0)
              currentIndex = idx
          }

          Component.onCompleted: refresh()

          Connections {
            target: Cpp_AI_Assistant
            function onCurrentProviderChanged() { modelCombo.refresh() }
          }
        }

        //
        // Auto-approve edits: skips per-call confirmation for write-class
        // tools. Blocked tools stay blocked.
        //
        CheckBox {
          id: autoApproveCheck

          Layout.preferredHeight: 26
          ToolTip.delay: 400
          ToolTip.visible: hovered
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          ToolTip.text: qsTr("Run editing actions without asking each time. "
                             + "Blocked actions stay blocked.")
          text: qsTr("Auto-approve edits")
          checked: Cpp_AI_Assistant.autoApproveEdits
          onToggled: Cpp_AI_Assistant.autoApproveEdits = checked
        }

        //
        // Allow device control: unblocks driver config, connect/disconnect and
        // writes behind a C++ warning dialog; Connections resyncs on decline.
        //
        CheckBox {
          id: deviceControlCheck

          Layout.preferredHeight: 26
          ToolTip.delay: 400
          ToolTip.visible: hovered
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          ToolTip.text: qsTr("Let the AI configure devices, connect/disconnect and send data. "
                             + "Each action still asks for your approval.")
          text: qsTr("Allow device control")
          checked: Cpp_AI_Assistant.allowDeviceControl
          onToggled: Cpp_AI_Assistant.allowDeviceControl = checked

          Connections {
            target: Cpp_AI_Assistant
            function onAllowDeviceControlChanged() {
              deviceControlCheck.checked = Cpp_AI_Assistant.allowDeviceControl
            }
          }
        }

        //
        // Spacer right-aligning the key-manager button + status pill
        //
        Item {
          Layout.fillWidth: true
        }

        //
        //
        //
        ToolButton {
          display: AbstractButton.IconOnly
          Layout.preferredWidth: 26
          Layout.preferredHeight: 26
          ToolTip.text: qsTr("Manage API keys")
          ToolTip.visible: hovered
          ToolTip.delay: 400
          icon.width: 14
          icon.height: 14
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/icons/buttons/wrench.svg"
          onClicked: Cpp_AI_Assistant.openKeyManager()
        }

        //
        // Status pill: Ready / Working + cache token info
        //
        Label {
          Layout.alignment: Qt.AlignRight
          opacity: 0.55
          font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
          color: Cpp_ThemeManager.colors["pane_section_label"]
          text: {
            const r = Cpp_AI_Assistant.cacheReadTokens
            const c = Cpp_AI_Assistant.cacheCreatedTokens
            const status = Cpp_AI_Assistant.busy ? qsTr("Working")
                                                  : qsTr("Ready")
            if (r > 0) return status + qsTr("  •  cache %1k tok").arg(Math.round(r / 1000))
            if (c > 0) return status + qsTr("  •  cache write %1k tok").arg(Math.round(c / 1000))
            return status
          }
          Component.onCompleted: font.capitalization = Font.AllUppercase
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
}
