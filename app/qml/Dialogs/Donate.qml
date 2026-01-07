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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets"

SmartDialog {
  id: root

  //
  // Window options
  //
  staysOnTop: true
  title: qsTr("Support Serial Studio")

  //
  // This is used when the window is shown automatically
  // every now and then to the user.
  //
  function showAutomatically() {
    if (!app.proVersion)
      showNormal()
  }

  //
  // This is used when the user opens this dialog from
  // the "about" window.
  //
  function show() {
    showNormal()
  }

  //
  // Window controls
  //
  contentItem: ColumnLayout {
    id: column
    spacing: 16
    anchors.centerIn: parent

      RowLayout {
        spacing: 16
        Layout.fillWidth: true
        Layout.fillHeight: true

        Image {
          sourceSize: Qt.size(120, 120)
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/rcc/images/buy-qr.svg"

          Rectangle {
            border.width: 2
            color: "transparent"
            anchors.fill: parent
            border.color: "#000"
          }
        }

        ColumnLayout {
          spacing: 4
          Layout.fillWidth: true
          Layout.fillHeight: true

          Item {
            Layout.fillHeight: true
          }

          Label {
            id: title
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.customUiFont(1.33, true)
            text: qsTr("Support the development of %1!").arg("Serial Studio")
          }

          Item {
            Layout.fillHeight: true
          }

          Label {
            Layout.fillWidth: true
            Layout.maximumWidth: title.implicitWidth
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Serial Studio is free & open-source software supported by volunteers. " +
                       "Consider donating or obtaining a Pro license to support development efforts :)")
          }

          Item {
            Layout.fillHeight: true
          }

          Label {
            opacity: 0.8
            Layout.fillWidth: true
            Layout.maximumWidth: title.implicitWidth
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            text: qsTr("You can also support this project by sharing it, reporting bugs and proposing new features!")
          }

          Item {
            Layout.fillHeight: true
          }
        }
      }

      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Close")
          onClicked: root.close()
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }

        Item {
          Layout.fillWidth: true
        }

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Donate")
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/paypal.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          onClicked: {
            root.close()
            Qt.openUrlExternally("https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE")
          }
        }

        Button {
          icon.width: 18
          icon.height: 18
          highlighted: true
          horizontalPadding: 8
          Keys.onEnterPressed: clicked()
          Keys.onReturnPressed: clicked()
          Layout.alignment: Qt.AlignVCenter
          text: qsTr("Get Serial Studio Pro")
          Component.onCompleted: Qt.callLater(forceActiveFocus)
          icon.source: "qrc:/rcc/icons/buttons/buy.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          onClicked: {
            root.close()
            Qt.openUrlExternally("https://store.serial-studio.com/buy/ba46c099-0d51-4d98-9154-6be5c35bc1ec")
          }
        }
      }
    }
}
