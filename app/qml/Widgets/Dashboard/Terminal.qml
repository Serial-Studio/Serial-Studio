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
      root.forceActiveFocus()
  }

  //
  // Custom properties
  //
  property bool minimal: true
  property int minimumRows: 24
  property int minimumColumns: 80
  property int minimumHeight: terminal.charHeight * root.minimumRows
  property int minimumWidth: terminal.charWidth * root.minimumColumns

  //
  // Save settings
  //
  Settings {
    property alias echo: echoCheck.checked
    property alias hex: hexCheckbox.checked
    property alias timestamp: timestampCheck.checked
    property alias checksum: checkumCombo.currentIndex
    property alias vt100Enabled: terminal.vt100emulation
    property alias lineEnding: lineEndingCombo.currentIndex
    property alias displayMode: displayModeCombo.currentIndex
  }

  //
  // Function to send through serial port data
  //
  function sendData() {
    Cpp_IO_Console.send(send.text)
    send.clear()
  }

  //
  // Clears console output
  //
  function clear() {
    Cpp_IO_Console.clear()
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
      vt100emulation: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumHeight: root.minimal ? 0 : terminal.charHeight * root.minimumRows
      Layout.minimumWidth: root.minimal ? 0 : terminal.charWidth * root.minimumColumns

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
        font: terminal.font
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        enabled: Cpp_IO_Manager.readWrite
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
            const formattedText = Cpp_IO_Console.formatUserHex(send.text);
            const isValid = Cpp_IO_Console.validateUserHex(formattedText);

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
          Cpp_IO_Console.historyUp()
          send.text = Cpp_IO_Console.currentHistoryString
        }

        //
        // Navigate command history downwards with <down>
        //
        Keys.onDownPressed: {
          Cpp_IO_Console.historyDown()
          send.text = Cpp_IO_Console.currentHistoryString
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
        checked: Cpp_IO_Console.dataMode === 1
        onCheckedChanged: {
          const newValue = checked ? 1 : 0
          if (Cpp_IO_Console.dataMode !== newValue)
            Cpp_IO_Console.dataMode = newValue
        }
      }

      ComboBox {
        id: lineEndingCombo
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_IO_Console.lineEndings
        currentIndex: Cpp_IO_Console.lineEnding
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_Console.lineEnding)
            Cpp_IO_Console.lineEnding = currentIndex
        }
      }

      ComboBox {
        id: checkumCombo
        Layout.minimumWidth: 128
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_IO_Console.checksumMethods
        currentIndex: Cpp_IO_Console.checksumMethod
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_Console.checksumMethod)
            Cpp_IO_Console.checksumMethod = currentIndex
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
        enabled: Cpp_IO_Manager.readWrite && (send.length > 0 || Cpp_IO_Console.lineEnding != 0)
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
        checked: Cpp_IO_Console.showTimestamp
        onCheckedChanged: {
          if (Cpp_IO_Console.showTimestamp !== checked)
            Cpp_IO_Console.showTimestamp = checked
        }
      }

      CheckBox {
        id: echoCheck
        text: qsTr("Echo")
        checked: Cpp_IO_Console.echo
        Layout.alignment: Qt.AlignVCenter
        onCheckedChanged: {
          if (Cpp_IO_Console.echo !== checked)
            Cpp_IO_Console.echo = checked
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

      Item {
        Layout.fillWidth: true
      }

      ComboBox {
        id: displayModeCombo

        implicitHeight: 24
        Layout.fillWidth: true
        Layout.maximumWidth: 164
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_IO_Console.displayModes
        currentIndex: Cpp_IO_Console.displayMode
        displayText: qsTr("Display: %1").arg(currentText)
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_Console.displayMode)
            Cpp_IO_Console.displayMode = currentIndex
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
