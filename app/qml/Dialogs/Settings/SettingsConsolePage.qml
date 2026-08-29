/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Item {
  id: root

  Layout.fillWidth: true
  Layout.fillHeight: true
  implicitHeight: consoleLayout.implicitHeight + 16

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
    color: Cpp_ThemeManager.colors["groupbox_background"]
  }

  GridLayout {
    id: consoleLayout

    columns: 2
    rowSpacing: 4
    columnSpacing: 8
    anchors.margins: 8
    anchors.fill: parent

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 2
      text: qsTr("Display")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } Rectangle {
      implicitHeight: 1
      Layout.columnSpan: 2
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    } Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    }

    Label {
      text: qsTr("Display Mode")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _displayMode

      Layout.fillWidth: true
      model: Cpp_Console_Handler.displayModes
      currentIndex: Cpp_Console_Handler.displayMode
      onActivated: (index) => {
        if (Cpp_Console_Handler.displayMode !== index)
          Cpp_Console_Handler.displayMode = index
      }

      Connections {
        target: Cpp_Console_Handler
        function onDisplayModeChanged() {
          _displayMode.currentIndex = Cpp_Console_Handler.displayMode
        }
      }
    }

    Label {
      text: qsTr("Font Family")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _consoleFontFamily

      Layout.fillWidth: true
      model: Cpp_Console_Handler.availableFonts
      currentIndex: Cpp_Console_Handler.fontFamilyIndex

      onActivated: {
        Cpp_Console_Handler.fontFamily = currentText
      }

      Connections {
        target: Cpp_Console_Handler
        function onFontFamilyChanged() {
          _consoleFontFamily.currentIndex = Cpp_Console_Handler.fontFamilyIndex
        }
      }
    }

    Label {
      text: qsTr("Font Size")
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _consoleFontSize

      to: 72
      from: 6
      editable: true
      Layout.fillWidth: true
      value: Cpp_Console_Handler.fontSize

      onValueModified: {
        Cpp_Console_Handler.fontSize = value
      }

      Connections {
        target: Cpp_Console_Handler
        function onFontSizeChanged() {
          _consoleFontSize.value = Cpp_Console_Handler.fontSize
        }
      }
    }

    Label {
      text: qsTr("Scrollback Lines")
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _scrollbackLines

      from: 100
      to: 100000
      stepSize: 100
      editable: true
      Layout.fillWidth: true
      value: Cpp_Console_Handler.scrollbackLines

      onValueModified: {
        Cpp_Console_Handler.scrollbackLines = value
      }

      Connections {
        target: Cpp_Console_Handler
        function onScrollbackLinesChanged() {
          _scrollbackLines.value = Cpp_Console_Handler.scrollbackLines
        }
      }
    }

    Label {
      text: qsTr("Show Timestamps")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      id: _showTimestamp

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Console_Handler.showTimestamp
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_Console_Handler.showTimestamp)
          Cpp_Console_Handler.showTimestamp = checked
      }

      Connections {
        target: Cpp_Console_Handler
        function onShowTimestampChanged() {
          _showTimestamp.checked = Cpp_Console_Handler.showTimestamp
        }
      }
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      text: qsTr("Data Transmission")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } Rectangle {
      implicitHeight: 1
      Layout.columnSpan: 2
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    } Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    }

    Label {
      text: qsTr("Line Ending")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _lineEnding

      Layout.fillWidth: true
      model: Cpp_Console_Handler.lineEndings
      currentIndex: Cpp_Console_Handler.lineEnding
      onActivated: (index) => {
        if (Cpp_Console_Handler.lineEnding !== index)
          Cpp_Console_Handler.lineEnding = index
      }

      Connections {
        target: Cpp_Console_Handler
        function onLineEndingChanged() {
          _lineEnding.currentIndex = Cpp_Console_Handler.lineEnding
        }
      }
    }

    Label {
      text: qsTr("Input Mode")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _dataMode

      Layout.fillWidth: true
      model: Cpp_Console_Handler.dataModes
      currentIndex: Cpp_Console_Handler.dataMode
      onActivated: (index) => {
        if (Cpp_Console_Handler.dataMode !== index)
          Cpp_Console_Handler.dataMode = index
      }

      Connections {
        target: Cpp_Console_Handler
        function onDataModeChanged() {
          _dataMode.currentIndex = Cpp_Console_Handler.dataMode
        }
      }
    }

    Label {
      text: qsTr("Text Encoding")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _encoding

      Layout.fillWidth: true
      model: Cpp_Console_Handler.textEncodings
      currentIndex: Cpp_Console_Handler.encoding
      onActivated: (index) => {
        if (Cpp_Console_Handler.encoding !== index)
          Cpp_Console_Handler.encoding = index
      }

      Connections {
        target: Cpp_Console_Handler
        function onEncodingChanged() {
          _encoding.currentIndex = Cpp_Console_Handler.encoding
        }
      }
    }

    Label {
      text: qsTr("Checksum")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _checksumMethod

      Layout.fillWidth: true
      model: Cpp_Console_Handler.checksumMethods
      currentIndex: Cpp_Console_Handler.checksumMethod
      onActivated: (index) => {
        if (Cpp_Console_Handler.checksumMethod !== index)
          Cpp_Console_Handler.checksumMethod = index
      }

      Connections {
        target: Cpp_Console_Handler
        function onChecksumMethodChanged() {
          _checksumMethod.currentIndex = Cpp_Console_Handler.checksumMethod
        }
      }
    }

    Label {
      text: qsTr("Echo Sent Data")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      id: _echo

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Console_Handler.echo
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_Console_Handler.echo)
          Cpp_Console_Handler.echo = checked
      }

      Connections {
        target: Cpp_Console_Handler
        function onEchoChanged() {
          _echo.checked = Cpp_Console_Handler.echo
        }
      }
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 2
      text: qsTr("Escape Codes")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } Rectangle {
      implicitHeight: 1
      Layout.columnSpan: 2
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    } Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    }

    Label {
      text: qsTr("VT100 Emulation")
      color: Cpp_ThemeManager.colors["text"]
      opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : 1
    } Switch {
      id: _vt100Emulation

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Console_Handler.vt100Emulation
      enabled: !Cpp_Console_Handler.imageWidgetActive
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : 1
      onCheckedChanged: {
        if (checked !== Cpp_Console_Handler.vt100Emulation)
          Cpp_Console_Handler.vt100Emulation = checked
      }

      Connections {
        target: Cpp_Console_Handler
        function onVt100EmulationChanged() {
          _vt100Emulation.checked = Cpp_Console_Handler.vt100Emulation
        }
      }
    }

    Label {
      text: qsTr("ANSI Colors")
      enabled: _vt100Emulation.checked
      color: Cpp_ThemeManager.colors["text"]
      opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : (enabled ? 1 : 0.5)
    } Switch {
      id: _ansiColors

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Console_Handler.ansiColors
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      enabled: _vt100Emulation.checked && !Cpp_Console_Handler.imageWidgetActive
      opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : (enabled ? 1 : 0.5)
      onCheckedChanged: {
        if (checked !== Cpp_Console_Handler.ansiColors)
          Cpp_Console_Handler.ansiColors = checked
      }

      Connections {
        target: Cpp_Console_Handler
        function onAnsiColorsChanged() {
          _ansiColors.checked = Cpp_Console_Handler.ansiColors
        }
      }
    }

    Item { Layout.fillHeight: true }
    Item { Layout.fillHeight: true }
  }
}
