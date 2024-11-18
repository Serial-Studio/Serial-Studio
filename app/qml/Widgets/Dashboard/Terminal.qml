/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
  } Shortcut {
    onActivated: Cpp_IO_Console.print()
    sequences: [StandardKey.Print]
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
      enabled: Cpp_IO_Console.saveAvailable
    }

    MenuSeparator {}

    MenuItem {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Print")
      enabled: Cpp_IO_Console.saveAvailable
      onTriggered: Cpp_IO_Console.print()
    }

    MenuItem {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Save as") + "..."
      onTriggered: Cpp_IO_Console.save()
      enabled: Cpp_IO_Console.saveAvailable
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
          if (hexCheckbox.checked)
            send.text = Cpp_IO_Console.formatUserHex(send.text)
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
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        onClicked: Cpp_IO_Console.save()
        enabled: Cpp_IO_Console.saveAvailable
        icon.source: "qrc:/rcc/icons/buttons/save.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }

      Button {
        icon.width: 18
        icon.height: 18
        implicitHeight: 24
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        onClicked: Cpp_IO_Console.print()
        enabled: Cpp_IO_Console.saveAvailable
        icon.source: "qrc:/rcc/icons/buttons/print.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }

      Button {
        icon.width: 18
        icon.height: 18
        implicitHeight: 24
        onClicked: root.clear()
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Console.saveAvailable
        icon.source: "qrc:/rcc/icons/buttons/clear.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }
    }
  }
}
