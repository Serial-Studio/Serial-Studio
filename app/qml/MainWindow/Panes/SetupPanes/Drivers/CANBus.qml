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

  //
  // No Plugin Available Indicator
  //
  ColumnLayout {
    spacing: 4
    anchors.centerIn: parent
    visible: Cpp_IO_CANBus.pluginList.length === 0

    Image {
      sourceSize: Qt.size(96, 96)
      Layout.alignment: Qt.AlignHCenter
      source: "qrc:/rcc/images/driver.svg"
    }

    Item {
      implicitHeight: 4
    }

    Label {
      wrapMode: Label.WordWrap
      Layout.alignment: Qt.AlignHCenter
      Layout.maximumWidth: root.width - 64
      text: qsTr("No CAN Drivers Found")
      horizontalAlignment: Label.AlignHCenter
      font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
    }

    Label {
      opacity: 0.8
      wrapMode: Label.WordWrap
      Layout.alignment: Qt.AlignHCenter
      Layout.maximumWidth: root.width - 64
      text: qsTr("Install CAN hardware drivers for your system")
      horizontalAlignment: Label.AlignHCenter
      font: Cpp_Misc_CommonFonts.customUiFont(1.2, false)
    }
  }

  //
  // Main Layout
  //
  ColumnLayout {
    spacing: 4
    anchors.margins: 0
    anchors.fill: parent
    visible: Cpp_IO_CANBus.pluginList.length > 0

    GridLayout {
      id: layout
      columns: 2
      rowSpacing: 4
      columnSpacing: 4
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected

      //
      // CAN Driver selector
      //
      Label {
        text: qsTr("CAN Driver") + ":"
      } ComboBox {
        id: _pluginCombo
        Layout.fillWidth: true
        currentIndex: Cpp_IO_CANBus.pluginIndex

        property bool _updating: false

        model: ListModel {
          id: pluginModel
        }

        textRole: "display"

        Component.onCompleted: _pluginCombo.updatePluginModel()

        Connections {
          target: Cpp_IO_CANBus
          function onAvailablePluginsChanged() {
            _pluginCombo.updatePluginModel()
          }
        }

        function updatePluginModel() {
          _updating = true
          pluginModel.clear()
          const plugins = Cpp_IO_CANBus.pluginList
          for (let i = 0; i < plugins.length; ++i) {
            pluginModel.append({
              "display": Cpp_IO_CANBus.pluginDisplayName(plugins[i]),
              "value": plugins[i]
            })
          }
          currentIndex = Cpp_IO_CANBus.pluginIndex
          _updating = false
        }

        onCurrentIndexChanged: {
          if (!_updating && currentIndex !== Cpp_IO_CANBus.pluginIndex)
            Cpp_IO_CANBus.pluginIndex = currentIndex
        }
      }

      //
      // CAN Interface selector
      //
      Label {
        text: qsTr("Interface") + ":"
        visible: Cpp_IO_CANBus.interfaceList.length > 0
      } ComboBox {
        id: _interfaceCombo
        Layout.fillWidth: true
        model: Cpp_IO_CANBus.interfaceList
        visible: Cpp_IO_CANBus.interfaceList.length > 0
        currentIndex: Cpp_IO_CANBus.interfaceIndex
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_CANBus.interfaceIndex)
            Cpp_IO_CANBus.interfaceIndex = currentIndex
        }
      }

      //
      // Spacer
      //
      Item {
        Layout.minimumHeight: 8 / 2
        Layout.maximumHeight: 8 / 2
        visible: Cpp_IO_CANBus.interfaceList.length > 0
      } Item {
        Layout.minimumHeight: 8 / 2
        Layout.maximumHeight: 8 / 2
        visible: Cpp_IO_CANBus.interfaceList.length > 0
      }

      //
      // Bitrate selector
      //
      Label {
        text: qsTr("Bitrate") + ":"
        visible: Cpp_IO_CANBus.interfaceList.length > 0
      } ComboBox {
        id: _bitrateCombo
        editable: true
        Layout.fillWidth: true
        model: Cpp_IO_CANBus.bitrateList
        visible: Cpp_IO_CANBus.interfaceList.length > 0

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
        text: qsTr("Flexible Data-Rate") + ":"
        enabled: Cpp_IO_CANBus.interfaceList.length > 0
        opacity: enabled ? 1 : 0
      } CheckBox {
        id: _canFDCheck
        Layout.leftMargin: -8
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_IO_CANBus.canFD
        enabled: Cpp_IO_CANBus.interfaceList.length > 0
        opacity: enabled ? 1 : 0
        onCheckedChanged: {
          if (Cpp_IO_CANBus.canFD !== checked)
            Cpp_IO_CANBus.canFD = checked
        }
      }
    }

    //
    // No Interface Found Indicator
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignHCenter
      visible: Cpp_IO_CANBus.interfaceList.length === 0

      Item {
        implicitHeight: 8
      }

      Image {
        sourceSize: Qt.size(96, 96)
        Layout.alignment: Qt.AlignHCenter
        source: "qrc:/rcc/images/hammer.svg"
      }

      Item {
        implicitHeight: 4
      }

      Label {
        wrapMode: Label.WordWrap
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: root.width - 64
        text: qsTr("No CAN Interfaces Found")
        horizontalAlignment: Label.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
      }

      Label {
        opacity: 0.8
        wrapMode: Label.WordWrap
        textFormat: Label.RichText
        Layout.alignment: Qt.AlignHCenter
        text: Cpp_IO_CANBus.interfaceError
        Layout.maximumWidth: root.width - 64
        horizontalAlignment: Label.AlignHCenter
        onLinkActivated: Qt.openUrlExternally(link)
        font: Cpp_Misc_CommonFonts.customUiFont(1.2, false)
      }

      Item {
        implicitHeight: 8
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
