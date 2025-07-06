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
      active: Cpp_CommercialBuild
      source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml"

      onLoaded: {
        if (item)
          root.buses.push(item)
      }
    }

    Loader {
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      active: Cpp_CommercialBuild
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
      active: Cpp_CommercialBuild
      source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml"

      onLoaded: {
        if (item)
          root.buses.push(item)
      }
    }
  }
}
