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

import SerialStudio
import "Drivers" as Drivers

Rectangle {
  id: root
  radius: 2
  border.width: 1
  implicitHeight: layout.implicitHeight + 32
  color: Cpp_ThemeManager.colors["groupbox_background"]
  border.color: Cpp_ThemeManager.colors["groupbox_border"]

  //
  // Create list of device panels
  //
  property var buses: []

  //
  // Device configuration
  //
  StackLayout {
    id: layout
    clip: true
    anchors.margins: 8
    anchors.fill: parent
    currentIndex: Cpp_IO_Manager.busType
    implicitHeight: {
      let maxHeight = 0;
      for (let i = 0; i < root.buses.length; ++i) {
        const item = root.buses[i];
        if (item && item.implicitHeight > maxHeight) {
          maxHeight = item.implicitHeight;
        }
      }

      return maxHeight + 32;
    }

    Loader {
      active: true
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      sourceComponent: Component {
        Drivers.UART {
          Component.onCompleted: root.buses.push(this)
        }
      }
    }

    Loader {
      active: true
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      sourceComponent: Component {
        Drivers.Network {
          Component.onCompleted: root.buses.push(this)
        }
      }
    }

    Loader {
      active: true
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      sourceComponent: Component {
        Drivers.BluetoothLE {
          Component.onCompleted: root.buses.push(this)
        }
      }
    }

    Loader {
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      active: Cpp_QtCommercial_Available
      source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml"

      onLoaded: {
        if (item)
          root.buses.push(item)
      }
    }

    Loader {
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      active: Cpp_QtCommercial_Available
      source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml"

      onLoaded: {
        if (item)
          root.buses.push(item)
      }
    }
  }
}
