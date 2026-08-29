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
  implicitHeight: dashboardLayout.implicitHeight + 16

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
    color: Cpp_ThemeManager.colors["groupbox_background"]
  }

  GridLayout {
    id: dashboardLayout

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
      text: qsTr("Dashboard Font")
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
      text: qsTr("Font Family")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _widgetFontFamily

      Layout.fillWidth: true
      model: Cpp_Misc_CommonFonts.availableFonts
      currentIndex: Cpp_Misc_CommonFonts.widgetFontIndex

      onActivated: {
        Cpp_Misc_CommonFonts.widgetFontFamily = currentText
      }

      Connections {
        target: Cpp_Misc_CommonFonts
        function onFontsChanged() {
          _widgetFontFamily.currentIndex = Cpp_Misc_CommonFonts.widgetFontIndex
        }
      }
    }

    Label {
      text: qsTr("Font Size")
      color: Cpp_ThemeManager.colors["text"]
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.Combo {
        id: _widgetSizePreset

        Layout.fillWidth: true
        model: [qsTr("Small"), qsTr("Normal"), qsTr("Large"), qsTr("Extra Large"), qsTr("Custom")]

        function syncFromScale() {
          const scale = Cpp_Misc_CommonFonts.widgetFontScale
          if (Math.abs(scale - 0.85) < 0.01)
            currentIndex = 0
          else if (Math.abs(scale - 1.00) < 0.01)
            currentIndex = 1
          else if (Math.abs(scale - 1.25) < 0.01)
            currentIndex = 2
          else if (Math.abs(scale - 1.50) < 0.01)
            currentIndex = 3
          else
            currentIndex = 4
        }

        Component.onCompleted: syncFromScale()

        onActivated: (index) => {
                       const scales = [0.85, 1.00, 1.25, 1.50]
                       if (index < 4)
                         Cpp_Misc_CommonFonts.widgetFontScale = scales[index]
                     }

        Connections {
          target: Cpp_Misc_CommonFonts
          function onFontsChanged() { _widgetSizePreset.syncFromScale() }
        }
      }

      SpinBox {
        id: _widgetFontCustom

        to: 300
        from: 50
        editable: true
        visible: _widgetSizePreset.currentIndex === 4
        value: Math.round(Cpp_Misc_CommonFonts.widgetFontScale * 100)

        textFromValue: (val) => val + "%"
        valueFromText: (text) => parseInt(text)

        onValueModified: {
          Cpp_Misc_CommonFonts.widgetFontScale = value / 100.0
        }

        Connections {
          target: Cpp_Misc_CommonFonts
          function onFontsChanged() {
            _widgetFontCustom.value = Math.round(Cpp_Misc_CommonFonts.widgetFontScale * 100)
          }
        }
      }
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      text: qsTr("Layout")
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
      text: qsTr("Layout Margin")
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _layoutMargin

      from: 0
      stepSize: 1
      editable: true
      to: 2147483647
      Layout.fillWidth: true
      value: Cpp_UI_Dashboard.layoutMargin
      onValueChanged: {
        if (value !== Cpp_UI_Dashboard.layoutMargin)
          Cpp_UI_Dashboard.layoutMargin = value
      }

      Connections {
        target: Cpp_UI_Dashboard
        function onLayoutMarginChanged() {
          _layoutMargin.value = Cpp_UI_Dashboard.layoutMargin
        }
      }
    }

    Label {
      text: qsTr("Layout Spacing")
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _layoutSpacing

      from: -1
      stepSize: 1
      editable: true
      to: 2147483647
      Layout.fillWidth: true
      value: Cpp_UI_Dashboard.layoutSpacing
      onValueChanged: {
        if (value !== Cpp_UI_Dashboard.layoutSpacing)
          Cpp_UI_Dashboard.layoutSpacing = value
      }

      Connections {
        target: Cpp_UI_Dashboard
        function onLayoutSpacingChanged() {
          _layoutSpacing.value = Cpp_UI_Dashboard.layoutSpacing
        }
      }
    }

    Label {
      text: qsTr("Show Actions Panel")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      id: _actionsPanel

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_UI_Dashboard.showActionPanel
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_UI_Dashboard.showActionPanel)
          Cpp_UI_Dashboard.showActionPanel = checked
      }
    }

    Label {
      text: qsTr("Auto-Hide Toolbar")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      id: _autoHideToolbar

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_UI_Dashboard.autoHideToolbar
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_UI_Dashboard.autoHideToolbar)
          Cpp_UI_Dashboard.autoHideToolbar = checked
      }

      Connections {
        target: Cpp_UI_Dashboard
        function onAutoHideToolbarChanged() {
          _autoHideToolbar.checked = Cpp_UI_Dashboard.autoHideToolbar
        }
      }
    }

    Label {
      text: qsTr("Show Alignment Guides")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      id: _showAlignmentGuides

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_UI_Dashboard.showAlignmentGuides
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_UI_Dashboard.showAlignmentGuides)
          Cpp_UI_Dashboard.showAlignmentGuides = checked
      }

      Connections {
        target: Cpp_UI_Dashboard
        function onShowAlignmentGuidesChanged() {
          _showAlignmentGuides.checked = Cpp_UI_Dashboard.showAlignmentGuides
        }
      }
    }

    Item { Layout.fillHeight: true }
    Item { Layout.fillHeight: true }
  }
}
