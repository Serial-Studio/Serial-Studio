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

import SerialStudio
import ".." as Widgets

Item {
  id: root

  implicitWidth: layout.implicitWidth + 32
  implicitHeight: layout.implicitHeight + 32

  //
  // Widget data inputs (unused)
  //
  property var color
  property var model
  property var windowRoot
  property string widgetId

  //
  // Super important for shortcuts & VT-100 keyboard input
  //
  onVisibleChanged: {
    if (visible && !app.commandPaletteOpen) {
      if (root.vt100Interactive)
        Qt.callLater(terminal.forceActiveFocus)
      else
        Qt.callLater(root.forceActiveFocus)
    }
  }

  //
  // When a device connects in VT-100 mode, give the terminal widget focus
  // so the user can start typing immediately without clicking first.
  //
  Connections {
    target: Cpp_IO_Manager

    function onConnectedChanged() {
      if (Cpp_IO_Manager.isConnected && Cpp_Console_Handler.vt100Emulation
          && !app.commandPaletteOpen)
        Qt.callLater(terminal.forceActiveFocus)
    }
  }

  //
  // Custom properties
  //
  property bool minimal: true
  property int minimumRows: 24
  property bool searchOpen: false
  property int minimumColumns: 80
  property bool annotationsOpen: false
  readonly property int annotationsHeight: 260
  readonly property bool hasToolbar: consoleToolbar.visible
  property int minimumHeight: Cpp_Console_Handler.defaultCharHeight * root.minimumRows
  property int minimumWidth: Cpp_Console_Handler.defaultCharWidth * root.minimumColumns

  //
  // Rows the console gives up to the annotation panel: opening the panel takes its space from
  // the console instead of growing the widget, so the terminal's minimum size never moves.
  //
  readonly property int consoleMinimumHeight: {
    const base = Cpp_Console_Handler.defaultCharHeight * root.minimumRows
    const taken = root.annotationsOpen ? root.annotationsHeight + layout.spacing + 4 : 0
    return Math.max(0, base - taken)
  }

  //
  // Expose the live terminal dimensions (columns/rows) so parent items can
  // react to resize events and update the PTY size accordingly.
  //
  readonly property int terminalRows: terminal.terminalRows
  readonly property int terminalColumns: terminal.terminalColumns

  //
  // Function to send through serial port data
  //
  function sendData() {
    Cpp_Console_Handler.send(send.text)
    send.clear()
  }

  //
  // Clears console output
  //
  function clear() {
    Cpp_Console_Handler.clear()
    terminal.clear()
  }

  //
  // Copy function
  //
  function copy() {
    terminal.copy()
  }

  //
  // Select all text
  //
  function selectAll() {
    terminal.selectAll()
  }

  //
  // Closes the search bar, clears search state and returns focus to the terminal
  //
  function closeSearch() {
    root.searchOpen = false
    searchField.clear()
    terminal.clearSearch()
    terminal.forceActiveFocus()
  }

  //
  // VT-100 read-write mode forwards Ctrl+C / Ctrl+A as SIGINT / SOH.
  //
  readonly property bool vt100Interactive: Cpp_Console_Handler.vt100Emulation
                                           && Cpp_IO_Manager.readWrite

  Shortcut {
    onActivated: root.selectAll()
    sequences: [StandardKey.SelectAll]
    enabled: terminal.activeFocus && !root.vt100Interactive
  } Shortcut {
    onActivated: root.copy()
    sequences: [StandardKey.Copy]
    enabled: terminal.activeFocus && !root.vt100Interactive
  } Shortcut {
    sequences: [StandardKey.Find]
    enabled: terminal.activeFocus || searchField.activeFocus
    onActivated: {
      root.searchOpen = true
      searchField.forceActiveFocus()
      searchField.selectAll()
    }
  } Shortcut {
    sequence: "Escape"
    onActivated: root.closeSearch()
    enabled: root.searchOpen && (terminal.activeFocus || searchField.activeFocus)
  }

  //
  // Right-click context menu
  //
  Menu {
    id: contextMenu

    onClosed: terminal.forceActiveFocus()

    MenuItem {
      id: copyMenu

      text: qsTr("Copy")
      opacity: enabled ? 1 : 0.5
      onClicked: terminal.copy()
      enabled: terminal.copyAvailable
    }

    MenuItem {
      enabled: !terminal.empty
      text: qsTr("Select all")
      opacity: enabled ? 1 : 0.5
      onTriggered: terminal.selectAll()
    }

    MenuItem {
      text: qsTr("Clear")
      onTriggered: root.clear()
      opacity: enabled ? 1 : 0.5
    }
  }

  //
  // Controls
  //
  ColumnLayout {
    id: layout

    spacing: 4
    anchors.fill: parent

    //
    // Console behavior toolbar
    //
    Rectangle {
      id: consoleToolbar

      implicitHeight: 48
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["window_toolbar_background"]

      Rectangle {
        height: 1
        width: parent.width
        anchors.bottom: parent.bottom
        color: Cpp_ThemeManager.colors["window_border"]
      }

      //
      // Ribbon toolbar: never hides when the console is too narrow. Collapsible
      // sections fold to a compact icon+label dropdown first, then it scrolls.
      //
      Widgets.RibbonToolbar {
        id: consoleRibbon

        anchors.fill: parent
        anchors.rightMargin: 8
        secondaryToolbar: true
        fadeColor: Cpp_ThemeManager.colors["window_toolbar_background"]

        //
        // Console actions (collapsible): clear, find, collapse duplicates, annotations
        //
        Widgets.RibbonSection {
          collapsible: true
          collapsePriority: 10
          compactCollapse: true
          collapsedText: qsTr("Utilities")
          separatorColor: Cpp_ThemeManager.colors["widget_border"]
          collapsedIcon: Cpp_Misc_IconRegistry.icon("console", "utilities", 32)

          Widgets.IconButton {
            id: clearButton

            flat: true
            leftPadding: 8
            rightPadding: 8
            implicitHeight: 32
            text: qsTr("Clear")
            onClicked: root.clear()
            icon.color: "transparent"
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Clear console output")
            icon.source: Cpp_Misc_IconRegistry.icon("console", "clear", 32)
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
          }

          Widgets.IconButton {
            id: searchButton

            flat: true
            leftPadding: 8
            rightPadding: 8
            text: qsTr("Find")
            implicitHeight: 32
            checked: root.searchOpen
            icon.color: "transparent"
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Search console output")
            icon.source: Cpp_Misc_IconRegistry.icon("console", "search", 32)
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding

            onClicked: {
              if (root.searchOpen)
                root.closeSearch()

              else {
                root.searchOpen = true
                searchField.forceActiveFocus()
                searchField.selectAll()
              }
            }
          }

          Widgets.IconButton  {
            id: collapseButton

            flat: true
            leftPadding: 8
            rightPadding: 8
            implicitHeight: 32
            text: qsTr("Collapse")
            icon.color: "transparent"
            Layout.alignment: Qt.AlignVCenter
            checked: Cpp_Console_Handler.collapseDuplicates
            ToolTip.text: qsTr("Collapse repeated lines into a single entry")
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
            icon.source: Cpp_Misc_IconRegistry.icon("console", "collapse-duplicates", 32)
            onClicked: Cpp_Console_Handler.collapseDuplicates = !Cpp_Console_Handler.collapseDuplicates
          }

          Widgets.IconButton {
            id: annotationsButton

            flat: true
            leftPadding: 8
            rightPadding: 8
            implicitHeight: 32
            text: qsTr("Annotations")
            icon.color: "transparent"
            checked: root.annotationsOpen
            Layout.alignment: Qt.AlignVCenter
            onClicked: root.annotationsOpen = !root.annotationsOpen
            icon.source: Cpp_Misc_IconRegistry.icon("console", "annotations", 32)
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
            ToolTip.text: qsTr("Label the incoming bytes with a decoder and inspect them")
          }
        }

        //
        // Pause
        //
        Widgets.RibbonSection {
          showSeparator: false
          separatorColor: Cpp_ThemeManager.colors["widget_border"]

          Widgets.IconButton {
            id: pauseButton

            flat: true
            leftPadding: 8
            rightPadding: 8
            implicitHeight: 32
            checked: terminal.paused
            icon.color: "transparent"
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_IO_Manager.isConnected
            onClicked: terminal.paused = !terminal.paused
            text: terminal.paused ? qsTr("Resume") : qsTr("Pause")
            icon.source: terminal.paused
                         ? Cpp_Misc_IconRegistry.icon("commands", "resume", 16)
                         : Cpp_Misc_IconRegistry.icon("commands", "pause", 16)
            ToolTip.text: terminal.paused
                          ? qsTr("Resume console updates")
                          : qsTr("Freeze the console display (data keeps logging)")
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
          }
        }

        //
        // Spacer
        //
        Item {
          Layout.fillWidth: true
        }

        //
        // Display mode (collapsible): plain text, hex
        //
        Widgets.RibbonSection {
          collapsible: true
          collapsePriority: 20
          compactCollapse: true
          collapsedText: qsTr("Format")
          separatorColor: Cpp_ThemeManager.colors["widget_border"]
          collapsedIcon: Cpp_Misc_IconRegistry.icon("console", "plain-text", 16)

          Widgets.IconButton {
            flat: true
            leftPadding: 8
            rightPadding: 8
            text: qsTr("Text")
            implicitHeight: 32
            icon.color: "transparent"
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Plain text display mode")
            checked: Cpp_Console_Handler.displayMode === 0
            onClicked: Cpp_Console_Handler.displayMode = 0
            icon.source: Cpp_Misc_IconRegistry.icon("console", "plain-text", 16)
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
          }

          Widgets.IconButton {
            flat: true
            leftPadding: 8
            rightPadding: 8
            text: qsTr("Hex")
            implicitHeight: 32
            icon.color: "transparent"
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Hex display mode")
            checked: Cpp_Console_Handler.displayMode === 1
            onClicked: Cpp_Console_Handler.displayMode = 1
            icon.source: Cpp_Misc_IconRegistry.icon("console", "hexadecimal", 16)
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
          }
        }

        //
        // Settings (always visible)
        //
        Widgets.RibbonSection {
          showSeparator: false

          Widgets.IconButton {
            id: settingsButton

            flat: true
            leftPadding: 8
            rightPadding: 8
            implicitHeight: 32
            text: qsTr("Settings")
            icon.color: "transparent"
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Console settings")
            icon.source: Cpp_Misc_IconRegistry.icon("console", "settings", 32)
            Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
            onClicked: settingsPopup.visible ? settingsPopup.close() : settingsPopup.open()

            Popup {
              id: settingsPopup

              margins: 8
              padding: 12
              y: settingsButton.height + 4
              x: settingsButton.width - width
              closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

              background: Rectangle {
                radius: 4
                opacity: 0.95
                color: Cpp_ThemeManager.colors["window"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              contentItem: ColumnLayout {
                spacing: 4

                CheckBox {
                  id: timestampCheck

                  onCheckedChanged: {
                    if (Cpp_Console_Handler.showTimestamp !== checked)
                      Cpp_Console_Handler.showTimestamp = checked
                  }
                  text: qsTr("Show Timestamp")
                  checked: Cpp_Console_Handler.showTimestamp
                }

                CheckBox {
                  id: echoCheck

                  text: qsTr("Echo")
                  onCheckedChanged: {
                    if (Cpp_Console_Handler.echo !== checked)
                      Cpp_Console_Handler.echo = checked
                  }
                  checked: Cpp_Console_Handler.echo
                }

                CheckBox {
                  id: vt100Check

                  onCheckedChanged: {
                    if (Cpp_Console_Handler.vt100Emulation !== checked)
                      Cpp_Console_Handler.vt100Emulation = checked
                  }
                  opacity: enabled ? 1 : 0.8
                  text: qsTr("Emulate VT-100")
                  checked: Cpp_Console_Handler.vt100Emulation
                  enabled: !Cpp_Console_Handler.imageWidgetActive
                }

                CheckBox {
                  id: ansiColorsCheck

                  onCheckedChanged: {
                    if (enabled && Cpp_Console_Handler.ansiColors !== checked)
                      Cpp_Console_Handler.ansiColors = checked
                  }
                  text: qsTr("ANSI Colors")
                  opacity: enabled ? 1 : 0.8
                  checked: Cpp_Console_Handler.vt100Emulation && Cpp_Console_Handler.ansiColors
                  enabled: Cpp_Console_Handler.vt100Emulation && !Cpp_Console_Handler.imageWidgetActive
                }

                RowLayout {
                  spacing: 4

                  Label {
                    text: qsTr("Scrollback Lines")
                    Layout.alignment: Qt.AlignVCenter
                  }

                  SpinBox {
                    id: scrollbackSpin

                    from: 100
                    to: 100000
                    stepSize: 100
                    editable: true
                    Layout.fillWidth: true
                    value: Cpp_Console_Handler.scrollbackLines
                    onValueModified: Cpp_Console_Handler.scrollbackLines = value

                    Connections {
                      target: Cpp_Console_Handler
                      function onScrollbackLinesChanged() {
                        scrollbackSpin.value = Cpp_Console_Handler.scrollbackLines
                      }
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
    // Console display
    //
    TerminalWidget {
      id: terminal

      Layout.leftMargin: 8
      Layout.rightMargin: 8
      Layout.fillWidth: true
      Layout.fillHeight: true
      ansiColors: Cpp_Console_Handler.ansiColors
      Layout.topMargin: consoleToolbar.visible ? 4 : 8
      vt100emulation: Cpp_Console_Handler.vt100Emulation
      Layout.minimumHeight: root.minimal ? 0 : root.consoleMinimumHeight
      Layout.minimumWidth: root.minimal ? 0 : Cpp_Console_Handler.defaultCharWidth * root.minimumColumns

      Rectangle {
        border.width: 1
        color: "transparent"
        anchors.fill: parent
        border.color: Cpp_ThemeManager.colors["console_border"]
      }

      MouseArea {
        id: mouseArea

        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        propagateComposedEvents: true
        acceptedButtons: Qt.RightButton

        onClicked: (mouse) => {
                     if (mouse.button === Qt.RightButton) {
                       contextMenu.popup()
                       mouse.accepted = true
                     }
                   }
      }

      //
      // In-console search bar (Ctrl+F / Cmd+F)
      //
      Rectangle {
        id: searchBar

        radius: 4
        border.width: 1
        anchors.margins: 8
        anchors.top: parent.top
        visible: root.searchOpen
        anchors.right: parent.right
        implicitHeight: searchRow.implicitHeight + 8
        implicitWidth: searchRow.implicitWidth + 16
        color: Cpp_ThemeManager.colors["console_base"]
        border.color: Cpp_ThemeManager.colors["console_border"]

        RowLayout {
          id: searchRow

          spacing: 4
          anchors.centerIn: parent

          Widgets.LineField {
            id: searchField

            implicitWidth: 164
            implicitHeight: 24
            placeholderText: qsTr("Find in console") + "..."
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

            onTextChanged: terminal.setSearchQuery(text, caseButton.checked)

            Keys.onReturnPressed: (event) => {
                                    if (event.modifiers & Qt.ShiftModifier)
                                    terminal.searchPrevious()
                                    else
                                    terminal.searchNext()
                                  }

            Keys.onEnterPressed: (event) => {
                                   if (event.modifiers & Qt.ShiftModifier)
                                   terminal.searchPrevious()
                                   else
                                   terminal.searchNext()
                                 }

            Keys.onEscapePressed: root.closeSearch()
          }

          Label {
            visible: searchField.length > 0
            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["console_text"]
            text: terminal.searchMatchCount > 0
                  ? qsTr("%1 of %2").arg(terminal.searchCurrentMatch).arg(terminal.searchMatchCount)
                  : qsTr("No results")
          }

          Button {
            id: caseButton

            text: "Aa"
            checkable: true
            implicitWidth: 32
            implicitHeight: 24
            ToolTip.delay: 500
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Match case")
            checked: Cpp_Console_Handler.searchCaseSensitive
            onCheckedChanged: {
              if (Cpp_Console_Handler.searchCaseSensitive !== checked)
                Cpp_Console_Handler.searchCaseSensitive = checked

              terminal.setSearchQuery(searchField.text, checked)
            }
          }

          Widgets.IconButton {
            iconSize: 16
            implicitHeight: 24
            Layout.maximumWidth: 24
            opacity: enabled ? 1 : 0.5
            ToolTip.text: qsTr("Previous match")
            onClicked: terminal.searchPrevious()
            enabled: terminal.searchMatchCount > 0
            icon.source: "qrc:/icons/buttons/backward.svg"
          }

          Widgets.IconButton {
            iconSize: 16
            implicitHeight: 24
            Layout.maximumWidth: 24
            opacity: enabled ? 1 : 0.5
            ToolTip.text: qsTr("Next match")
            onClicked: terminal.searchNext()
            enabled: terminal.searchMatchCount > 0
            icon.source: "qrc:/icons/buttons/forward.svg"
          }

          Widgets.IconButton {
            iconSize: 16
            implicitHeight: 24
            Layout.maximumWidth: 24
            ToolTip.text: qsTr("Close search")
            onClicked: root.closeSearch()
            icon.source: "qrc:/icons/buttons/close.svg"
          }
        }
      }
    }

    //
    // Frame annotation layer (track / table / payload / decoder), toggled from the toolbar
    //
    ConsoleAnnotations {
      id: annotationsPanel

      Layout.leftMargin: 8
      Layout.rightMargin: 8
      Layout.fillWidth: true
      Layout.minimumHeight: 0
      visible: root.annotationsOpen
      Layout.topMargin: root.annotationsOpen ? 4 : 0
      Layout.preferredHeight: root.annotationsHeight
    }

    //
    // Data-write controls
    //
    RowLayout {
      id: sendCtrl

      Layout.leftMargin: 8
      Layout.rightMargin: 8
      Layout.bottomMargin: 8
      Layout.fillWidth: true
      visible: root.width > implicitWidth

      Widgets.IconButton {
        iconSize: 16
        id: ftButton

        implicitHeight: 24
        Layout.maximumWidth: 24
        visible: !app.runtimeMode
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        onClicked: app.showFileTransmission()
        icon.source: "qrc:/icons/buttons/attach.svg"
        ToolTip.text: qsTr("Send a file to the connected device")
      }

      Widgets.Combo {
        id: deviceCombo

        Layout.maximumWidth: 144
        Layout.preferredWidth: 144
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.deviceNames
        visible: Cpp_Console_Handler.multiDeviceMode
        onActivated: (index) => Cpp_Console_Handler.setCurrentDeviceIndex(index)

        //
        // Mirror the C++ device selection (restored on connect) into the combobox
        //
        function syncIndex() {
          const idx = Cpp_Console_Handler.currentDeviceIndex
          if (idx >= 0 && idx < deviceCombo.count)
            deviceCombo.currentIndex = idx
        }

        Component.onCompleted: syncIndex()

        Connections {
          target: Cpp_Console_Handler
          function onCurrentDeviceIdChanged() { deviceCombo.syncIndex() }
          function onDeviceNamesChanged() { deviceCombo.syncIndex() }
        }
      }

      Widgets.LineField {
        id: send

        implicitWidth: 64
        implicitHeight: 24
        Layout.fillWidth: true
        Layout.minimumWidth: 96
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.monoFont
        placeholderText: qsTr("Send Data to Device") + "..."
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
        // Send data on <enter>
        //
        Keys.onReturnPressed: root.sendData()

        //
        // Add space automatically in hex view
        //
        onTextChanged: {
          if (hexCheckbox.checked) {
            const currentCursorPosition = send.cursorPosition;
            const cursorAtEnd = (currentCursorPosition === send.text.length);

            //
            // Format the text
            //
            const originalText = send.text;
            const formattedText = Cpp_Console_Handler.formatUserHex(send.text);
            const isValid = Cpp_Console_Handler.validateUserHex(formattedText);

            if (originalText !== formattedText) {
              send.text = formattedText;

              //
              // Restore the cursor position, adjusting for added spaces
              //
              if (!cursorAtEnd) {
                //
                // Remove spaces from originalText and formattedText to compare lengths
                //
                const cleanedOriginalText = originalText.replace(/ /g, '');
                const cleanedFormattedText = formattedText.replace(/ /g, '');

                //
                // Calculate the difference in length due to formatting
                //
                const lengthDifference = cleanedFormattedText.length - cleanedOriginalText.length;

                //
                // Count spaces before the cursor in both texts
                //
                let spacesBeforeCursorOriginal = (originalText.slice(0, currentCursorPosition).match(/ /g) || []).length;
                let spacesBeforeCursorFormatted = (formattedText.slice(0, currentCursorPosition).match(/ /g) || []).length;

                //
                // Calculate adjustment factor
                //
                const adjustment = spacesBeforeCursorFormatted - spacesBeforeCursorOriginal + lengthDifference;

                //
                // Restore the cursor position with adjustment
                //
                send.cursorPosition = Math.min(currentCursorPosition + adjustment, send.text.length);
              }
            }

            send.palette.text = isValid
                ? Cpp_ThemeManager.colors["console_text"]
                : Cpp_ThemeManager.colors["alarm"];
          }
        }

        //
        // Navigate command history upwards with <up>
        //
        Keys.onUpPressed: {
          Cpp_Console_Handler.historyUp()
          send.text = Cpp_Console_Handler.currentHistoryString
        }

        //
        // Navigate command history downwards with <down>
        //
        Keys.onDownPressed: {
          Cpp_Console_Handler.historyDown()
          send.text = Cpp_Console_Handler.currentHistoryString
        }

        //
        // Clear text when device is disconnected
        //
        Connections {
          target: Cpp_IO_Manager

          function onConnectedChanged() {
            if (!Cpp_IO_Manager.isConnected)
              send.clear()
          }
        }
      }

      CheckBox {
        id: hexCheckbox

        text: qsTr("Hex")
        onCheckedChanged: {
          const newValue = checked ? 1 : 0
          if (Cpp_Console_Handler.dataMode !== newValue)
            Cpp_Console_Handler.dataMode = newValue
        }
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        checked: Cpp_Console_Handler.dataMode === 1
      }

      Widgets.Combo {
        id: lineEndingCombo

        Layout.minimumWidth: 96
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.lineEndings
        currentIndex: Cpp_Console_Handler.lineEnding

        onActivated: (index) => {
                       if (Cpp_Console_Handler.lineEnding !== index)
                       Cpp_Console_Handler.lineEnding = index
                     }
      }

      Widgets.Combo {
        id: checkumCombo

        Layout.minimumWidth: 96
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.checksumMethods
        currentIndex: Cpp_Console_Handler.checksumMethod

        onActivated: (index) => {
                       if (Cpp_Console_Handler.checksumMethod !== index)
                       Cpp_Console_Handler.checksumMethod = index
                     }
      }

      Widgets.IconButton {
        iconSize: 16
        id: sendBt

        implicitHeight: 24
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        onClicked: root.sendData()
        icon.source: "qrc:/icons/buttons/send.svg"
        ToolTip.text: qsTr("Send data to the device")
        enabled: Cpp_IO_Manager.readWrite && (send.length > 0 || Cpp_Console_Handler.lineEnding != 0)
      }
    }

    //
    // Placeholder that preserves the send-row footprint when the controls are
    // hidden (too narrow), keeping the terminal border spacing constant.
    //
    Item {
      Layout.fillWidth: true
      Layout.preferredHeight: 4
      visible: !sendCtrl.visible
    }
  }
}
