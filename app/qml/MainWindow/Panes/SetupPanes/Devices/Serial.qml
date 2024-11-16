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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root
  implicitHeight: layout.implicitHeight

  //
  // Access to properties
  //
  property alias dtr: _dtr.checked
  property alias port: _portCombo.currentIndex
  property alias baudRate: _baudCombo.currentIndex
  property alias dataBits: _dataCombo.currentIndex
  property alias parity: _parityCombo.currentIndex
  property alias flowControl: _flowCombo.currentIndex
  property alias stopBits: _stopBitsCombo.currentIndex
  property alias autoReconnect: _autoreconnect.checked
  property alias ignoreDataDelimeters: _ignoreDataDelimeters.checked

  //
  // Update listbox models when translation is changed
  //
  Connections {
    target: Cpp_Misc_Translator
    function onLanguageChanged() {
      var oldParityIndex = _parityCombo.currentIndex
      var oldFlowControlIndex = _flowCombo.currentIndex

      _parityCombo.model = Cpp_IO_Serial.parityList
      _flowCombo.model = Cpp_IO_Serial.flowControlList

      _parityCombo.currentIndex = oldParityIndex
      _flowCombo.currentIndex = oldFlowControlIndex
    }
  }

  //
  // Controls
  //
  GridLayout {
    id: layout
    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    anchors.margins: 0
    anchors.fill: parent

    //
    // COM port selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("COM Port") + ":"
      enabled: !Cpp_IO_Manager.connected
    } ComboBox {
      id: _portCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Serial.portList
      enabled: !Cpp_IO_Manager.connected
      currentIndex: Cpp_IO_Serial.portIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_Serial.portIndex)
            Cpp_IO_Serial.portIndex = currentIndex
        }
      }
    }

    //
    // Baud rate selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Baud Rate") + ":"
    } ComboBox {
      id: _baudCombo
      editable: true
      currentIndex: 4
      Layout.fillWidth: true
      model: Cpp_IO_Serial.baudRateList

      validator: IntValidator {
        bottom: 1
      }

      onAccepted: {
        if (find(editText) === -1)
          Cpp_IO_Serial.appendBaudRate(editText)
      }

      onCurrentTextChanged: {
        var value = currentText
        Cpp_IO_Serial.baudRate = value
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    } Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    }

    //
    // Data bits selector
    //
    Label {
      text: qsTr("Data Bits") + ":"
    } ComboBox {
      id: _dataCombo
      Layout.fillWidth: true
      model: Cpp_IO_Serial.dataBitsList
      currentIndex: Cpp_IO_Serial.dataBitsIndex
      onCurrentIndexChanged: {
        if (Cpp_IO_Serial.dataBitsIndex !== currentIndex)
          Cpp_IO_Serial.dataBitsIndex = currentIndex
      }
    }

    //
    // Parity selector
    //
    Label {
      text: qsTr("Parity") + ":"
    } ComboBox {
      id: _parityCombo
      Layout.fillWidth: true
      model: Cpp_IO_Serial.parityList
      currentIndex: Cpp_IO_Serial.parityIndex
      onCurrentIndexChanged: {
        if (Cpp_IO_Serial.parityIndex !== currentIndex)
          Cpp_IO_Serial.parityIndex = currentIndex
      }
    }

    //
    // Stop bits selector
    //
    Label {
      text: qsTr("Stop Bits") + ":"
    } ComboBox {
      id: _stopBitsCombo
      Layout.fillWidth: true
      model: Cpp_IO_Serial.stopBitsList
      currentIndex: Cpp_IO_Serial.stopBitsIndex
      onCurrentIndexChanged: {
        if (Cpp_IO_Serial.stopBitsIndex !== currentIndex)
          Cpp_IO_Serial.stopBitsIndex = currentIndex
      }
    }

    //
    // Flow control selector
    //
    Label {
      text: qsTr("Flow Control") + ":"
    } ComboBox {
      id: _flowCombo
      Layout.fillWidth: true
      model: Cpp_IO_Serial.flowControlList
      currentIndex: Cpp_IO_Serial.flowControlIndex
      onCurrentIndexChanged: {
        if (Cpp_IO_Serial.flowControlIndex !== currentIndex)
          Cpp_IO_Serial.flowControlIndex = currentIndex
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    } Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    }

    //
    // Auto-reconnect
    //
    Label {
      text: qsTr("Auto Reconnect") + ":"
    } CheckBox {
      id: _autoreconnect
      Layout.maximumHeight: 18
      Layout.leftMargin: -8
      checked: Cpp_IO_Serial.autoReconnect
      Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
      onCheckedChanged: {
        if (Cpp_IO_Serial.autoReconnect !== checked)
          Cpp_IO_Serial.autoReconnect = checked
      }
    }

    //
    // DTR Signal
    //
    Label {
      text: qsTr("Send DTR Signal") + ":"
    } CheckBox {
      id: _dtr
      Layout.maximumHeight: 18
      Layout.leftMargin: -8
      checked: Cpp_IO_Serial.dtrEnabled
      Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
      onCheckedChanged: {
        if (Cpp_IO_Serial.dtrEnabled !== checked)
          Cpp_IO_Serial.dtrEnabled = checked
      }
    }

    //
    // Ignore data delimeters
    //
    Label {
      text: qsTr("No Delimiters") + ":"
      enabled: opacity == 1
    } CheckBox {
      id: _ignoreDataDelimeters
      Layout.alignment: Qt.AlignLeft
      Layout.leftMargin: -8
      checked: Cpp_IO_Serial.ignoreDataDelimeters
      onCheckedChanged: {
        if (Cpp_IO_Serial.ignoreDataDelimeters !== checked)
          Cpp_IO_Serial.ignoreDataDelimeters = checked
      }
    }

    //
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }
  }
}


