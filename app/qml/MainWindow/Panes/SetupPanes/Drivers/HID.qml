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
      text: qsTr("HID Device") + ":"
      enabled: !Cpp_IO_Manager.isConnected
      Layout.maximumWidth: root.width / 2
    } ComboBox {
      id: deviceCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_HID.deviceList
      enabled: !Cpp_IO_Manager.isConnected
      currentIndex: Cpp_IO_HID.deviceIndex

      onCurrentIndexChanged: {
        if (enabled && currentIndex !== Cpp_IO_HID.deviceIndex)
          Cpp_IO_HID.deviceIndex = currentIndex
      }

      Connections {
        target: Cpp_IO_HID
        function onDeviceIndexChanged() {
          if (deviceCombo.currentIndex !== Cpp_IO_HID.deviceIndex)
            deviceCombo.currentIndex = Cpp_IO_HID.deviceIndex
        }
      }
    }

    //
    // Usage page (post-connect info)
    //
    Label {
      opacity: 0.5
      text: qsTr("Usage Page") + ":"
      visible: Cpp_IO_Manager.isConnected
      Layout.maximumWidth: root.width / 2
    } TextField {
      readOnly: true
      Layout.fillWidth: true
      opacity: 0.8
      text: Cpp_IO_HID.usagePage
      visible: Cpp_IO_Manager.isConnected
      font.family: Cpp_Misc_CommonFonts.monoFont.family
    }

    //
    // Usage (post-connect info)
    //
    Label {
      opacity: 0.5
      text: qsTr("Usage") + ":"
      visible: Cpp_IO_Manager.isConnected
      Layout.maximumWidth: root.width / 2
    } TextField {
      readOnly: true
      Layout.fillWidth: true
      opacity: 0.8
      text: Cpp_IO_HID.usage
      visible: Cpp_IO_Manager.isConnected
      font.family: Cpp_Misc_CommonFonts.monoFont.family
    }

    //
    // Spacer before info block
    //
    Item { implicitHeight: 4 } Item { implicitHeight: 4 }

    //
    // Info block — always visible, spans both columns
    //
    RowLayout {
      spacing: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true

      Image {
        sourceSize.width: 20
        sourceSize.height: 20
        Layout.alignment: Qt.AlignTop
        source: "qrc:/rcc/icons/panes/info.svg"
      }

      ColumnLayout {
        spacing: 2
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        Label {
          opacity: 0.75
          wrapMode: Label.WordWrap
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          text: qsTr("Connect gamepads, joysticks, steering wheels, "
                   + "flight controllers, and other HID-class USB devices.")
        }

        Label {
          opacity: 0.7
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          text: "<a href='https://usb.org/sites/default/files/hut1_5.pdf'>"
              + qsTr("HID Usage Tables (USB.org)")
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
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }
  }
}
