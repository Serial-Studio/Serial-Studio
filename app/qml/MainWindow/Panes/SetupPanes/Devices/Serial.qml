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
      editable: Qt.platform.os !== "windows"
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_Serial.portIndex)
            Cpp_IO_Serial.portIndex = currentIndex
        }
      }

      onAccepted: {
        if (_portCombo.editable && _portCombo.find(_portCombo.editText) === -1)
          Cpp_IO_Serial.registerDevice(_portCombo.editText)
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
      currentIndex: 6
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
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }
  }
}


