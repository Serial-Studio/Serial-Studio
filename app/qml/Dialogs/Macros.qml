/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window geometry & title
  //
  fixedSize: false
  contentPadding: 0
  preferredWidth: 880
  preferredHeight: 560
  title: qsTr("Macros")

  //
  // Command catalog, loaded once from the live registry
  //
  property var catalog: []
  Component.onCompleted: {
    root.catalog = bridge.catalog()
    if (typeof app !== "undefined" && app.macroDraft.length > 0) {
      macroEditor.setLanguage(app.macroDraftLanguage)
      macroEditor.setText(app.macroDraft)
    }

    else
      root.seedStarterMacro()
  }

  //
  // Hello world starter macros, one per language combo index (JavaScript, Lua)
  //
  readonly property var starterMacros: [
    [
      "//",
      "// Hello world macro",
      "//",
      "// - Press the play button to run it",
      "// - Output shows up in the Terminal tab",
      "// - Every Serial Studio API command is reachable",
      "//   through: apiCall(method, params)",
      "// - The reply is always an object:",
      "//   { ok: true, result: ... } on success, or",
      "//   { ok: false, error: \"...\" } on failure",
      "// - The SDK wrappers are preloaded too, so",
      "//   api.getCommands() equals the apiCall above",
      "//",
      "",
      "// Call api.getCommands",
      "const reply = apiCall(\"api.getCommands\")",
      "if (!reply.ok)",
      "  throw new Error(reply.error)",
      "",
      "// Obtain the list of commands",
      "const commands = reply.result.commands",
      "console.log(\"Hello from Serial Studio! \"",
      "            + commands.length + \" commands available.\\n\")",
      "",
      "// Print the first few commands",
      "for (let i = 0; i < Math.min(5, commands.length); ++i) {",
      "  const cmd = commands[i]",
      "  console.log(\"  \" + cmd.name + \" - \"",
      "              + cmd.description + \"\\n\")",
      "}",
      ""
    ].join("\n"),
    [
      "--",
      "-- Hello world macro",
      "--",
      "-- - Press the play button to run it",
      "-- - Output shows up in the Terminal tab",
      "-- - Every Serial Studio API command is reachable",
      "--   through: apiCall(method, params)",
      "-- - The reply is always a table:",
      "--   { ok = true, result = ... } on success, or",
      "--   { ok = false, error = \"...\" } on failure",
      "-- - The SDK wrappers are preloaded too, so",
      "--   api.getCommands() equals the apiCall above",
      "--",
      "",
      "-- Call api.getCommands",
      "local reply = apiCall(\"api.getCommands\")",
      "if not reply.ok then",
      "  error(reply.error)",
      "end",
      "",
      "-- Obtain the list of commands",
      "local commands = reply.result.commands",
      "print(\"Hello from Serial Studio! \"",
      "      .. #commands .. \" commands available.\\n\")",
      "",
      "-- Print the first few commands",
      "for i = 1, math.min(5, #commands) do",
      "  local cmd = commands[i]",
      "  print(\"  \" .. cmd.name .. \" - \"",
      "        .. cmd.description .. \"\\n\")",
      "end",
      ""
    ].join("\n")
  ]

  //
  // Fills the editor with the current language's starter; marked saved so it never
  // triggers the unsaved-changes prompt
  //
  function seedStarterMacro() {
    macroEditor.setText(root.starterMacros[macroEditor.language])
    macroEditor.markSaved()
  }

  //
  // Save settings
  //
  Settings {
    category: "Macros"
    property alias dW: root.width
    property alias dH: root.height
  }

  //
  // Exact-name catalog lookup shared by the docs panel, tree insert, and completion
  //
  function findCommand(name) {
    for (let i = 0; i < root.catalog.length; ++i) {
      if (root.catalog[i].name === name)
        return root.catalog[i]
    }

    return null
  }

  //
  // Builds an input-line template with a JSON parameter skeleton for a catalog entry
  //
  function commandTemplate(cmd) {
    if (cmd === null)
      return ""

    if (cmd.params.length === 0)
      return cmd.name + " "

    const parts = []
    for (let i = 0; i < cmd.params.length; ++i) {
      const param = cmd.params[i]
      let value = "\"\""
      if (param.type === "integer" || param.type === "number")
        value = "0"
      else if (param.type === "boolean")
        value = "false"
      else if (param.type === "array")
        value = "[]"
      else if (param.type === "object")
        value = "{}"

      parts.push("\"" + param.name + "\": " + value)
    }

    return cmd.name + " { " + parts.join(", ") + " }"
  }

  //
  // In-process command dispatcher
  //
  ApiTerminalBridge {
    id: bridge
  }

  //
  // Macro execution engine, output routed into the shared scrollback
  //
  MacroRunner {
    id: macroRunner

    onLogMessage: (message) => terminalPane.appendText(message)
    onFinished: (result) => terminalPane.appendText(
                  (result.length > 0 ? result : qsTr("[macro] finished")) + "\n")
    onScriptError: (message) => terminalPane.appendText(qsTr("[macro] %1").arg(message) + "\n")
    onMacroSaved: (fileName) => {
                    macroEditor.markSaved()
                    terminalPane.appendText(qsTr("[macro] saved %1").arg(fileName) + "\n")
                  }
    onMacroLoaded: (text, language, fileName) => {
                     macroEditor.setLanguage(language)
                     macroEditor.setText(text)
                     terminalPane.appendText(qsTr("[macro] loaded %1").arg(fileName) + "\n")
                   }
  }

  //
  // Session-scoped draft: keep unsaved editor content across window close/reopen
  //
  onClosing: {
    if (typeof app !== "undefined") {
      app.macroDraft = macroEditor.text
      app.macroDraftLanguage = macroEditor.language
    }
  }

  //
  // User interface controls
  //
  dialogContent: ColumnLayout {
    spacing: 0

    //
    // Titlebar separator
    //
    Rectangle {
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    //
    // Dialog Contents
    //
    SplitView {
      Layout.fillWidth: true
      Layout.fillHeight: true

      handle: Rectangle {
        implicitWidth: 1
        implicitHeight: 1
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      //
      // Command discovery pane
      //
      Item {
        id: discoveryPane

        SplitView.minimumWidth: 260
        SplitView.preferredWidth: 320

        //
        // Search filter over command name + description
        //
        property string searchText: ""
        readonly property var filteredCommands: {
          const q = searchText.toLowerCase().trim()
          if (!q)
            return root.catalog

          return root.catalog.filter(function(c) {
            return c.name.toLowerCase().indexOf(q) >= 0
                || c.description.toLowerCase().indexOf(q) >= 0
          })
        }

        //
        // Documentation target: an exactly-typed command wins over the list selection
        //
        readonly property var docCommand: {
          for (let i = 0; i < root.catalog.length; ++i) {
            if (root.catalog[i].name === commandInput.typedToken)
              return root.catalog[i]
          }

          if (commandList.currentIndex >= 0 && commandList.currentIndex < filteredCommands.length)
            return filteredCommands[commandList.currentIndex]

          return null
        }

        //
        // Inserts a command with its JSON parameter skeleton into the input line
        //
        function insertCommand(name) {
          commandInput.text = root.commandTemplate(root.findCommand(name))
          commandInput.cursorPosition = commandInput.text.length
          commandInput.forceActiveFocus()
        }

        ColumnLayout {
          spacing: 0
          anchors.fill: parent

          //
          // Search field
          //
          Rectangle {
            implicitHeight: 40
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_background"]

            Rectangle {
              height: 1
              width: parent.width
              anchors.bottom: parent.bottom
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            Widgets.SearchField {
              implicitHeight: 28
              placeholderText: qsTr("Search commands...")
              color: Cpp_ThemeManager.colors["base"]
              onTextChanged: discoveryPane.searchText = text

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
          // Command list, grouped by scope
          //
          ListView {
            id: commandList

            clip: true
            currentIndex: -1
            Layout.fillWidth: true
            Layout.fillHeight: true
            section.property: "scope"
            model: discoveryPane.filteredCommands
            section.delegate: Rectangle {
              required property string section

              height: 24
              width: commandList.width
              color: Cpp_ThemeManager.colors["groupbox_background"]

              Label {
                text: parent.section
                font: Cpp_Misc_CommonFonts.boldUiFont

                anchors {
                  leftMargin: 8
                  left: parent.left
                  verticalCenter: parent.verticalCenter
                }
              }
            }

            delegate: ItemDelegate {
              required property int index
              required property var modelData

              width: commandList.width
              highlighted: ListView.isCurrentItem
              onClicked: commandList.currentIndex = index
              onDoubleClicked: discoveryPane.insertCommand(modelData.name)
              Keys.onReturnPressed: discoveryPane.insertCommand(modelData.name)

              contentItem: ColumnLayout {
                spacing: 0

                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  text: parent.parent.modelData.verb
                  font: Cpp_Misc_CommonFonts.monoFont
                }

                Label {
                  opacity: 0.7
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  font: Cpp_Misc_CommonFonts.uiFont
                  text: parent.parent.modelData.description
                }
              }
            }
          }

          //
          // Documentation panel for the selected or typed command
          //
          Rectangle {
            implicitHeight: 220
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_background"]

            Rectangle {
              height: 1
              width: parent.width
              anchors.top: parent.top
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            ScrollView {
              contentWidth: availableWidth

              anchors {
                margins: 8
                fill: parent
              }

              ColumnLayout {
                spacing: 4
                width: parent.parent.availableWidth
                visible: discoveryPane.docCommand !== null

                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  text: discoveryPane.docCommand ? discoveryPane.docCommand.name : ""
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Text.WordWrap
                  font: Cpp_Misc_CommonFonts.uiFont
                  text: discoveryPane.docCommand ? discoveryPane.docCommand.description : ""
                }

                Label {
                  opacity: 0.7
                  Layout.topMargin: 4
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  visible: discoveryPane.docCommand !== null
                  text: discoveryPane.docCommand
                        && discoveryPane.docCommand.params.length > 0 ? qsTr("Parameters")
                                                                      : qsTr("No parameters")
                }

                Repeater {
                  model: discoveryPane.docCommand ? discoveryPane.docCommand.params : []

                  delegate: ColumnLayout {
                    required property var modelData

                    spacing: 0
                    Layout.fillWidth: true

                    Label {
                      Layout.fillWidth: true
                      elide: Text.ElideRight
                      font: Cpp_Misc_CommonFonts.monoFont
                      text: modelData.name
                            + (modelData.type !== "" ? " (" + modelData.type + ")" : "")
                            + (modelData.required ? " *" : "")
                    }

                    Label {
                      opacity: 0.7
                      Layout.fillWidth: true
                      wrapMode: Text.WordWrap
                      text: modelData.description
                      font: Cpp_Misc_CommonFonts.uiFont
                      visible: modelData.description !== ""
                    }
                  }
                }
              }
            }
          }
        }
      }

      //
      // Right pane: Terminal / Script tabs
      //
      Item {
        id: rightPane

        SplitView.fillWidth: true
        SplitView.minimumWidth: 420

        ColumnLayout {
          spacing: 0
          anchors.fill: parent

          TabBar {
            id: rightTabBar

            Layout.leftMargin: -1
            Layout.bottomMargin: -1

            implicitHeight: 24
            Layout.fillWidth: true
            Layout.maximumHeight: 24

            TabButton {
              text: qsTr("Terminal")
              width: implicitWidth + 2 * 8
              height: rightTabBar.height + 3
            }

            TabButton {
              text: qsTr("Script")
              width: implicitWidth + 2 * 8
              height: rightTabBar.height + 3
            }
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          StackLayout {
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: rightTabBar.currentIndex

            //
            // Terminal tab
            //
            Item {
              id: terminalPane

              //
              // Session-only command history
              //
              property var history: []
              property int historyIndex: -1

              //
              // Echoes the line, dispatches it in-process and prints the JSON response
              //
              function runLine() {
                const line = commandInput.text.trim()
                if (line === "")
                  return

                terminalPane.history.push(line)
                terminalPane.historyIndex = terminalPane.history.length
                terminalPane.appendText("> " + line)

                const response = bridge.run(line)
                terminalPane.appendText(JSON.stringify(response, null, 2) + "\n")
                commandInput.clear()
              }

              //
              // Appends a block of text and keeps the view pinned to the bottom
              //
              function appendText(text) {
                scrollback.append(text)
                scrollback.cursorPosition = scrollback.length
              }

              //
              // Recalls a history entry into the input line
              //
              function recallHistory(delta) {
                if (terminalPane.history.length === 0)
                  return

                const index = Math.min(Math.max(terminalPane.historyIndex + delta, 0),
                                       terminalPane.history.length)
                terminalPane.historyIndex = index
                commandInput.text = index < terminalPane.history.length
                    ? terminalPane.history[index]
                    : ""
              }

              ColumnLayout {
                spacing: 0
                anchors.fill: parent

                //
                // Response scrollback
                //
                ScrollView {
                  Layout.fillWidth: true
                  Layout.fillHeight: true
                  contentWidth: availableWidth
                  ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                  TextArea {
                    id: scrollback

                    readOnly: true
                    selectByMouse: true
                    font: Cpp_Misc_CommonFonts.monoFont
                    wrapMode: TextArea.WrapAtWordBoundaryOrAnywhere
                    color: Cpp_ThemeManager.colors["console_text"]
                    placeholderText: qsTr("Type a command below to get started")

                    background: Rectangle {
                      color: Cpp_ThemeManager.colors["console_base"]
                    }

                    MouseArea {
                      anchors.fill: parent
                      cursorShape: Qt.IBeamCursor
                      acceptedButtons: Qt.RightButton
                      onClicked: (mouse) => {
                                   contextMenu.popup()
                                   mouse.accepted = true
                                 }
                    }
                  }
                }

                //
                // Right-click context menu
                //
                Menu {
                  id: contextMenu

                  MenuItem {
                    text: qsTr("Copy")
                    onTriggered: scrollback.copy()
                    enabled: scrollback.selectedText.length > 0
                  }

                  MenuItem {
                    text: qsTr("Select All")
                    enabled: scrollback.length > 0
                    onTriggered: scrollback.selectAll()
                  }

                  MenuItem {
                    text: qsTr("Clear")
                    enabled: scrollback.length > 0
                    onTriggered: scrollback.clear()
                  }
                }

                //
                // Input row
                //
                Rectangle {
                  implicitHeight: 40
                  Layout.fillWidth: true
                  Layout.leftMargin: -1
                  Layout.rightMargin: -1
                  Layout.bottomMargin: -1
                  color: Cpp_ThemeManager.colors["groupbox_background"]

                  Rectangle {
                    height: 1
                    width: parent.width
                    anchors.top: parent.top
                    color: Cpp_ThemeManager.colors["groupbox_border"]
                  }

                  RowLayout {
                    spacing: 4

                    anchors {
                      margins: 8
                      fill: parent
                      topMargin: 0
                      bottomMargin: 0
                    }

                    Widgets.LineField {
                      id: commandInput

                      implicitHeight: 24
                      Layout.fillWidth: true
                      Layout.alignment: Qt.AlignVCenter
                      font: Cpp_Misc_CommonFonts.monoFont
                      placeholderText: qsTr("command.name { \"parameter\": value }")
                      palette.base: Cpp_ThemeManager.colors["console_base"]
                      palette.text: Cpp_ThemeManager.colors["console_text"]
                      palette.highlight: Cpp_ThemeManager.colors["console_highlight"]
                      palette.highlightedText: Cpp_ThemeManager.colors["console_text"]
                      palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]

                      background: Rectangle {
                        border.width: 1
                        color: Cpp_ThemeManager.colors["console_base"]
                        border.color: Cpp_ThemeManager.colors["console_border"]
                      }

                      //
                      // Command-name completion, active while the first token is still being typed
                      //
                      readonly property string typedToken: text.trim().split(" ")[0]
                      readonly property bool completing: text.length > 0 && text.indexOf(" ") < 0
                      readonly property var completions: {
                        if (!completing)
                          return []

                        const prefix = []
                        const inside = []
                        for (let i = 0; i < root.catalog.length; ++i) {
                          const name = root.catalog[i].name
                          if (name.startsWith(typedToken))
                            prefix.push(name)
                          else if (name.indexOf(typedToken) >= 0)
                            inside.push(name)
                        }

                        return prefix.concat(inside)
                      }

                      function acceptCompletion() {
                        if (completionList.currentIndex < 0
                            || completionList.currentIndex >= completions.length)
                          return

                        const name = completions[completionList.currentIndex]
                        const cmd = root.findCommand(name)
                        text = cmd !== null ? root.commandTemplate(cmd) : name + " "
                        cursorPosition = text.length
                      }

                      onAccepted: terminalPane.runLine()
                      onCompletionsChanged: completionList.currentIndex = completions.length > 0 ? 0 : -1

                      Keys.onUpPressed: (event) => {
                                          if (completionPopup.visible)
                                          completionList.currentIndex = Math.max(completionList.currentIndex - 1, 0)
                                          else
                                          terminalPane.recallHistory(-1)

                                          event.accepted = true
                                        }
                      Keys.onDownPressed: (event) => {
                                            if (completionPopup.visible)
                                            completionList.currentIndex = Math.min(completionList.currentIndex + 1,
                                                                                   commandInput.completions.length - 1)
                                            else
                                            terminalPane.recallHistory(+1)

                                            event.accepted = true
                                          }
                      Keys.onTabPressed: (event) => {
                                           const was_open = completionPopup.visible
                                           if (was_open)
                                           commandInput.acceptCompletion()

                                           event.accepted = was_open
                                         }
                      Keys.onReturnPressed: (event) => {
                                              const completion = completionList.currentIndex >= 0
                                              ? commandInput.completions[completionList.currentIndex]
                                              : ""
                                              if (completionPopup.visible && completion !== commandInput.typedToken) {
                                                commandInput.acceptCompletion()
                                                event.accepted = true
                                              } else
                                              event.accepted = false
                                            }
                      Keys.onEscapePressed: (event) => {
                                              const was_open = completionPopup.visible
                                              completionPopup.forceClosed = true
                                              event.accepted = was_open
                                            }
                      onTextChanged: completionPopup.forceClosed = false

                      //
                      // Completion popup, opens above the input row
                      //
                      Popup {
                        id: completionPopup

                        property bool forceClosed: false

                        focus: false
                        padding: 1
                        y: -height
                        width: commandInput.width
                        closePolicy: Popup.NoAutoClose
                        height: Math.min(completionList.contentHeight + 2, 240)
                        visible: !forceClosed && commandInput.activeFocus
                                 && commandInput.completions.length > 0

                        background: Rectangle {
                          color: Cpp_ThemeManager.colors["base"]
                          border.color: Cpp_ThemeManager.colors["groupbox_border"]
                        }

                        ListView {
                          id: completionList

                          clip: true
                          anchors.fill: parent
                          model: commandInput.completions

                          delegate: ItemDelegate {
                            width: completionList.width
                            highlighted: ListView.isCurrentItem
                            onClicked: {
                              completionList.currentIndex = index
                              commandInput.acceptCompletion()
                            }

                            contentItem: Label {
                              text: modelData
                              elide: Text.ElideRight
                              font: Cpp_Misc_CommonFonts.monoFont
                            }
                          }
                        }
                      }
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      opacity: enabled ? 1 : 0.5
                      ToolTip.text: qsTr("Run command")
                      Layout.alignment: Qt.AlignVCenter
                      onClicked: terminalPane.runLine()
                      icon.source: "qrc:/icons/buttons/send.svg"
                      enabled: commandInput.text.trim().length > 0
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      opacity: enabled ? 1 : 0.5
                      onClicked: scrollback.clear()
                      enabled: scrollback.length > 0
                      Layout.alignment: Qt.AlignVCenter
                      ToolTip.text: qsTr("Clear output")
                      icon.source: "qrc:/icons/buttons/clear.svg"
                    }
                  }
                }
              }
            }

            //
            // Script tab
            //
            Item {
              id: scriptPane

              //
              // Echoes the run header, switches to the Terminal tab and dispatches the macro
              //
              function runMacro() {
                const lua = macroEditor.language === SerialStudio.Lua
                terminalPane.appendText("> [macro] run (" + (lua ? "lua" : "js") + ")")
                rightTabBar.currentIndex = 0
                if (lua)
                  macroRunner.runLua(macroEditor.text)
                else
                  macroRunner.runJs(macroEditor.text)
              }

              //
              // Compile-only check, result printed to the scrollback
              //
              function verifyMacro() {
                const verdict = macroRunner.verify(macroEditor.text, macroEditor.language)
                if (verdict.ok)
                  terminalPane.appendText(qsTr("[macro] verify: no syntax errors") + "\n")
                else
                  terminalPane.appendText(qsTr("[macro] verify failed: %1").arg(verdict.error) + "\n")
              }

              //
              // Runs an action directly, or after confirmation when the editor has edits
              //
              function confirmDiscard(action) {
                if (!macroEditor.isModified) {
                  action()
                  return
                }

                discardDialog.pendingAction = action
                discardDialog.open()
              }

              //
              // Unsaved-changes confirmation
              //
              Dialog {
                id: discardDialog

                modal: true
                title: qsTr("Discard changes?")
                anchors.centerIn: Overlay.overlay
                standardButtons: Dialog.Yes | Dialog.No
                onAccepted: {
                  if (pendingAction)
                    pendingAction()

                  pendingAction = null
                }

                property var pendingAction: null

                Label {
                  text: qsTr("The macro editor has unsaved changes.")
                }
              }

              ColumnLayout {
                spacing: 0
                anchors.fill: parent

                //
                // Macro toolbar
                //
                Rectangle {
                  implicitHeight: 40
                  Layout.fillWidth: true
                  color: Cpp_ThemeManager.colors["groupbox_background"]

                  Rectangle {
                    height: 1
                    width: parent.width
                    anchors.bottom: parent.bottom
                    color: Cpp_ThemeManager.colors["groupbox_border"]
                  }

                  RowLayout {
                    spacing: 4

                    anchors {
                      margins: 8
                      fill: parent
                      topMargin: 0
                      bottomMargin: 0
                    }

                    Widgets.Combo {
                      id: languageCombo

                      implicitHeight: 24
                      Layout.preferredWidth: 120
                      Layout.alignment: Qt.AlignVCenter
                      currentIndex: macroEditor.language
                      model: [qsTr("JavaScript"), qsTr("Lua")]
                      onActivated: (index) => {
                                     const pristine = macroEditor.text.trim() === ""
                                     || root.starterMacros.indexOf(macroEditor.text) >= 0
                                     macroEditor.setLanguage(index)
                                     if (pristine)
                                     root.seedStarterMacro()
                                   }
                    }

                    Item {
                      Layout.fillWidth: true
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      ToolTip.text: qsTr("Load macro")
                      Layout.alignment: Qt.AlignVCenter
                      icon.source: "qrc:/icons/buttons/open.svg"
                      onClicked: scriptPane.confirmDiscard(function() { macroRunner.loadMacro() })
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      ToolTip.text: qsTr("Save macro")
                      Layout.alignment: Qt.AlignVCenter
                      icon.source: "qrc:/icons/buttons/save.svg"
                      onClicked: macroRunner.saveMacro(macroEditor.text, macroEditor.language)
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      Layout.alignment: Qt.AlignVCenter
                      ToolTip.text: qsTr("Verify macro")
                      onClicked: scriptPane.verifyMacro()
                      icon.source: "qrc:/icons/buttons/apply.svg"
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      enabled: !macroRunner.busy
                      opacity: enabled ? 1 : 0.5
                      ToolTip.text: qsTr("Run macro")
                      onClicked: scriptPane.runMacro()
                      Layout.alignment: Qt.AlignVCenter
                      icon.source: "qrc:/icons/buttons/media-play.svg"
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      opacity: enabled ? 1 : 0.5
                      enabled: macroRunner.canStop
                      onClicked: macroRunner.stop()
                      ToolTip.text: qsTr("Stop macro")
                      Layout.alignment: Qt.AlignVCenter
                      icon.source: "qrc:/icons/buttons/media-stop.svg"
                    }

                    Widgets.IconButton {
                      iconSize: 16
                      implicitHeight: 24
                      Layout.maximumWidth: 24
                      Layout.alignment: Qt.AlignVCenter
                      ToolTip.text: qsTr("Clear editor")
                      icon.source: "qrc:/icons/buttons/clear.svg"
                      onClicked: scriptPane.confirmDiscard(function() { macroEditor.clear() })
                    }
                  }
                }

                //
                // Embedded code editor
                //
                MacroEditor {
                  id: macroEditor

                  Layout.margins: -1
                  Layout.fillWidth: true
                  Layout.fillHeight: true
                }
              }
            }
          }
        }
      }
    }
  }
}
