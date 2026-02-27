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
    // Device selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("USB Device") + ":"
      enabled: !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: deviceCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_USB.deviceList
      enabled: !Cpp_IO_Manager.isConnected
      currentIndex: Cpp_IO_USB.deviceIndex

      onCurrentIndexChanged: {
        if (enabled && currentIndex !== Cpp_IO_USB.deviceIndex)
          Cpp_IO_USB.deviceIndex = currentIndex
      }

      Connections {
        target: Cpp_IO_USB
        function onDeviceIndexChanged() {
          if (deviceCombo.currentIndex !== Cpp_IO_USB.deviceIndex)
            deviceCombo.currentIndex = Cpp_IO_USB.deviceIndex
        }
      }
    }

    //
    // Transfer mode (pre-connect, user chooses before clicking Connect)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Transfer Mode") + ":"
      enabled: deviceCombo.currentIndex > 0 && !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: modeCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: deviceCombo.currentIndex > 0 && !Cpp_IO_Manager.isConnected
      model: [
        qsTr("Bulk Stream"),
        qsTr("Advanced (Bulk + Control)"),
        qsTr("Isochronous")
      ]
      currentIndex: Cpp_IO_USB.transferMode

      onCurrentIndexChanged: {
        if (enabled && currentIndex !== Cpp_IO_USB.transferMode)
          Cpp_IO_USB.transferMode = currentIndex
      }

      Connections {
        target: Cpp_IO_USB
        function onTransferModeChanged() {
          if (modeCombo.currentIndex !== Cpp_IO_USB.transferMode)
            modeCombo.currentIndex = Cpp_IO_USB.transferMode
        }
      }
    }

    //
    // IN endpoint (only shown while connected — populated after open())
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("IN Endpoint") + ":"
      visible: Cpp_IO_Manager.isConnected
      enabled: Cpp_IO_Manager.isConnected
    } ComboBox {
      id: inEndpointCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      visible: Cpp_IO_Manager.isConnected
      enabled: Cpp_IO_Manager.isConnected
      model: Cpp_IO_USB.inEndpointList
      currentIndex: Cpp_IO_USB.inEndpointIndex

      onCurrentIndexChanged: {
        if (enabled && currentIndex !== Cpp_IO_USB.inEndpointIndex)
          Cpp_IO_USB.inEndpointIndex = currentIndex
      }

      Connections {
        target: Cpp_IO_USB
        function onInEndpointIndexChanged() {
          if (inEndpointCombo.currentIndex !== Cpp_IO_USB.inEndpointIndex)
            inEndpointCombo.currentIndex = Cpp_IO_USB.inEndpointIndex
        }
        function onEndpointListChanged() {
          inEndpointCombo.model = Cpp_IO_USB.inEndpointList
          inEndpointCombo.currentIndex = Cpp_IO_USB.inEndpointIndex
        }
      }
    }

    //
    // OUT endpoint (only shown while connected)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("OUT Endpoint") + ":"
      visible: Cpp_IO_Manager.isConnected
      enabled: Cpp_IO_Manager.isConnected
    } ComboBox {
      id: outEndpointCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      visible: Cpp_IO_Manager.isConnected
      enabled: Cpp_IO_Manager.isConnected
      model: Cpp_IO_USB.outEndpointList
      currentIndex: Cpp_IO_USB.outEndpointIndex

      onCurrentIndexChanged: {
        if (enabled && currentIndex !== Cpp_IO_USB.outEndpointIndex)
          Cpp_IO_USB.outEndpointIndex = currentIndex
      }

      Connections {
        target: Cpp_IO_USB
        function onOutEndpointIndexChanged() {
          if (outEndpointCombo.currentIndex !== Cpp_IO_USB.outEndpointIndex)
            outEndpointCombo.currentIndex = Cpp_IO_USB.outEndpointIndex
        }
        function onEndpointListChanged() {
          outEndpointCombo.model = Cpp_IO_USB.outEndpointList
          outEndpointCombo.currentIndex = Cpp_IO_USB.outEndpointIndex
        }
      }
    }

    //
    // ISO packet size (only shown while connected in Isochronous mode)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Max Packet Size") + ":"
      visible: Cpp_IO_Manager.isConnected && Cpp_IO_USB.isoModeEnabled
      enabled: Cpp_IO_Manager.isConnected
    } SpinBox {
      id: isoPacketSpin
      from: 1
      to: 49152
      stepSize: 64
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      visible: Cpp_IO_Manager.isConnected && Cpp_IO_USB.isoModeEnabled
      enabled: Cpp_IO_Manager.isConnected
      value: Cpp_IO_USB.isoPacketSize

      onValueModified: Cpp_IO_USB.isoPacketSize = value

      Connections {
        target: Cpp_IO_USB
        function onIsoPacketSizeChanged() {
          if (isoPacketSpin.value !== Cpp_IO_USB.isoPacketSize)
            isoPacketSpin.value = Cpp_IO_USB.isoPacketSize
        }
      }
    }

    //
    // Spacer before warnings
    //
    Item {
      implicitHeight: 4
      visible: Cpp_IO_USB.advancedModeEnabled || (Cpp_IO_Manager.isConnected && Cpp_IO_USB.isoModeEnabled)
    } Item {
      implicitHeight: 4
      visible: Cpp_IO_USB.advancedModeEnabled || (Cpp_IO_Manager.isConnected && Cpp_IO_USB.isoModeEnabled)
    }

    //
    // Advanced Control warning
    //
    RowLayout {
      spacing: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: Cpp_IO_USB.advancedModeEnabled

      Image {
        sourceSize.width: 32
        sourceSize.height: 32
        Layout.alignment: Qt.AlignTop
        source: "qrc:/rcc/icons/panes/important.svg"
      }

      ColumnLayout {
        spacing: 2
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        Label {
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
          text: qsTr("Control Transfers Enabled")
        }

        Label {
          opacity: 0.85
          wrapMode: Label.WordWrap
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.customUiFont(1.0, false)
          text: qsTr("Sending incorrect control requests may crash or damage connected hardware. Use with caution.")
        }

        Label {
          opacity: 0.7
          font: Cpp_Misc_CommonFonts.customUiFont(1.0, false)
          text: "<a href='https://libusb.sourceforge.io/api-1.0/libusb_io.html'>"
              + qsTr("Learn about USB control transfers")
              + "</a>"
          textFormat: Text.RichText
          onLinkActivated: (link) => Qt.openUrlExternally(link)

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }
      }
    }

    //
    // Isochronous info notice (only shown post-connect when controls are visible)
    //
    RowLayout {
      spacing: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: Cpp_IO_Manager.isConnected && Cpp_IO_USB.isoModeEnabled

      Image {
        sourceSize.width: 20
        sourceSize.height: 20
        Layout.alignment: Qt.AlignVCenter
        source: "qrc:/rcc/icons/panes/info.svg"
      }

      Label {
        opacity: 0.75
        wrapMode: Label.WordWrap
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
        text: qsTr("Packet size should match the maximum transfer size reported "
                 + "by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).")
      }
    }

    //
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }
  }
}
