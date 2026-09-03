/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

import "../../../../Widgets" as Widgets

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
    // Device selector
    //
    Label {
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      text: qsTr("USB Device") + ":"
    } Widgets.Combo {
      id: deviceCombo

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_USB.deviceList
      currentIndex: Cpp_IO_USB.deviceIndex

      onActivated: (index) => {
        if (enabled && index !== Cpp_IO_USB.deviceIndex)
          Cpp_IO_USB.deviceIndex = index
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
      enabled: deviceCombo.currentIndex > 0 && app.ioEnabled
    } Widgets.Combo {
      id: modeCombo

      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: deviceCombo.currentIndex > 0 && app.ioEnabled
      model: [
        qsTr("Bulk/Interrupt Stream"),
        qsTr("Advanced (Bulk + Control)"),
        qsTr("Isochronous")
      ]
      currentIndex: Cpp_IO_USB.transferMode

      onActivated: (index) => {
        if (!enabled)
          return

        //
        // Advanced control transfers are consent-gated: the driver refuses them until the user
        // said yes here, so a project or an API client cannot turn them on behind their back.
        //
        if (index === 1 && !Cpp_IO_USB.advancedTransferConsent) {
          _advancedConsentDialog.open()
          return
        }

        if (Cpp_IO_USB.transferMode !== index)
          Cpp_IO_USB.transferMode = index
      }

      Connections {
        target: Cpp_IO_USB
        function onTransferModeChanged() {
          if (modeCombo.currentIndex !== Cpp_IO_USB.transferMode)
            modeCombo.currentIndex = Cpp_IO_USB.transferMode
        }
      }

      Dialog {
        id: _advancedConsentDialog

        modal: true
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.No
        title: qsTr("Enable Advanced USB Control Transfers?")

        Label {
          width: 360
          wrapMode: Text.WordWrap
          text: qsTr("This enables control transfers in addition to bulk transfers. Sending " +
                     "incorrect control requests can crash or damage connected hardware. Only " +
                     "enable this if you know what you are doing.")
        }

        onAccepted: Cpp_IO_USB.grantAdvancedTransferConsent()
        onRejected: modeCombo.currentIndex = Cpp_IO_USB.transferMode
      }
    }

    //
    // IN endpoint (populated at device selection; locked while connected)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("IN Endpoint") + ":"
      visible: deviceCombo.currentIndex > 0
      enabled: app.ioEnabled && !Cpp_IO_Manager.isConnected
    } Widgets.Combo {
      id: inEndpointCombo

      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_USB.inEndpointList
      visible: deviceCombo.currentIndex > 0
      currentIndex: Cpp_IO_USB.inEndpointIndex
      enabled: app.ioEnabled && !Cpp_IO_Manager.isConnected

      onActivated: (index) => {
        if (enabled && index !== Cpp_IO_USB.inEndpointIndex)
          Cpp_IO_USB.inEndpointIndex = index
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
    // OUT endpoint (populated at device selection; locked while connected)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("OUT Endpoint") + ":"
      visible: deviceCombo.currentIndex > 0
      enabled: app.ioEnabled && !Cpp_IO_Manager.isConnected
    } Widgets.Combo {
      id: outEndpointCombo

      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_USB.outEndpointList
      visible: deviceCombo.currentIndex > 0
      currentIndex: Cpp_IO_USB.outEndpointIndex
      enabled: app.ioEnabled && !Cpp_IO_Manager.isConnected

      onActivated: (index) => {
        if (enabled && index !== Cpp_IO_USB.outEndpointIndex)
          Cpp_IO_USB.outEndpointIndex = index
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
    // ISO packet size (only shown in Isochronous mode; locked while connected)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Max Packet Size") + ":"
      enabled: app.ioEnabled && !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_USB.isoModeEnabled && deviceCombo.currentIndex > 0
    } SpinBox {
      id: isoPacketSpin

      from: 1
      to: 49152
      stepSize: 64
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      value: Cpp_IO_USB.isoPacketSize
      enabled: app.ioEnabled && !Cpp_IO_Manager.isConnected
      visible: Cpp_IO_USB.isoModeEnabled && deviceCombo.currentIndex > 0

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
    // Control-transfer composer (Advanced Control mode, while connected)
    //
    UsbControlComposer {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: Cpp_IO_USB.advancedModeEnabled && Cpp_IO_Manager.isConnected
    }

    //
    // Info block: pre-connect, spans both columns
    //
    Item {
      implicitHeight: 4
      visible: !Cpp_IO_Manager.isConnected && !Cpp_IO_USB.advancedModeEnabled
    } Item {
      implicitHeight: 4
      visible: !Cpp_IO_Manager.isConnected && !Cpp_IO_USB.advancedModeEnabled
    } RowLayout {
      spacing: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: !Cpp_IO_Manager.isConnected && !Cpp_IO_USB.advancedModeEnabled

      Image {
        sourceSize.width: 20
        sourceSize.height: 20
        Layout.alignment: Qt.AlignTop
        source: Cpp_Misc_IconRegistry.icon("panes", "info", 24)
      }

      ColumnLayout {
        spacing: 2
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        Label {
          opacity: 0.75
          Layout.fillWidth: true
          wrapMode: Label.WordWrap
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          text: qsTr("Connect to USB devices using bulk, control, or "
                   + "isochronous transfers. Suitable for data loggers, "
                   + "custom firmware devices, and USB instruments.")
        }

        Label {
          opacity: 0.7
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          text: "<a href='https://www.usb.org/documents'>"
              + qsTr("USB specifications (USB.org)")
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
    // No usable endpoints warning
    //
    RowLayout {
      spacing: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: deviceCombo.currentIndex > 0 && Cpp_IO_USB.inEndpointList.length <= 1

      Image {
        sourceSize.width: 20
        sourceSize.height: 20
        Layout.alignment: Qt.AlignTop
        source: Cpp_Misc_IconRegistry.icon("panes", "important", 24)
      }

      Label {
        opacity: 0.85
        Layout.fillWidth: true
        wrapMode: Label.WordWrap
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
        text: qsTr("No compatible data endpoints were found for this transfer "
                 + "mode. Try another transfer mode. Devices that speak a "
                 + "dedicated protocol (e.g. CAN or Modbus adapters) should be "
                 + "connected through their own driver instead.")
      }
    }

    //
    // Spacer before warnings
    //
    Item {
      implicitHeight: 4
      visible: Cpp_IO_USB.advancedModeEnabled || (Cpp_IO_USB.isoModeEnabled && deviceCombo.currentIndex > 0)
    } Item {
      implicitHeight: 4
      visible: Cpp_IO_USB.advancedModeEnabled || (Cpp_IO_USB.isoModeEnabled && deviceCombo.currentIndex > 0)
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
        source: Cpp_Misc_IconRegistry.icon("panes", "important", 32)
      }

      ColumnLayout {
        spacing: 2
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        Label {
          Layout.fillWidth: true
          text: qsTr("Control Transfers Enabled")
          font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
        }

        Label {
          opacity: 0.85
          Layout.fillWidth: true
          wrapMode: Label.WordWrap
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
    // Isochronous info notice (shown whenever the packet-size control is visible)
    //
    RowLayout {
      spacing: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: Cpp_IO_USB.isoModeEnabled && deviceCombo.currentIndex > 0

      Image {
        sourceSize.width: 20
        sourceSize.height: 20
        Layout.alignment: Qt.AlignVCenter
        source: Cpp_Misc_IconRegistry.icon("panes", "info", 24)
      }

      Label {
        opacity: 0.75
        Layout.fillWidth: true
        wrapMode: Label.WordWrap
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
