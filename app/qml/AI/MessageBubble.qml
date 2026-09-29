/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Item {
  id: root

  //
  // Public properties
  //
  property string role: "assistant"
  property string text: ""
  property string thinking: ""
  property bool streaming: false
  property var toolCalls: []
  property real maxWidth: 600
  property var expansionStore: null

  readonly property bool isUser: role === "user"
  readonly property bool hasText: text.length > 0
  readonly property bool isError: role === "error"
  readonly property bool hasThinking: thinking.length > 0
  readonly property bool showTyping: !isUser && !isError && streaming && !hasText

  //
  // CallStatus::AwaitingConfirm
  //
  readonly property int kAwaitingConfirm: 1

  function groupStartFor(idx) {
    if (idx < 0 || idx >= root.toolCalls.length) return -1
    const cur = root.toolCalls[idx]
    if (cur.status !== root.kAwaitingConfirm) return -1
    var start = idx
    while (start > 0) {
      const prev = root.toolCalls[start - 1]
      if (prev.status !== root.kAwaitingConfirm) break
      if ((prev.family || "") !== (cur.family || "")) break
      start -= 1
    }
    return start
  }

  function groupSizeFor(idx) {
    const start = groupStartFor(idx)
    if (start < 0) return 0
    const fam = root.toolCalls[start].family || ""
    var n = 0
    for (var i = start; i < root.toolCalls.length; i++) {
      const c = root.toolCalls[i]
      if (c.status !== root.kAwaitingConfirm) break
      if ((c.family || "") !== fam) break
      n += 1
    }
    return n
  }

  //
  // Geometry
  //
  implicitHeight: layout.implicitHeight + 12

  ColumnLayout {
    id: layout

    spacing: 6
    width: root.isUser
           ? Math.min(root.maxWidth, parent.width - 16)
           : parent.width - 16
    anchors.left: root.isUser ? undefined : parent.left
    anchors.right: root.isUser ? parent.right : undefined
    anchors.leftMargin: 8
    anchors.rightMargin: 8
    anchors.top: parent.top

    //
    // Role label (ALL-CAPS section style, matches Help Center)
    //
    Label {
      Layout.alignment: root.isUser ? Qt.AlignRight : Qt.AlignLeft
      visible: root.hasText || root.showTyping || root.hasThinking
      opacity: 0.85
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: root.isError
             ? Cpp_ThemeManager.colors["alarm"]
             : Cpp_ThemeManager.colors["pane_section_label"]
      text: root.isError
            ? qsTr("Error")
            : (root.isUser ? qsTr("You") : qsTr("Assistant"))
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    //
    // Thinking preamble (visible while reasoning, hidden once response arrives)
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      visible: !root.isUser && root.hasThinking && root.streaming

      BusyIndicator {
        running: parent.visible
        Layout.preferredWidth: 14
        Layout.preferredHeight: 14
        Layout.alignment: Qt.AlignTop
      }

      TextEdit {
        id: thinkingText

        Layout.fillWidth: true
        readOnly: true
        selectByMouse: true
        wrapMode: TextEdit.Wrap
        text: root.thinking
        font: Cpp_Misc_CommonFonts.customUiFont(0.92, false)
        color: Qt.darker(Cpp_ThemeManager.colors["text"], 1.2)
        textFormat: TextEdit.PlainText
      }
    }

    //
    // Typing indicator (3 pulsing dots) when streaming has started but no
    // text or thinking has arrived yet.
    //
    Row {
      spacing: 5
      Layout.topMargin: 4
      Layout.bottomMargin: 4
      Layout.alignment: Qt.AlignLeft
      visible: root.showTyping && !root.hasThinking

      Repeater {
        model: 3

        Rectangle {
          width: 7
          height: 7
          radius: 3.5
          color: Cpp_ThemeManager.colors["text"]
          opacity: 0.35

          SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: parent.parent.visible
            PauseAnimation { duration: index * 150 }
            NumberAnimation { from: 0.3; to: 0.95; duration: 350 }
            NumberAnimation { from: 0.95; to: 0.3; duration: 350 }
            PauseAnimation { duration: (2 - index) * 150 }
          }
        }
      }
    }

    //
    // Assistant: unboxed full-width text. User: right-aligned bubble.
    //
    Loader {
      visible: root.hasText
      Layout.fillWidth: !root.isUser
      sourceComponent: root.isError
                       ? errorBubble
                       : (root.isUser ? userBubble : assistantPlain)
      Layout.alignment: root.isUser ? Qt.AlignRight : Qt.AlignLeft
    }

    //
    // Tool-call cards (assistant only) with category dividers + batch headers
    //
    Repeater {
      model: root.isUser ? [] : root.toolCalls

      delegate: ColumnLayout {
        spacing: 4
        Layout.fillWidth: true

        readonly property string category: modelData.category || ""
        readonly property string previousCategory: index === 0
          ? ""
          : (root.toolCalls[index - 1].category || "")
        readonly property bool showSectionLabel: category.length > 0
                                                 && category !== previousCategory

        readonly property string family: modelData.family || ""
        readonly property int groupSize: root.groupSizeFor(index)
        readonly property int groupStart: root.groupStartFor(index)
        readonly property bool inGroup: groupSize >= 2 && groupStart >= 0
        readonly property bool isGroupHead: inGroup && groupStart === index

        Label {
          Layout.topMargin: index === 0 ? 8 : 12
          Layout.bottomMargin: 2
          Layout.leftMargin: 2
          visible: showSectionLabel
          opacity: 0.85
          font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
          color: Cpp_ThemeManager.colors["pane_section_label"]
          text: category === "discovery"
                ? qsTr("Discovery")
                : qsTr("Execution")
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        //
        // Batched approval header: shown above the first card of a group
        // of >=2 pending Confirm calls sharing the same parent family.
        //
        Rectangle {
          Layout.fillWidth: true
          Layout.topMargin: 4
          visible: isGroupHead
          implicitHeight: groupRow.implicitHeight + 16
          radius: 8
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.width: 1
          border.color: Cpp_ThemeManager.colors["accent"]

          RowLayout {
            id: groupRow

            spacing: 10
            anchors.margins: 8
            anchors.fill: parent

            ColumnLayout {
              spacing: 2
              Layout.fillWidth: true

              Label {
                text: qsTr("Approve %1 actions in %2?")
                        .arg(groupSize)
                        .arg(family)
                font: Cpp_Misc_CommonFonts.boldUiFont
                color: Cpp_ThemeManager.colors["text"]
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
              }

              Label {
                text: qsTr("These calls will run together. Expand each "
                           + "card below to inspect arguments.")
                opacity: 0.75
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                color: Cpp_ThemeManager.colors["text"]
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
              }
            }

            Widgets.PushButton {
              text: qsTr("Approve all")
              font: Cpp_Misc_CommonFonts.uiFont
              onClicked: Cpp_AI_Assistant.approveToolCallGroup(family)
            }

            Widgets.PushButton {
              text: qsTr("Deny all")
              font: Cpp_Misc_CommonFonts.uiFont
              onClicked: Cpp_AI_Assistant.denyToolCallGroup(family)
            }
          }
        }

        ToolCallCard {
          Layout.fillWidth: true
          groupedConfirm: inGroup
          callId: modelData.callId
          toolName: modelData.name
          argsJson: modelData.args
          resultJson: modelData.result
          statusValue: modelData.status
          expansionStore: root.expansionStore
          Layout.topMargin: showSectionLabel ? 0 : 4
          verification: modelData.verification !== undefined ? modelData.verification : null
        }
      }
    }
  }

  //
  // Components: user bubble (right-aligned pill) and assistant plain text
  //
  Component {
    id: userBubble

    Rectangle {
      id: bubble

      radius: 14
      implicitWidth: Math.min(userText.implicitWidth + 28,
                              root.maxWidth)
      implicitHeight: userText.implicitHeight + 22
      color: Cpp_ThemeManager.colors["highlight"]

      TextEdit {
        id: userText

        readOnly: true
        selectByMouse: true
        wrapMode: TextEdit.Wrap
        anchors.margins: 12
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.fill: parent
        text: root.text
        font: Cpp_Misc_CommonFonts.uiFont
        textFormat: TextEdit.PlainText
        color: Cpp_ThemeManager.colors["highlighted_text"]
        selectionColor: Cpp_ThemeManager.colors["base"]
        selectedTextColor: Cpp_ThemeManager.colors["text"]

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.IBeamCursor
          acceptedButtons: Qt.RightButton
          onClicked: function(mouse) {
            userMenu.popup(mouse.x, mouse.y)
          }
        }

        Menu {
          id: userMenu

          MenuItem {
            text: qsTr("Copy")
            enabled: userText.selectedText.length > 0
            onTriggered: userText.copy()
          }
          MenuItem {
            text: qsTr("Copy All")
            onTriggered: {
              userText.selectAll()
              userText.copy()
              userText.deselect()
            }
          }
          MenuItem {
            text: qsTr("Select All")
            onTriggered: userText.selectAll()
          }

          MenuSeparator {}

          Menu {
            title: qsTr("Remember this…")

            MenuItem {
              text: qsTr("As a preference about me")
              onTriggered: Cpp_AI_Assistant.addMemory("user", root.text)
            }
            MenuItem {
              text: qsTr("As a correction / lesson")
              onTriggered: Cpp_AI_Assistant.addMemory("feedback", root.text)
            }
            MenuItem {
              text: qsTr("As a fact about this project")
              onTriggered: Cpp_AI_Assistant.addMemory("project", root.text)
            }
            MenuItem {
              text: qsTr("As a reference")
              onTriggered: Cpp_AI_Assistant.addMemory("reference", root.text)
            }
          }
        }
      }
    }
  }

  Component {
    id: assistantPlain

    TextEdit {
      id: assistantText

      readOnly: true
      selectByMouse: true
      wrapMode: TextEdit.Wrap
      text: root.text
      font: Cpp_Misc_CommonFonts.uiFont
      textFormat: root.streaming ? TextEdit.PlainText : TextEdit.MarkdownText
      color: Cpp_ThemeManager.colors["text"]
      selectionColor: Cpp_ThemeManager.colors["highlight"]
      selectedTextColor: Cpp_ThemeManager.colors["highlighted_text"]

      onLinkActivated: function(link) { Qt.openUrlExternally(link) }

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        acceptedButtons: Qt.RightButton
        onClicked: function(mouse) {
          assistantMenu.popup(mouse.x, mouse.y)
        }
      }

      Menu {
        id: assistantMenu

        MenuItem {
          text: qsTr("Copy")
          enabled: assistantText.selectedText.length > 0
          onTriggered: assistantText.copy()
        }
        MenuItem {
          text: qsTr("Copy All")
          onTriggered: {
            assistantText.selectAll()
            assistantText.copy()
            assistantText.deselect()
          }
        }
        MenuItem {
          text: qsTr("Select All")
          onTriggered: assistantText.selectAll()
        }
      }
    }
  }

  //
  // Error bubble: bold red text, selectable, wraps, copy menu
  //
  Component {
    id: errorBubble

    TextEdit {
      id: errorText

      readOnly: true
      selectByMouse: true
      wrapMode: TextEdit.Wrap
      text: root.text
      font: Cpp_Misc_CommonFonts.boldUiFont
      textFormat: TextEdit.PlainText
      color: Cpp_ThemeManager.colors["alarm"]
      selectionColor: Cpp_ThemeManager.colors["highlight"]
      selectedTextColor: Cpp_ThemeManager.colors["highlighted_text"]

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        acceptedButtons: Qt.RightButton
        onClicked: function(mouse) {
          errorMenu.popup(mouse.x, mouse.y)
        }
      }

      Menu {
        id: errorMenu

        MenuItem {
          text: qsTr("Copy")
          enabled: errorText.selectedText.length > 0
          onTriggered: errorText.copy()
        }
        MenuItem {
          text: qsTr("Copy All")
          onTriggered: {
            errorText.selectAll()
            errorText.copy()
            errorText.deselect()
          }
        }
      }
    }
  }
}
