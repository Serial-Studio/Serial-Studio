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
      visible: opacity > 0
      opacity: Cpp_IO_Bluetooth_LE.operatingSystemSupported && Cpp_IO_Bluetooth_LE.deviceCount > 0 ? 1 : 0

      Label {
        id: devLabel
        opacity: enabled ? 1 : 0.5
        text: qsTr("Device") + ":"
        enabled: !Cpp_IO_Manager.connected
        Layout.minimumWidth: Math.max(devLabel.implicitWidth, servLabel.implicitWidth)
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
      visible: opacity > 0
      opacity: Cpp_IO_Bluetooth_LE.operatingSystemSupported && serviceNames.count > 1 ? 1 : 0

      Label {
        id: servLabel
        text: qsTr("Service") + ":"
        Layout.minimumWidth: Math.max(devLabel.implicitWidth, servLabel.implicitWidth)
      }

      ComboBox {
        id: serviceNames
        Layout.fillWidth: true
        model: Cpp_IO_Bluetooth_LE.serviceNames
        onCurrentIndexChanged: Cpp_IO_Bluetooth_LE.selectService(currentIndex)
      }
    }

    //
    // Scanning indicator
    //
    RowLayout {
      spacing: 4
      visible: opacity > 0
      opacity: Cpp_IO_Bluetooth_LE.operatingSystemSupported && Cpp_IO_Bluetooth_LE.deviceCount < 1 ? 1 : 0

      BusyIndicator {
        running: parent.visible
        Layout.minimumWidth: 16
        Layout.minimumHeight: 16
        Layout.alignment: Qt.AlignVCenter
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
      visible: opacity > 0
      opacity: !Cpp_IO_Bluetooth_LE.operatingSystemSupported

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


