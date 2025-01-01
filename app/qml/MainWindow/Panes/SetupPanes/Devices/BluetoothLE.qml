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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root
  implicitHeight: layout.implicitHeight

  //
  // Control layout
  //
  ColumnLayout {
    id: layout
    spacing: 4
    anchors.margins: 0
    anchors.fill: parent

    //
    // Device selector + refresh button
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: Cpp_IO_Bluetooth_LE.operatingSystemSupported && Cpp_IO_Bluetooth_LE.deviceCount > 0

      Label {
        id: devLabel
        opacity: enabled ? 1 : 0.5
        text: qsTr("Device") + ":"
        enabled: !Cpp_IO_Manager.connected
        Layout.minimumWidth: Math.max(devLabel.implicitWidth,
                                      servLabel.implicitWidth,
                                      charLabel.implicitWidth)
      }

      ComboBox {
        id: _deviceCombo
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.connected
        model: Cpp_IO_Bluetooth_LE.deviceNames
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_Bluetooth_LE.currentDevice)
            Cpp_IO_Bluetooth_LE.selectDevice(currentIndex)
        }
      }

      Button {
        icon.width: 16
        icon.height: 16
        implicitWidth: 24
        implicitHeight: 24
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.connected
        onClicked: Cpp_IO_Bluetooth_LE.startDiscovery()
        icon.source: "qrc:/rcc/icons/buttons/refresh.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }
    }

    //
    // Service selector
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: Cpp_IO_Bluetooth_LE.operatingSystemSupported && serviceNames.count > 1

      Label {
        id: servLabel
        text: qsTr("Service") + ":"
        Layout.minimumWidth: Math.max(devLabel.implicitWidth,
                                      servLabel.implicitWidth,
                                      charLabel.implicitWidth)
      }

      ComboBox {
        id: serviceNames
        Layout.fillWidth: true
        model: Cpp_IO_Bluetooth_LE.serviceNames
        onCurrentIndexChanged: Cpp_IO_Bluetooth_LE.selectService(currentIndex)
      }
    }

    //
    // Characteristic selector
    //
    RowLayout {
      id: characteristicSelector

      spacing: 4
      Layout.fillWidth: true
      visible: Cpp_IO_Bluetooth_LE.operatingSystemSupported && characteristicNames.count > 1

      Label {
        id: charLabel
        text: qsTr("Characteristic") + ":"
        Layout.minimumWidth: Math.max(devLabel.implicitWidth,
                                      servLabel.implicitWidth,
                                      charLabel.implicitWidth)
      }

      ComboBox {
        id: characteristicNames
        Layout.fillWidth: true
        model: Cpp_IO_Bluetooth_LE.characteristicNames
        currentIndex: Cpp_IO_Bluetooth_LE.characteristicIndex
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_Bluetooth_LE.characteristicIndex)
            Cpp_IO_Bluetooth_LE.characteristicIndex = currentIndex
        }
      }
    }

    //
    // Scanning indicator
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: Cpp_IO_Bluetooth_LE.operatingSystemSupported && Cpp_IO_Bluetooth_LE.deviceCount < 1

      Image {
        id: spinner
        visible: running
        sourceSize: Qt.size(18, 18)
        Layout.alignment: Qt.AlignVCenter
        source: "qrc:/rcc/images/spinner.svg"

        property bool running: parent.visible

        onRunningChanged: spinner.rotation += 360
        Component.onCompleted: spinner.rotation += 360

        Timer {
          repeat: true
          interval: 1000
          running: spinner.running
          onTriggered: spinner.rotation += 360
        }

        Behavior on rotation {
          NumberAnimation {
            duration: 1000
          }
        }
      }

      Label {
        Layout.fillWidth: true
        text: qsTr("Scanning....")
        Layout.alignment: Qt.AlignVCenter
      }
    }

    //
    // OS not supported indicator
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: !Cpp_IO_Bluetooth_LE.operatingSystemSupported

      Image {
        sourceSize: Qt.size(96, 96)
        Layout.alignment: Qt.AlignVCenter
        source: "qrc:/rcc/images/hammer.svg"
      }

      Label {
        Layout.fillWidth: true
        wrapMode: Label.WordWrap
        Layout.alignment: Qt.AlignVCenter
        text: qsTr("Sorry, this system is not supported yet. " +
                   "We'll update Serial Studio to work with this operating " +
                   "system as soon as Qt officially supports it.")
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


