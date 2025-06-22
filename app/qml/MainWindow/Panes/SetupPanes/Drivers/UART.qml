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

Item {
  id: root
  implicitHeight: layout.implicitHeight

  //
  // Save settings
  //
  Settings {
    category: "UartDriver"
    property alias dtr: _dtr.checked
    property alias port: _portCombo.currentIndex
    property alias dataBits: _dataCombo.currentIndex
    property alias parity: _parityCombo.currentIndex
    property alias flowControl: _flowCombo.currentIndex
    property alias stopBits: _stopBitsCombo.currentIndex
    property alias autoReconnect: _autoreconnect.checked
  }

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
      enabled: !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: _portCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Serial.portList
      enabled: !Cpp_IO_Manager.isConnected
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
      Layout.fillWidth: true

      validator: IntValidator { bottom: 1 }

      Component.onCompleted: {
        Qt.callLater(() => {
          const rates = Cpp_IO_Serial.baudRateList
          const current = String(Cpp_IO_Serial.baudRate)

          _baudCombo.model = rates

          const idx = rates.indexOf(current)
          if (idx !== -1) {
            _baudCombo.currentIndex = idx
          } else {
            _baudCombo.currentIndex = -1
            _baudCombo.editText = current
          }
        })
      }

      onEditTextChanged: {
        const value = parseInt(editText)
        if (!isNaN(value) && value > 0) {
          if (Cpp_IO_Serial.baudRate !== value)
            Cpp_IO_Serial.baudRate = value
        }
      }

      onCurrentIndexChanged: {
        if (currentIndex >= 0 && currentIndex < model.length) {
          const value = parseInt(model[currentIndex])
          if (!isNaN(value) && Cpp_IO_Serial.baudRate !== value) {
            Cpp_IO_Serial.baudRate = value
            editText = String(value)
          }
        }
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


