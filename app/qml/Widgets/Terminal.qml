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
import SerialStudio as SerialStudio

Item {
  id: root
  property alias widgetEnabled: textEdit.widgetEnabled
  property alias vt100emulation: textEdit.vt100emulation

  //
  // Save settings
  //
  Settings {
    property alias hex: hexCheckbox.checked
    property alias echo: echoCheckbox.checked
    property alias timestamp: timestampCheck.checked
    property alias autoscroll: autoscrollCheck.checked
    property alias vt100Enabled: textEdit.vt100emulation
    property alias lineEnding: lineEndingCombo.currentIndex
    property alias displayMode: displayModeCombo.currentIndex
  }

  //
  // Load welcome guide
  //
  function showWelcomeGuide() {
    clear()
    Cpp_IO_Console.append(Cpp_Misc_Translator.welcomeConsoleText() + "\n")
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
    textEdit.clear()
  }

  //
  // Copy function
  //
  function copy() {
    textEdit.copy()
  }

  //
  // Select all text
  //
  function selectAll() {
    textEdit.selectAll()
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

    MenuItem {
      id: copyMenu
      text: qsTr("Copy")
      opacity: enabled ? 1 : 0.5
      onClicked: textEdit.copy()
      enabled: textEdit.copyAvailable
    }

    MenuItem {
      text: qsTr("Select all")
      enabled: !textEdit.empty
      opacity: enabled ? 1 : 0.5
      onTriggered: textEdit.selectAll()
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
    spacing: 4
    anchors.fill: parent
    anchors.topMargin: -6

    //
    // Console display
    //
    SerialStudio.Terminal {
      id: textEdit
      focus: true
      readOnly: true
      vt100emulation: true
      centerOnScroll: false
      undoRedoEnabled: false
      Layout.fillWidth: true
      Layout.fillHeight: true
      maximumBlockCount: 12000
      font: Cpp_Misc_CommonFonts.monoFont
      autoscroll: Cpp_IO_Console.autoscroll
      renderTarget: PaintedItem.FramebufferObject
      wordWrapMode: Text.WrapAtWordBoundaryOrAnywhere
      placeholderText: qsTr("No data received so far") + "..."

      MouseArea {
        id: mouseArea
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        propagateComposedEvents: true
        acceptedButtons: Qt.RightButton
        anchors.rightMargin: textEdit.scrollbarWidth

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

      TextField {
        id: send
        height: 24
        font: textEdit.font
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        enabled: Cpp_IO_Manager.readWrite
        placeholderText: qsTr("Send Data to Device") + "..."

        palette.base: Cpp_ThemeManager.colors["console_base"]
        palette.text: Cpp_ThemeManager.colors["console_text"]
        palette.button: Cpp_ThemeManager.colors["console_button"]
        palette.window: Cpp_ThemeManager.colors["console_window"]
        palette.buttonText: Cpp_ThemeManager.colors["console_button_text"]
        palette.placeholderText: Cpp_ThemeManager.colors["console_placeholder_text"]
        palette.highlightedText: Cpp_ThemeManager.colors["console_highlighted_text"]

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
        onCheckedChanged: Cpp_IO_Console.dataMode = checked ? 1 : 0
      }

      CheckBox {
        visible: false
        text: qsTr("Echo")
        id: echoCheckbox
        opacity: enabled ? 1 : 0.5
        checked: Cpp_IO_Console.echo
        Layout.alignment: Qt.AlignVCenter
        enabled: Cpp_IO_Manager.readWrite
        onCheckedChanged: {
          if (Cpp_IO_Console.echo !== checked)
            Cpp_IO_Console.echo = checked
        }
      }

      ComboBox {
        id: lineEndingCombo
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Manager.readWrite
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_IO_Console.lineEndings()
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
        id: autoscrollCheck
        text: qsTr("Autoscroll")
        Layout.alignment: Qt.AlignVCenter
        checked: Cpp_IO_Console.autoscroll
        onCheckedChanged: {
          if (Cpp_IO_Console.autoscroll !== checked)
            Cpp_IO_Console.autoscroll = checked
        }
      }

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
        id: vt100Check
        text: qsTr("Emulate VT-100")
        Layout.alignment: Qt.AlignVCenter
        checked: textEdit.vt100emulation
        onCheckedChanged: {
          if (textEdit.vt100emulation !== checked)
            textEdit.vt100emulation = checked
        }
      }

      Item {
        Layout.fillWidth: true
      }

      ComboBox {
        id: displayModeCombo
        Layout.fillWidth: true
        Layout.maximumWidth: 164
        Layout.alignment: Qt.AlignVCenter
        model: Cpp_IO_Console.displayModes()
        currentIndex: Cpp_IO_Console.displayMode
        displayText: qsTr("Display: %1").arg(currentText)
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_Console.displayMode)
            Cpp_IO_Console.displayMode = currentIndex
        }
      }

      Button {
        height: 24
        icon.width: 18
        icon.height: 18
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        onClicked: Cpp_IO_Console.save()
        enabled: Cpp_IO_Console.saveAvailable
        icon.source: "qrc:/icons/buttons/save.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }

      Button {
        height: 24
        icon.width: 18
        icon.height: 18
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        onClicked: Cpp_IO_Console.print()
        enabled: Cpp_IO_Console.saveAvailable
        icon.source: "qrc:/icons/buttons/print.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }

      Button {
        height: 24
        icon.width: 18
        icon.height: 18
        onClicked: root.clear()
        Layout.maximumWidth: 32
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_IO_Console.saveAvailable
        icon.source: "qrc:/icons/buttons/clear.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }
    }
  }
}
