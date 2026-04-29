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

import SerialStudio
import "Drivers" as Drivers

Rectangle {
  id: root

  radius: 2
  border.width: 1
  color: Cpp_ThemeManager.colors["groupbox_background"]
  border.color: Cpp_ThemeManager.colors["groupbox_border"]

  //
  // Create list of device panels. Reassignment (not push) is used so the
  // `buses` property notifies, keeping width/height bindings live without
  // a separate counter dependency hack.
  //
  property var buses: []
  readonly property int kPaddingV: 8
  readonly property int kFadeHeight: 12
  readonly property int kMaxDriverHeight: 360

  function registerBus(item) {
    buses = buses.concat([item])
  }

  implicitWidth: {
    let maxW = 0
    for (let i = 0; i < root.buses.length; ++i) {
      const item = root.buses[i]
      if (item && item.implicitWidth > maxW)
        maxW = item.implicitWidth
    }
    return maxW
  }

  // Pinned at the cap + spacers + border so the window's minimum height is
  // stable across driver switches AND across driver-internal layout changes
  // (e.g. Modbus TCP/RTU, Network TCP/UDP). The Flickable inside handles
  // anything taller; anything shorter just gets whitespace at the bottom.
  implicitHeight: root.kMaxDriverHeight + 2 * root.kPaddingV + 2

  //
  // Device configuration — wrapped in a Flickable so tall drivers (Modbus,
  // BLE, CAN Bus…) can scroll vertically without stretching the whole pane.
  // The Flickable fills the bordered rectangle; padding is supplied by the
  // outer ColumnLayout's spacer items so scrolled content slides under the
  // edge-fade gradients drawn on top.
  //
  Flickable {
    id: scroll

    clip: true
    anchors.margins: 1
    contentWidth: width
    anchors.fill: parent
    contentHeight: outer.implicitHeight
    interactive: contentHeight > height
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar {
      policy: scroll.contentHeight > scroll.height
              ? ScrollBar.AsNeeded
              : ScrollBar.AlwaysOff
    }

    ColumnLayout {
      id: outer

      spacing: 0
      x: kPaddingV
      width: scroll.width - 2 * kPaddingV

      Item {
        Layout.fillWidth: true
        implicitHeight: root.kPaddingV
      }

      StackLayout {
        id: layout

        Layout.fillWidth: true
        currentIndex: Cpp_IO_Manager.busType
        Layout.preferredHeight: implicitHeight
        implicitHeight: {
          let maxHeight = 0
          for (let i = 0; i < root.buses.length; ++i) {
            const item = root.buses[i]
            if (item && item.implicitHeight > maxHeight)
              maxHeight = item.implicitHeight
          }

          return maxHeight
        }

    Loader {
      active: true
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      sourceComponent: Component {
        Drivers.UART {
          Component.onCompleted: root.registerBus(this)
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
          Component.onCompleted: root.registerBus(this)
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
          Component.onCompleted: root.registerBus(this)
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
          root.registerBus(item)
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
          root.registerBus(item)
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
          root.registerBus(item)
      }
    }

    Loader {
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      active: Cpp_CommercialBuild
      source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml"

      onLoaded: {
        if (item)
          root.registerBus(item)
      }
    }

    Loader {
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      active: Cpp_CommercialBuild
      source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml"

      onLoaded: {
        if (item)
          root.registerBus(item)
      }
    }

    Loader {
      asynchronous: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      active: Cpp_CommercialBuild
      source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml"

      onLoaded: {
        if (item)
          root.registerBus(item)
      }
    }
      }

      Item {
        Layout.fillWidth: true
        implicitHeight: root.kPaddingV
      }
    }
  }

  //
  // Edge fades — masks content scrolling under the bordered edges with a
  // small gradient from the pane background to transparent. Hidden when the
  // user is at the corresponding scroll boundary so they don't dim the
  // first / last items unnecessarily.
  //
  Rectangle {
    z: 2
    visible: opacity > 0.01
    height: root.kFadeHeight
    opacity: scroll.contentY > 0.5 ? 1 : 0
    anchors {
      topMargin: 1
      leftMargin: 1
      rightMargin: 1
      top: parent.top
      left: parent.left
      right: parent.right
    }

    Behavior on opacity { NumberAnimation { duration: 120 } }

    gradient: Gradient {
      GradientStop { position: 0; color: root.color }
      GradientStop {
        position: 1
        color: Qt.rgba(root.color.r, root.color.g, root.color.b, 0)
      }
    }
  }

  Rectangle {
    z: 2
    height: root.kFadeHeight
    visible: opacity > 0.01
    opacity: (scroll.contentHeight - scroll.contentY - scroll.height) > 0.5
             ? 1 : 0
    anchors {
      leftMargin: 1
      rightMargin: 1
      bottomMargin: 1
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }

    Behavior on opacity { NumberAnimation { duration: 120 } }

    gradient: Gradient {
      GradientStop {
        position: 0
        color: Qt.rgba(root.color.r, root.color.g, root.color.b, 0)
      }
      GradientStop { position: 1; color: root.color }
    }
  }
}
