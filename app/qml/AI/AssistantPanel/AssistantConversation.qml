/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

//
// Assistant conversation area: the centered welcome card while the chat is empty, and the
// framed message list with its drag-and-drop overlay once the user has sent a message.
//
Item {
  id: root

  signal composerClearRequested()

  //
  // True when the conversation has no messages yet
  //
  readonly property bool conversationEmpty:
    !Cpp_AI_Assistant.conversation
    || Cpp_AI_Assistant.conversation.messageCount === 0

Layout.fillWidth: true
Layout.fillHeight: true

//
// Empty state: centered card with icon + welcome + suggestion chips
//
Rectangle {
  id: welcomeCard

  z: 1
  radius: 12
  border.width: 0
  color: "transparent"
  border.color: "transparent"
  visible: root.conversationEmpty
  width: Math.min(parent.width - 32, 680)
  anchors.verticalCenter: parent.verticalCenter
  implicitHeight: welcomeColumn.implicitHeight + 36
  anchors.horizontalCenter: parent.horizontalCenter

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
    function onActiveChatChanged() {
      if (welcomeCard.visible)
        welcomeCard.shuffleSuggestions()
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
      qsTr("What is the historian, and why would I use it?"),
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
      qsTr("Add a canvas widget for a custom visualization"),
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
      source: Cpp_Misc_IconRegistry.icon("editor", "ai", 48)
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
                + "selected provider. Keys are stored obfuscated in "
                + "this machine's settings file and never leave your "
                + "computer except to talk to the provider you choose.")
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

        delegate: Rectangle {
          id: chip

          radius: 14
          Layout.fillWidth: true
          Layout.preferredHeight: chipText.implicitHeight + 12
          border.width: 1
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
          color: chipArea.containsMouse
                 ? Cpp_ThemeManager.colors["alternate_base"]
                 : Cpp_ThemeManager.colors["base"]

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

          MouseArea {
            id: chipArea

            hoverEnabled: true
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              if (Cpp_AI_Assistant.busy)
                return

              root.composerClearRequested()
              Cpp_AI_Assistant.sendMessage(modelData)
            }
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

  radius: 8
  opacity: 1
  border.width: 1
  anchors.fill: parent
  enabled: !root.conversationEmpty
  color: Cpp_ThemeManager.colors["groupbox_background"]
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

          root.composerClearRequested()
          Cpp_AI_Assistant.sendMessage(text)
        })
      }
    }
  }
}

//
// Drag & drop authorizes any dropped file/folder for sandboxed reads this session
//
DropArea {
  id: fileDrop

  anchors.fill: parent

  //
  // Highlight the overlay while a drag hovers
  //
  onEntered: (drag) => {
    if (drag.urls.length > 0) {
      drag.accept(Qt.LinkAction)
      dropOverlay.opacity = 0.92
    }
  }

  onExited: dropOverlay.opacity = 0

  //
  // Register every dropped URL with the C++ sandbox allow-list
  //
  onDropped: (drop) => {
    dropOverlay.opacity = 0
    for (var i = 0; i < drop.urls.length; ++i) {
      var path = drop.urls[i].toString()
      if (Qt.platform.os !== "windows")
        path = path.replace(/^(file:\/{2})/, "")
      else
        path = path.replace(/^(file:\/{3})/, "")

      Cpp_AI_Assistant.addDroppedPath(decodeURIComponent(path))
    }
  }

  //
  // Drop affordance rectangle
  //
  Rectangle {
    id: dropOverlay

    opacity: 0
    radius: 12
    border.width: 2
    anchors.fill: parent
    color: Qt.rgba(Cpp_ThemeManager.colors["highlight"].r,
                   Cpp_ThemeManager.colors["highlight"].g,
                   Cpp_ThemeManager.colors["highlight"].b, 0.18)
    border.color: Cpp_ThemeManager.colors["highlight"]

    Behavior on opacity { NumberAnimation { duration: 120 } }

    Label {
      anchors.centerIn: parent
      text: qsTr("Drop files or folders to let the assistant read them")
      font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
      color: Cpp_ThemeManager.colors["highlighted_text"]
    }
  }

  //
  // Transient banner confirming the most recent authorization
  //
  Rectangle {
    id: dropToast

    radius: 8
    height: 30
    opacity: 0
    border.width: 1
    anchors.top: parent.top
    anchors.topMargin: 8
    width: toastLabel.implicitWidth + 28
    anchors.horizontalCenter: parent.horizontalCenter
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["highlight"]

    Behavior on opacity { NumberAnimation { duration: 160 } }

    Label {
      id: toastLabel

      anchors.centerIn: parent
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.uiFont
    }

    Timer {
      id: toastTimer

      repeat: false
      interval: 2600
      onTriggered: dropToast.opacity = 0
    }

    Connections {
      target: Cpp_AI_Assistant
      function onDroppedPathAdded(displayName, isDir) {
        toastLabel.text = isDir
          ? qsTr("Added folder \"%1\" - readable this session").arg(displayName)
          : qsTr("Added \"%1\" - readable this session").arg(displayName)
        dropToast.opacity = 1
        toastTimer.restart()
      }
      function onMemoryChanged() {
        toastLabel.text = qsTr("Assistant memory updated")
        dropToast.opacity = 1
        toastTimer.restart()
      }
    }
  }
}
}
