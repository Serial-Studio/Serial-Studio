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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Window options
  //
  width: minimumWidth
  height: minimumHeight
  title: qsTr("Support Serial Studio")
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + 32
  maximumHeight: column.implicitHeight + 32
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

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
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    //
    // Window controls
    //
    ColumnLayout {
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
            text: qsTr("Support the development of %1!").arg(Cpp_AppName)
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
          Component.onCompleted: forceActiveFocus()
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
}
