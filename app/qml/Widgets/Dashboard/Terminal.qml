/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
  // Super important to for shortcuts
  //
  onVisibleChanged: {
    if (visible)
      root.forceActiveFocus()
  }

  //
  // Custom properties
  //
  property int minimumRows: 24
  property int minimumColumns: 80

  //
  // Save settings
  //
  Settings {
    property alias echo: echoCheck.checked
    property alias hex: hexCheckbox.checked
    property alias timestamp: timestampCheck.checked
    property alias vt100Enabled: terminal.vt100emulation
    property alias lineEnding: lineEndingCombo.currentIndex
    property alias displayMode: displayModeCombo.currentIndex
  }

  //
  // Load welcome guide
  //
  function showWelcomeGuide() {
    clear()
    terminal.autoscroll = false
    Cpp_IO_Console.append(Cpp_Misc_Translator.welcomeConsoleText + "\n")
    terminal.autoscroll = true
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
    sequences: [StandardKey.SelectAll]
  } Shortcut {
    onActivated: root.copy()
    sequences: [StandardKey.Copy]
  } Shortcut {
    onActivated: Cpp_IO_Console.save()
    sequences: [StandardKey.Save, StandardKey.SaveAs]
  }

  //
  // Re-load welcome text when the language is changed
  //
  Component.onCompleted: root.showWelcomeGuide()
  Connections {
    target: Cpp_Misc_Translator
    function onLanguageChanged() {
      root.showWelcomeGuide()
    }
  }

  //
  // Re-load welcome text when Pro version is activated/deactivated
  //
  Loader {
    active: Cpp_QtCommercial_Available
    sourceComponent: Component {
      Connections {
        target: Cpp_Licensing_LemonSqueezy
        function onActivatedChanged() {
          root.showWelcomeGuide()
        }
      }
    }
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
    anchors.fill: parent
    anchors.topMargin: -6

    //
    // Console display
    //
    TerminalWidget {
      id: terminal
      vt100emulation: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumHeight: terminal.charHeight * root.minimumRows
      Layout.minimumWidth: terminal.charWidth * root.minimumColumns

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
      Layout.fillWidth: true

      Button {
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
            if (!Cpp_IO_Manager.connected)
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
          if (Cpp_IO_Console.dataMode !== checked)
            Cpp_IO_Console.dataMode = checked ? 1 : 0
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
    }

    //
    // Terminal output options
    //
    RowLayout {
      Layout.fillWidth: true

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
