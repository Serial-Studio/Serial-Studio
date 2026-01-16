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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root
  implicitWidth: layout.implicitWidth + 32
  implicitHeight: layout.implicitHeight + 32
  property alias vt100emulation: terminal.vt100emulation
  property alias ansiColors: terminal.ansiColors

  //
  // Widget data inputs (unused)
  //
  property var color
  property var model
  property var windowRoot

  //
  // Super important to for shortcuts
  //
  onVisibleChanged: {
    if (visible)
      Qt.callLater(root.forceActiveFocus)
  }

  //
  // Custom properties
  //
  property bool minimal: true
  property int minimumRows: 24
  property int minimumColumns: 80
  property int minimumHeight: Cpp_Console_Handler.defaultCharHeight * root.minimumRows
  property int minimumWidth: Cpp_Console_Handler.defaultCharWidth * root.minimumColumns

  //
  // Save settings
  //
  Settings {
    property alias echo: echoCheck.checked
    property alias hex: hexCheckbox.checked
    property alias timestamp: timestampCheck.checked
    property alias checksum: checkumCombo.currentIndex
    property alias vt100Enabled: terminal.vt100emulation
    property alias ansiColorsEnabled: terminal.ansiColors
    property alias lineEnding: lineEndingCombo.currentIndex
    property alias displayMode: displayModeCombo.currentIndex
  }

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
  // Shortcuts
  //
  Shortcut {
    onActivated: root.selectAll()
    enabled: terminal.activeFocus
    sequences: [StandardKey.SelectAll]
  } Shortcut {
    onActivated: root.copy()
    enabled: terminal.activeFocus
    sequences: [StandardKey.Copy]
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
      text: qsTr("Select all")
      enabled: !terminal.empty
      opacity: enabled ? 1 : 0.5
      onTriggered: terminal.selectAll()
    }

    MenuItem {
      text: qsTr("Clear")
      opacity: enabled ? 1 : 0.5
      onTriggered: root.clear()
    }
  }

  //
  // Controls
  //
  ColumnLayout {
    id: layout
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    //
    // Console display
    //
    TerminalWidget {
      id: terminal
      ansiColors: true
      vt100emulation: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumHeight: root.minimal ? 0 : Cpp_Console_Handler.defaultCharHeight * root.minimumRows
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
    }

    //
    // Data-write controls
    //
    RowLayout {
      id: sendCtrl
      Layout.fillWidth: true
      visible: root.width > implicitWidth

      Button {
        id: ftButton
        icon.width: 18
        icon.height: 18
        implicitHeight: 24
        Layout.maximumWidth: 24
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        enabled: Cpp_IO_Manager.readWrite
        onClicked: app.showFileTransmission()
        icon.source: "qrc:/rcc/icons/buttons/attach.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }

      TextField {
        id: send
        implicitWidth: 128
        implicitHeight: 24
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        enabled: Cpp_IO_Manager.readWrite
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
            // Get the current cursor position
            const currentCursorPosition = send.cursorPosition;
            const cursorAtEnd = (currentCursorPosition === send.text.length);

            // Format the text
            const originalText = send.text;
            const formattedText = Cpp_Console_Handler.formatUserHex(send.text);
            const isValid = Cpp_Console_Handler.validateUserHex(formattedText);

            // Update the text only if it has changed
            if (originalText !== formattedText) {
              send.text = formattedText;

              // Restore the cursor position, adjusting for added spaces
              if (!cursorAtEnd) {
                // Remove spaces from originalText and formattedText to compare lengths
                const cleanedOriginalText = originalText.replace(/ /g, '');
                const cleanedFormattedText = formattedText.replace(/ /g, '');

                // Calculate the difference in length due to formatting
                const lengthDifference = cleanedFormattedText.length - cleanedOriginalText.length;

                // Count spaces before the cursor in both texts
                let spacesBeforeCursorOriginal = (originalText.slice(0, currentCursorPosition).match(/ /g) || []).length;
                let spacesBeforeCursorFormatted = (formattedText.slice(0, currentCursorPosition).match(/ /g) || []).length;

                // Calculate adjustment factor
                const adjustment = spacesBeforeCursorFormatted - spacesBeforeCursorOriginal + lengthDifference;

                // Restore the cursor position with adjustment
                send.cursorPosition = Math.min(currentCursorPosition + adjustment, send.text.length);
              }
            }

            // Update the palette based on validation
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
        text: "HEX"
        id: hexCheckbox
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        enabled: Cpp_IO_Manager.readWrite
        checked: Cpp_Console_Handler.dataMode === 1
        onCheckedChanged: {
          const newValue = checked ? 1 : 0
          if (Cpp_Console_Handler.dataMode !== newValue)
            Cpp_Console_Handler.dataMode = newValue
        }
      }

      ComboBox {
        id: lineEndingCombo
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.lineEndings
        currentIndex: Cpp_Console_Handler.lineEnding
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_Console_Handler.lineEnding)
            Cpp_Console_Handler.lineEnding = currentIndex
        }
      }

      ComboBox {
        id: checkumCombo
        Layout.minimumWidth: 128
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.checksumMethods
        currentIndex: Cpp_Console_Handler.checksumMethod
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_Console_Handler.checksumMethod)
            Cpp_Console_Handler.checksumMethod = currentIndex
        }
      }

      Button {
        id: sendBt
        icon.width: 18
        icon.height: 18
        implicitHeight: 24
        Layout.maximumWidth: 32
        onClicked: root.sendData()
        opacity: enabled ? 1 : 0.5
        icon.source: "qrc:/rcc/icons/buttons/send.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        enabled: Cpp_IO_Manager.readWrite && (send.length > 0 || Cpp_Console_Handler.lineEnding != 0)
      }
    }

    //
    // Terminal output options
    //
    RowLayout {
      Layout.fillWidth: true
      visible: root.width > implicitWidth

      CheckBox {
        id: timestampCheck
        text: qsTr("Show Timestamp")
        Layout.alignment: Qt.AlignVCenter
        checked: Cpp_Console_Handler.showTimestamp
        onCheckedChanged: {
          if (Cpp_Console_Handler.showTimestamp !== checked)
            Cpp_Console_Handler.showTimestamp = checked
        }
      }

      CheckBox {
        id: echoCheck
        text: qsTr("Echo")
        checked: Cpp_Console_Handler.echo
        Layout.alignment: Qt.AlignVCenter
        onCheckedChanged: {
          if (Cpp_Console_Handler.echo !== checked)
            Cpp_Console_Handler.echo = checked
        }
      }

      CheckBox {
        id: vt100Check
        text: qsTr("Emulate VT-100")
        Layout.alignment: Qt.AlignVCenter
        checked: terminal.vt100emulation
        onCheckedChanged: {
          if (terminal.vt100emulation !== checked)
            terminal.vt100emulation = checked
        }
      }

      CheckBox {
        id: ansiColorsCheck
        text: qsTr("ANSI Colors")
        Layout.alignment: Qt.AlignVCenter
        checked: terminal.ansiColors
        onCheckedChanged: {
          if (terminal.ansiColors !== checked)
            terminal.ansiColors = checked
        }
      }

      Item {
        Layout.fillWidth: true
      }

      ComboBox {
        id: displayModeCombo

        implicitHeight: 24
        Layout.fillWidth: true
        Layout.maximumWidth: 164
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.displayModes
        currentIndex: Cpp_Console_Handler.displayMode
        displayText: qsTr("Display: %1").arg(currentText)
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_Console_Handler.displayMode)
            Cpp_Console_Handler.displayMode = currentIndex
        }
      }

      Button {
        icon.width: 18
        icon.height: 18
        implicitHeight: 24
        onClicked: root.clear()
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        icon.source: "qrc:/rcc/icons/buttons/clear.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }
    }
  }
}
