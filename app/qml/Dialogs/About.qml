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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  readonly property int year: new Date().getFullYear()

  //
  // Window options
  //
  title: qsTr("About")
  width: minimumWidth
  height: minimumHeight
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + root.titlebarHeight + 32
  maximumHeight: column.implicitHeight + root.titlebarHeight + 32
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Native window registration
  //
  property real titlebarHeight: 0
  onVisibleChanged: {
    if (visible) {
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["base"])
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
    }

    else {
      root.titlebarHeight = 0
      Cpp_NativeWindow.removeWindow(root)
    }
  }

  //
  // Background + window title on macOS
  //
  Rectangle {
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["window"]

    //
    // Drag the window anywhere
    //
    DragHandler {
      target: null
      onActiveChanged: {
        if (active)
          root.startSystemMove()
      }
    }

    //
    // Titlebar text
    //
    Label {
      text: root.title
      visible: root.titlebarHeight > 0
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

      anchors {
        topMargin: 6
        top: parent.top
        horizontalCenter: parent.horizontalCenter
      }
    }
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
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
      spacing: 4
      anchors.centerIn: parent

      RowLayout {
        id: _logo
        spacing: 8
        Layout.fillWidth: true

        Item {
          Layout.minimumWidth: 144
          Layout.maximumWidth: 144
          Layout.minimumHeight: 144
          Layout.maximumHeight: 144
          Layout.alignment: Qt.AlignVCenter

          Image {
            sourceSize.width: 128
            anchors.centerIn: parent
            source: Qt.platform.os === "osx" ? "qrc:/rcc/logo/icon-macOS.png" :
                                               "qrc:/rcc/logo/icon.svg"
          }
        }

        ColumnLayout {
          spacing: 0
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter

          Label {
            rightPadding: 16
            text: Application.displayName
            font: Cpp_Misc_CommonFonts.customUiFont(2, true)
          }

          Item {
            implicitHeight: 2
          }

          Label {
            opacity: 0.8
            text: qsTr("Version %1").arg(Cpp_AppVersion)
            font: Cpp_Misc_CommonFonts.customUiFont(1.5, false)
          }

          Item {
            implicitHeight: 4
          }

          Label {
            opacity: 0.8
            text: qsTr("Copyright © %1 %2").arg(root.year).arg(Cpp_AppOrganization)
          }

          Label {
            opacity: 0.8
            text: qsTr("All Rights Reserved")
            visible: Cpp_CommercialBuild
          }
        }
      }

      //
      // GPLv3 License label for Qt open source build
      //
      Label {
        opacity: 0.8
        Layout.fillWidth: true
        Layout.minimumWidth: 360
        visible: !Cpp_CommercialBuild
        Layout.maximumWidth: _logo.implicitWidth
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        text: qsTr("%1 is free software: you can redistribute it and/or modify " +
                   "it under the terms of the GNU General Public License as " +
                   "published by the Free Software Foundation; either version " +
                   "3 of the License, or (at your option) any later version.\n\n" +
                   "%1 is distributed in the hope that it will be useful, but " +
                   "WITHOUT ANY WARRANTY; without even the implied warranty of " +
                   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. " +
                   "See the GNU General Public License for more details.").arg(Cpp_AppName)
      }

      //
      // License label for activated commercial-build
      //
      Label {
        opacity: 0.8
        Layout.fillWidth: true
        Layout.maximumWidth: _logo.implicitWidth
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        visible: Cpp_CommercialBuild && Cpp_Licensing_LemonSqueezy.isActivated
        text: qsTr("This configuration is licensed for commercial and proprietary use. " +
                   "It may be used in closed-source and commercial applications, " +
                   "subject to the terms of the commercial license.")
      }

      //
      // License label for non-activated commercial-build
      //
      Label {
        opacity: 0.8
        Layout.fillWidth: true
        Layout.maximumWidth: _logo.implicitWidth
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        visible: Cpp_CommercialBuild && !Cpp_Licensing_LemonSqueezy.isActivated
        text: qsTr("This configuration is for personal and evaluation purposes only. " +
                   "Commercial use is prohibited unless a valid commercial " +
                   "license is activated.")
      }

      //
      // License label for all commercial builds
      //
      Label {
        opacity: 0.7
        Layout.fillWidth: true
        visible: Cpp_CommercialBuild
        Layout.maximumWidth: _logo.implicitWidth
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        text: qsTr("This software is provided 'as is' without warranty of " +
                   "any kind, express or implied, including but not limited " +
                   "to the warranties of merchantability or fitness for a " +
                   "particular purpose. In no event shall the author be " +
                   "liable for any damages arising from the use of this " +
                   "software.")
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 8
      }

      //
      // License management
      //
      Button {
        Layout.fillWidth: true
        text: qsTr("Manage License")
        visible: Cpp_CommercialBuild
        onClicked: app.showLicenseDialog()
      }

      //
      // Donate button (free & GPL3)
      //
      Button {
        text: qsTr("Donate")
        Layout.fillWidth: true
        visible: !app.proVersion
        onClicked: donateDialog.show()
      }

      //
      // Check for updates
      //
      Button {
        Layout.fillWidth: true
        text: qsTr("Check for Updates")
        onClicked: app.checkForUpdates()
      }

      //
      // License Agreement
      //
      Button {
        Layout.fillWidth: true
        text: qsTr("License Agreement")
        onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md")
      }

      //
      // Report bug
      //
      Button {
        Layout.fillWidth: true
        text: qsTr("Report Bug")
        onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/issues")
      }

      //
      // Acknowledgements
      //
      Button {
        Layout.fillWidth: true
        text: qsTr("Acknowledgements")
        onClicked: app.showAcknowledgements()
      }

      //
      // Website
      //
      Button {
        Layout.fillWidth: true
        text: qsTr("Website")
        onClicked: Qt.openUrlExternally("https://serial-studio.com/")
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 8
      }

      //
      // Close button
      //
      Button {
        text: qsTr("Close")
        Layout.fillWidth: true
        onClicked: root.close()
      }
    }
  }
}
