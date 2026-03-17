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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

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
    if (visible) {
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
      if (Cpp_IO_Manager.isConnected && Cpp_Console_Handler.vt100Emulation)
        Qt.callLater(terminal.forceActiveFocus)
    }
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
  // Expose the live terminal dimensions (columns/rows) so parent items can
  // react to resize events and update the PTY size accordingly.
  //
  readonly property int terminalColumns: terminal.terminalColumns
  readonly property int terminalRows: terminal.terminalRows

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
  // When VT-100 emulation is active in read-write mode, disable clipboard
  // shortcuts so that Ctrl+C / Ctrl+A reach keyPressEvent() as control
  // codes (SIGINT, SOH).  Copy / Select All remain available via the
  // right-click context menu.
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
    anchors.margins: 8
    anchors.fill: parent

    //
    // Console display
    //
    TerminalWidget {
      id: terminal

      Layout.fillWidth: true
      Layout.fillHeight: true
      ansiColors: Cpp_Console_Handler.ansiColors
      vt100emulation: Cpp_Console_Handler.vt100Emulation
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
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        onClicked: app.showFileTransmission()
        icon.source: "qrc:/rcc/icons/buttons/attach.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }

      ComboBox {
        id: deviceCombo

        visible: Cpp_Console_Handler.multiDeviceMode
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.deviceNames
        onCurrentIndexChanged: {
          if (visible)
            Cpp_Console_Handler.setCurrentDeviceIndex(currentIndex)
        }
      }

      TextField {
        id: send

        implicitWidth: 128
        implicitHeight: 24
        Layout.fillWidth: true
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
        id: hexCheckbox

        text: "HEX"
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

      ComboBox {
        id: lineEndingCombo

        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_Console_Handler.lineEnding)
            Cpp_Console_Handler.lineEnding = currentIndex
        }
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.lineEndings
        currentIndex: Cpp_Console_Handler.lineEnding
      }

      ComboBox {
        id: checkumCombo

        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_Console_Handler.checksumMethod)
            Cpp_Console_Handler.checksumMethod = currentIndex
        }
        Layout.minimumWidth: 128
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.checksumMethods
        currentIndex: Cpp_Console_Handler.checksumMethod
      }

      Button {
        id: sendBt

        icon.width: 18
        icon.height: 18
        implicitHeight: 24
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        onClicked: root.sendData()
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

        onCheckedChanged: {
          if (Cpp_Console_Handler.showTimestamp !== checked)
            Cpp_Console_Handler.showTimestamp = checked
        }
        text: qsTr("Show Timestamp")
        Layout.alignment: Qt.AlignVCenter
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
        Layout.alignment: Qt.AlignVCenter
      }

      CheckBox {
        id: vt100Check

        onCheckedChanged: {
          if (Cpp_Console_Handler.vt100Emulation !== checked)
            Cpp_Console_Handler.vt100Emulation = checked
        }
        opacity: enabled ? 1 : 0.8
        text: qsTr("Emulate VT-100")
        Layout.alignment: Qt.AlignVCenter
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
        Layout.alignment: Qt.AlignVCenter
        checked: Cpp_Console_Handler.vt100Emulation && Cpp_Console_Handler.ansiColors
        enabled: Cpp_Console_Handler.vt100Emulation && !Cpp_Console_Handler.imageWidgetActive
      }

      Item {
        Layout.fillWidth: true
      }

      ComboBox {
        id: displayModeCombo

        implicitHeight: 24
        Layout.fillWidth: true
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_Console_Handler.displayMode)
            Cpp_Console_Handler.displayMode = currentIndex
        }
        Layout.maximumWidth: 164
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_Console_Handler.displayModes
        currentIndex: Cpp_Console_Handler.displayMode
        displayText: qsTr("Display: %1").arg(currentText)
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
