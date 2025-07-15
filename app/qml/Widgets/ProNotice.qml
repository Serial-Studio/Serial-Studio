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
    // Activate icon
    //
    Item {
      Layout.minimumWidth: 96
      Layout.maximumWidth: 96
      Layout.minimumHeight: 96
      Layout.maximumHeight: 96
      Layout.alignment: Qt.AlignVCenter

      Image {
        sourceSize.width: 72
        anchors.centerIn: parent
        source: "qrc:/rcc/icons/toolbar/activate.svg"
      }
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
          visible: !Cpp_CommercialBuild
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
            if (Cpp_CommercialBuild)
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
          visible: Cpp_CommercialBuild
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
