/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root
  implicitHeight: layout.implicitHeight

  GridLayout {
    id: layout
    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    anchors.margins: 0
    anchors.fill: parent

    //
    // Protocol selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Protocol") + ":"
      enabled: !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: _protocolCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.protocolList
      enabled: !Cpp_IO_Manager.isConnected
      currentIndex: Cpp_IO_Modbus.protocolIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_Modbus.protocolIndex)
            Cpp_IO_Modbus.protocolIndex = currentIndex
        }
      }
    }

    //
    // Serial Port (only for Modbus RTU)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Serial Port") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      id: _serialPortCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.serialPortList
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
      currentIndex: Cpp_IO_Modbus.serialPortIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_Modbus.serialPortIndex)
            Cpp_IO_Modbus.serialPortIndex = currentIndex
        }
      }
    }

    //
    // Baud Rate (only for Modbus RTU)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Baud Rate") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      id: _baudRateCombo
      editable: true
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.baudRateList
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0

      property bool _initializing: true

      validator: IntValidator { bottom: 1 }

      Component.onCompleted: {
        Qt.callLater(() => {
          const current = String(Cpp_IO_Modbus.baudRate)
          const rates = Cpp_IO_Modbus.baudRateList
          const idx = rates.indexOf(current)
          if (idx !== -1) {
            _baudRateCombo.currentIndex = idx
          } else {
            _baudRateCombo.currentIndex = -1
            _baudRateCombo.editText = current
          }

          _initializing = false
        })
      }

      Connections {
        target: Cpp_IO_Modbus
        function onBaudRateChanged() {
          if (!_baudRateCombo._initializing && Cpp_IO_Modbus.protocolIndex === 0) {
            const current = String(Cpp_IO_Modbus.baudRate)
            const rates = Cpp_IO_Modbus.baudRateList
            const idx = rates.indexOf(current)
            if (idx !== -1 && idx !== _baudRateCombo.currentIndex) {
              _baudRateCombo.currentIndex = idx
            } else if (idx === -1) {
              _baudRateCombo.editText = current
            }
          }
        }
      }

      Connections {
        target: Cpp_IO_Modbus
        function onProtocolIndexChanged() {
          if (Cpp_IO_Modbus.protocolIndex === 0 && !_baudRateCombo._initializing) {
            const current = String(Cpp_IO_Modbus.baudRate)
            const rates = Cpp_IO_Modbus.baudRateList
            const idx = rates.indexOf(current)
            if (idx !== -1) {
              _baudRateCombo.currentIndex = idx
            } else {
              _baudRateCombo.currentIndex = -1
              _baudRateCombo.editText = current
            }
          }
        }
      }

      onEditTextChanged: {
        if (!_initializing) {
          const value = parseInt(editText)
          if (!isNaN(value) && value > 0) {
            if (Cpp_IO_Modbus.baudRate !== value)
              Cpp_IO_Modbus.baudRate = value
          }
        }
      }

      onCurrentIndexChanged: {
        if (!_initializing && currentIndex >= 0 && currentIndex < model.length) {
          const value = parseInt(model[currentIndex])
          if (!isNaN(value) && Cpp_IO_Modbus.baudRate !== value) {
            Cpp_IO_Modbus.baudRate = value
            editText = String(value)
          }
        }
      }
    }

    //
    // Parity (only for Modbus RTU)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Parity") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.parityList
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
      currentIndex: Cpp_IO_Modbus.parityIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_Modbus.parityIndex)
            Cpp_IO_Modbus.parityIndex = currentIndex
        }
      }
    }

    //
    // Data Bits (only for Modbus RTU)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Data Bits") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.dataBitsList
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
      currentIndex: Cpp_IO_Modbus.dataBitsIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_Modbus.dataBitsIndex)
            Cpp_IO_Modbus.dataBitsIndex = currentIndex
        }
      }
    }

    //
    // Stop Bits (only for Modbus RTU)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Stop Bits") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.stopBitsList
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 0
      currentIndex: Cpp_IO_Modbus.stopBitsIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_Modbus.stopBitsIndex)
            Cpp_IO_Modbus.stopBitsIndex = currentIndex
        }
      }
    }

    //
    // TCP Host (only for Modbus TCP)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Host") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 1
    } TextField {
      id: _hostField
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 1
      placeholderText: qsTr("IP Address")
      Component.onCompleted: text = Cpp_IO_Modbus.host

      onEditingFinished: {
        if (Cpp_IO_Modbus.host !== text)
          Cpp_IO_Modbus.host = text
      }
    }

    //
    // TCP Port (only for Modbus TCP)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Port") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 1
    } TextField {
      id: _portField
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_Modbus.protocolIndex === 1
      placeholderText: qsTr("TCP Port")
      validator: IntValidator { bottom: 1; top: 65535 }
      Component.onCompleted: text = Cpp_IO_Modbus.port

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Modbus.port !== value)
          Cpp_IO_Modbus.port = value
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
    // Slave Address
    //
    Label {
      text: qsTr("Slave Address") + ":"
    } TextField {
      id: _slaveField
      Layout.fillWidth: true
      placeholderText: qsTr("1-247")
      validator: IntValidator { bottom: 1; top: 247 }
      Component.onCompleted: text = Cpp_IO_Modbus.slaveAddress

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Modbus.slaveAddress !== value)
          Cpp_IO_Modbus.slaveAddress = value
      }
    }

    //
    // Poll Interval
    //
    Label {
      text: qsTr("Poll Interval (ms)") + ":"
    } TextField {
      id: _intervalField
      Layout.fillWidth: true
      placeholderText: qsTr("Polling interval")
      validator: IntValidator { bottom: 50; top: 60000 }
      Component.onCompleted: text = Cpp_IO_Modbus.pollInterval

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Modbus.pollInterval !== value)
          Cpp_IO_Modbus.pollInterval = value
      }
    }

    //
    // Register groups configuration button
    //
    Item {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.topMargin: 16
      implicitHeight: _groupButton.height + _groupStatus.height + 4

      Button {
        id: _groupButton
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: qsTr("Configure Register Groups...")
        onClicked: _groupsDialog.show()
      }

      Label {
        id: _groupStatus
        anchors.top: _groupButton.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        text: Cpp_IO_Modbus.registerGroupCount > 0 ?
                qsTr("%1 group(s) configured").arg(Cpp_IO_Modbus.registerGroupCount) :
                qsTr("No groups configured")
        opacity: 0.5
        font.italic: true
        font.pixelSize: 11
        horizontalAlignment: Text.AlignHCenter
      }
    }

    ModbusGroupsDialog {
      id: _groupsDialog
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
