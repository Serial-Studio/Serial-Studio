/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
      } ComboBox {
        id: _themeCombo
        Layout.fillWidth: true
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
