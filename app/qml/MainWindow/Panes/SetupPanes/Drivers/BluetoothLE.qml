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
        enabled: !Cpp_IO_Manager.isConnected
        Layout.minimumWidth: Math.max(devLabel.implicitWidth,
                                      servLabel.implicitWidth,
                                      charLabel.implicitWidth)
      }

      ComboBox {
        id: _deviceCombo
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
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
        enabled: !Cpp_IO_Manager.isConnected
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


