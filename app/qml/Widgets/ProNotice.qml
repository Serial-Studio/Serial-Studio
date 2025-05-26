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
import QtQuick.Controls

Rectangle {
  id: root

  //
  // Custom properties
  //
  property bool hidden: false
  property string titleText: ""
  property string subtitleText: ""
  property bool activationFlag: false
  property bool expandSubtitle: true
  property bool closeButtonEnabled: true

  //
  // Geometry & color
  //
  border.width: 1
  implicitWidth: showSomeSupport.implicitWidth + 16
  implicitHeight: showSomeSupport.implicitHeight + 16
  color: Cpp_ThemeManager.colors["groupbox_background"]
  border.color: Cpp_ThemeManager.colors["groupbox_border"]

  //
  // Notification visibility control
  //
  onActivationFlagChanged: hidden = false
  visible: root.activationFlag && !app.proVersion && !hidden

  //
  // Main layout
  //
  RowLayout {
    id: showSomeSupport

    spacing: 12
    anchors.margins: 8
    anchors.fill: parent

    //
    // Pro icon
    //
    Image {
      Layout.minimumWidth: 96
      Layout.maximumWidth: 96
      Layout.minimumHeight: 96
      Layout.maximumHeight: 96
      sourceSize: Qt.size(96, 96)
      Layout.alignment: Qt.AlignVCenter
      source: Cpp_Misc_Utilities.hdpiImagePath("qrc:/rcc/logo/icon-pro.png")
    }

    //
    // Title, subtitle and buy/activate buttons
    //
    ColumnLayout {
      spacing: 0
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter

      Label {
        id: title
        text: root.titleText
        Layout.fillWidth: true
        elide: Text.ElideRight
        font: Cpp_Misc_CommonFonts.customUiFont(1.33, true)
      }

      Item {
        implicitHeight: 4
      }

      Label {
        Layout.fillWidth: true
        text: root.subtitleText
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        Layout.maximumWidth: Math.max(expandSubtitle ? root.width - 16 - 96 - 12 : 256,
                                      title.implicitWidth)
      }

      Item {
        implicitHeight: 12
      }

      RowLayout {
        Layout.fillWidth: true

        Button {
          icon.width: 18
          icon.height: 18
          horizontalPadding: 8
          text: qsTr("Visit Website")
          visible: !Cpp_QtCommercial_Available
          icon.source: "qrc:/rcc/icons/buttons/website.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          onClicked: Qt.openUrlExternally("https://serial-studio.com/")
        }

        Button {
          icon.width: 18
          icon.height: 18
          horizontalPadding: 8
          text: qsTr("Buy License")
          icon.source: "qrc:/rcc/icons/buttons/buy.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]

          onClicked: {
            if (Cpp_QtCommercial_Available)
              Cpp_Licensing_LemonSqueezy.buy()
            else
              Qt.openUrlExternally("https://store.serial-studio.com")
          }
        }

        Button {
          icon.width: 18
          icon.height: 18
          horizontalPadding: 8
          text: qsTr("Activate")
          onClicked: app.showLicenseDialog()
          visible: Cpp_QtCommercial_Available
          icon.source: "qrc:/rcc/icons/buttons/activate.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }

        Item {
          Layout.fillWidth: true
        }
      }
    }

    //
    // Close button
    //
    Button {
      opacity: 0.8
      icon.width: 32
      icon.height: 32
      background: Item {}
      onClicked: root.hidden = true
      visible: root.closeButtonEnabled
      Layout.alignment: Qt.AlignRight | Qt.AlignTop
      icon.source: "qrc:/rcc/icons/buttons/cancel.svg"
      icon.color: Cpp_ThemeManager.colors["button_text"]
    }
  }
}
