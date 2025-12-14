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
    // CAN Plugin selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("CAN Plugin") + ":"
      enabled: !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: _pluginCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_CANBus.pluginList
      enabled: !Cpp_IO_Manager.isConnected
      currentIndex: Cpp_IO_CANBus.pluginIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_CANBus.pluginIndex)
            Cpp_IO_CANBus.pluginIndex = currentIndex
        }
      }
    }

    //
    // CAN Interface selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Interface") + ":"
      enabled: !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: _interfaceCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_CANBus.interfaceList
      enabled: !Cpp_IO_Manager.isConnected
      currentIndex: Cpp_IO_CANBus.interfaceIndex
      onCurrentIndexChanged: {
        if (enabled) {
          if (currentIndex !== Cpp_IO_CANBus.interfaceIndex)
            Cpp_IO_CANBus.interfaceIndex = currentIndex
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
    // Bitrate selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Bitrate") + ":"
    } ComboBox {
      id: _bitrateCombo
      editable: true
      Layout.fillWidth: true
      model: Cpp_IO_CANBus.bitrateList

      validator: IntValidator { bottom: 1 }

      Component.onCompleted: {
        Qt.callLater(() => {
          const current = String(Cpp_IO_CANBus.bitrate)
          const rates = Cpp_IO_CANBus.bitrateList

          const idx = rates.indexOf(current)
          if (idx !== -1) {
            _bitrateCombo.currentIndex = idx
          } else {
            _bitrateCombo.currentIndex = -1
            _bitrateCombo.editText = current
          }
        })
      }

      onEditTextChanged: {
        const value = parseInt(editText)
        if (!isNaN(value) && value > 0) {
          if (Cpp_IO_CANBus.bitrate !== value)
            Cpp_IO_CANBus.bitrate = value
        }
      }

      onCurrentIndexChanged: {
        if (currentIndex >= 0 && currentIndex < model.length) {
          const value = parseInt(model[currentIndex])
          if (!isNaN(value) && Cpp_IO_CANBus.bitrate !== value) {
            Cpp_IO_CANBus.bitrate = value
            editText = String(value)
          }
        }
      }
    }

    //
    // CAN FD checkbox
    //
    Label {
      text: qsTr("CAN FD") + ":"
    } CheckBox {
      id: _canFDCheck
      Layout.leftMargin: -8
      Layout.alignment: Qt.AlignLeft
      checked: Cpp_IO_CANBus.canFD
      onCheckedChanged: {
        if (Cpp_IO_CANBus.canFD !== checked)
          Cpp_IO_CANBus.canFD = checked
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
