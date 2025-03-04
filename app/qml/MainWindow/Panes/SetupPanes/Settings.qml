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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root
  implicitHeight: layout.implicitHeight + 16

  //
  // Access to properties
  //
  property alias tcpPlugins: _tcpPlugins.checked
  property alias language: _langCombo.currentIndex

  //
  // Background
  //
  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  //
  // Layout
  //
  ColumnLayout {
    id: layout
    anchors.fill: parent
    anchors.margins: 8

    //
    // Controls
    //
    GridLayout {
      columns: 2
      Layout.fillWidth: true
      rowSpacing: 8 / 2
      columnSpacing: 8 / 2

      //
      // Language selector
      //
      Label {
        text: qsTr("Language") + ":"
      } ComboBox {
        id: _langCombo
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.connected
        currentIndex: Cpp_Misc_Translator.language
        model: Cpp_Misc_Translator.availableLanguages
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_Misc_Translator.language)
            Cpp_Misc_Translator.setLanguage(currentIndex)
        }
      }

      //
      // Theme selector
      //
      Label {
        text: qsTr("Theme") + ":"
        visible: Cpp_CommercialBuild
      } ComboBox {
        id: _themeCombo
        Layout.fillWidth: true
        visible: Cpp_CommercialBuild
        currentIndex: Cpp_ThemeManager.theme
        model: Cpp_ThemeManager.availableThemes
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_ThemeManager.theme)
            Cpp_ThemeManager.setTheme(currentIndex)
        }
      }

      //
      // Plugins enabled
      //
      Label {
        text: qsTr("Plugin System") + ": "
      } Switch {
        id: _tcpPlugins
        Layout.leftMargin: -8
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_Plugins_Bridge.enabled
        palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
        onCheckedChanged: {
          if (checked !== Cpp_Plugins_Bridge.enabled)
            Cpp_Plugins_Bridge.enabled = checked
        }
      }

      //
      // Auto-updater
      //
      Label {
        text: qsTr("Automatic Updates") + ":"
      } Switch {
        Layout.leftMargin: -8
        Layout.alignment: Qt.AlignLeft
        checked: mainWindow.automaticUpdates
        palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
        onCheckedChanged: {
          if (checked !== mainWindow.automaticUpdates)
            mainWindow.automaticUpdates = checked
        }
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.minimumHeight: 16
    }

    //
    // Plugins label
    //
    RowLayout {
      spacing: 8

      Image {
        sourceSize: Qt.size(48, 48)
        source: "qrc:/rcc/images/tip.svg"
        Layout.alignment: Qt.AlignVCenter
      }

      Label {
        opacity: 0.6
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        text: qsTr("Using the plugin system, other applications \& scripts can " +
                   "interact with %1 by establishing a TCP connection on port " +
                   "7777.").arg(Cpp_AppName)
      }

      Item {
        Layout.minimumWidth: 16
      }
    }

    //
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
