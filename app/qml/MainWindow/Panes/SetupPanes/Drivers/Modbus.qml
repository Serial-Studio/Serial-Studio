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
  implicitWidth: layout.implicitWidth + 16

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
      text: qsTr("Protocol") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } ComboBox {
      id: _protocolCombo

      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.protocolList
      enabled: app.ioEnabled
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
      text: qsTr("Serial Port") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      id: _serialPortCombo

      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.serialPortList
      enabled: app.ioEnabled
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
      text: qsTr("Baud Rate") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      id: _baudRateCombo

      editable: true
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.baudRateList
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 0

      property bool _initializing: true
      property bool _updatingIndex: false

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
            _baudRateCombo._updatingIndex = true

            const rates = Cpp_IO_Modbus.baudRateList
            const current = String(Cpp_IO_Modbus.baudRate)
            _baudRateCombo.model = rates

            const idx = rates.indexOf(current)
            if (idx !== -1)
              _baudRateCombo.currentIndex = idx
            else
              _baudRateCombo.editText = current

            _baudRateCombo._updatingIndex = false
          }
        }
      }

      Connections {
        target: Cpp_IO_Modbus
        function onProtocolIndexChanged() {
          if (Cpp_IO_Modbus.protocolIndex === 0 && !_baudRateCombo._initializing) {
            _baudRateCombo._updatingIndex = true

            const rates = Cpp_IO_Modbus.baudRateList
            const current = String(Cpp_IO_Modbus.baudRate)
            _baudRateCombo.model = rates

            const idx = rates.indexOf(current)
            if (idx !== -1)
              _baudRateCombo.currentIndex = idx
            else
              _baudRateCombo.editText = current

            _baudRateCombo._updatingIndex = false
          }
        }
      }

      onAccepted: {
        if (!_initializing) {
          const value = parseInt(editText)
          if (!isNaN(value) && value > 0) {
            if (Cpp_IO_Modbus.baudRate !== value)
              Cpp_IO_Modbus.baudRate = value
          }
        }
      }

      onCurrentIndexChanged: {
        if (!_initializing && !_updatingIndex
            && currentIndex >= 0 && currentIndex < model.length) {
          const value = parseInt(model[currentIndex])
          if (!isNaN(value) && Cpp_IO_Modbus.baudRate !== value) {
            _updatingIndex = true
            Cpp_IO_Modbus.baudRate = value
            _updatingIndex = false
          }
        }
      }
    }

    //
    // Parity (only for Modbus RTU)
    //
    Label {
      text: qsTr("Parity") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.parityList
      enabled: app.ioEnabled
      currentIndex: Cpp_IO_Modbus.parityIndex
      visible: Cpp_IO_Modbus.protocolIndex === 0
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
      text: qsTr("Data Bits") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.dataBitsList
      enabled: app.ioEnabled
      currentIndex: Cpp_IO_Modbus.dataBitsIndex
      visible: Cpp_IO_Modbus.protocolIndex === 0
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
      text: qsTr("Stop Bits") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 0
    } ComboBox {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Modbus.stopBitsList
      enabled: app.ioEnabled
      currentIndex: Cpp_IO_Modbus.stopBitsIndex
      visible: Cpp_IO_Modbus.protocolIndex === 0
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
      text: qsTr("Host") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 1
    } TextField {
      id: _hostField

      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("IP Address")
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 1
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
      text: qsTr("Port") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 1
    } TextField {
      id: _portField

      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("TCP Port")
      enabled: app.ioEnabled
      visible: Cpp_IO_Modbus.protocolIndex === 1
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
      implicitHeight: _groupButton.height + _importButton.height
                      + _groupStatus.height + 12

      Button {
        id: _groupButton

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onClicked: _groupsDialog.show()
        text: qsTr("Configure Register Groups...")
      }

      Button {
        id: _importButton

        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _groupButton.bottom
        text: qsTr("Import Register Map...")
        onClicked: Cpp_JSON_ModbusMapImporter.importRegisterMap()
      }

      Label {
        id: _groupStatus

        opacity: 0.5
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _importButton.bottom
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        horizontalAlignment: Text.AlignHCenter
        text: Cpp_IO_Modbus.registerGroupCount > 0 ?
                qsTr("%1 group(s) configured").arg(Cpp_IO_Modbus.registerGroupCount) :
                qsTr("No groups configured")
      }
    }

    ModbusGroupsDialog {
      id: _groupsDialog
    }

    ModbusPreviewDialog {
      id: _modbusPreviewDialog
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
