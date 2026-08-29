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
  implicitHeight: startupLayout.implicitHeight + 16

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  GridLayout {
    id: startupLayout

    columns: 2
    rowSpacing: 4
    columnSpacing: 8
    anchors.margins: 8
    anchors.fill: parent

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
      visible: Cpp_Misc_GraphicsBackend.configurable
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 2
      text: qsTr("Graphics")
      visible: Cpp_Misc_GraphicsBackend.configurable
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } Rectangle {
      implicitHeight: 1
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: Cpp_Misc_GraphicsBackend.configurable
      color: Cpp_ThemeManager.colors["groupbox_border"]
    } Item {
      implicitHeight: 2
      Layout.columnSpan: 2
      visible: Cpp_Misc_GraphicsBackend.configurable
    }

    Label {
      text: qsTr("Rendering Backend")
      visible: Cpp_Misc_GraphicsBackend.configurable
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _rhiBackend

      Layout.fillWidth: true
      visible: Cpp_Misc_GraphicsBackend.configurable
      model: Cpp_Misc_GraphicsBackend.availableBackends.map(e => e.label)
      currentIndex: {
        const list = Cpp_Misc_GraphicsBackend.availableBackends
        for (let i = 0; i < list.length; ++i)
          if (list[i].id === Cpp_Misc_GraphicsBackend.currentBackend)
            return i

        return 0
      }

      onActivated: (index) => {
        const list = Cpp_Misc_GraphicsBackend.availableBackends
        if (index < 0 || index >= list.length)
          return

        const id = list[index].id
        if (id === Cpp_Misc_GraphicsBackend.currentBackend)
          return

        Cpp_Misc_GraphicsBackend.currentBackend = id
        Cpp_Misc_GraphicsBackend.promptRestartAndQuit()
      }
    }

    Label {
      text: qsTr("Display Scaling")
      visible: Cpp_Misc_HighDpiScaling.configurable
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _hidpiScaling

      Layout.fillWidth: true
      visible: Cpp_Misc_HighDpiScaling.configurable
      model: Cpp_Misc_HighDpiScaling.availableModes.map(e => e.label)
      currentIndex: {
        const list = Cpp_Misc_HighDpiScaling.availableModes
        for (let i = 0; i < list.length; ++i)
          if (list[i].id === Cpp_Misc_HighDpiScaling.currentMode)
            return i

        return 0
      }

      onActivated: (index) => {
        const list = Cpp_Misc_HighDpiScaling.availableModes
        if (index < 0 || index >= list.length)
          return

        const id = list[index].id
        if (id === Cpp_Misc_HighDpiScaling.currentMode)
          return

        Cpp_Misc_HighDpiScaling.currentMode = id
        Cpp_Misc_HighDpiScaling.promptRestartAndQuit()
      }
    }

    Label {
      text: qsTr("Custom Scale (%)")
      visible: Cpp_Misc_HighDpiScaling.configurable
               && Cpp_Misc_HighDpiScaling.customSelected
      color: Cpp_ThemeManager.colors["text"]
    } SpinBox {
      id: _hidpiPercent

      from: Cpp_Misc_HighDpiScaling.minimumPercent
      to: Cpp_Misc_HighDpiScaling.maximumPercent
      stepSize: 25
      editable: true
      Layout.fillWidth: true
      value: Cpp_Misc_HighDpiScaling.customPercent
      visible: Cpp_Misc_HighDpiScaling.configurable
               && Cpp_Misc_HighDpiScaling.customSelected
      onValueModified: {
        if (value === Cpp_Misc_HighDpiScaling.customPercent)
          return

        Cpp_Misc_HighDpiScaling.customPercent = value
        Cpp_Misc_HighDpiScaling.promptRestartAndQuit()
      }

      Connections {
        target: Cpp_Misc_HighDpiScaling
        function onCustomPercentChanged() {
          _hidpiPercent.value = Cpp_Misc_HighDpiScaling.customPercent
        }
      }
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: Cpp_Misc_GraphicsBackend.configurable ? 6 : 2
      text: qsTr("System")
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
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Apply Performance Hints")
    } Switch {
      id: _performanceMode

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Misc_ModuleManager.performanceMode
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_Misc_ModuleManager.performanceMode)
          Cpp_Misc_ModuleManager.performanceMode = checked
      }

      Connections {
        target: Cpp_Misc_ModuleManager
        function onPerformanceModeChanged() {
          _performanceMode.checked = Cpp_Misc_ModuleManager.performanceMode
        }
      }
    }

    Label {
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Keep Display Awake")
    } Switch {
      id: _inhibitIdleSleep

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Misc_ModuleManager.inhibitIdleSleep
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_Misc_ModuleManager.inhibitIdleSleep)
          Cpp_Misc_ModuleManager.inhibitIdleSleep = checked
      }

      Connections {
        target: Cpp_Misc_ModuleManager
        function onInhibitIdleSleepChanged() {
          _inhibitIdleSleep.checked = Cpp_Misc_ModuleManager.inhibitIdleSleep
        }
      }
    }

    Label {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.topMargin: -2
      opacity: 0.7
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
      text: qsTr("Performance hints raise process priority and opt out of OS "
                 + "power throttling. Changes take effect the next time Serial "
                 + "Studio starts.")
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      text: qsTr("Updates & News")
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
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Automatically Check for Updates")
    } Switch {
      id: _automaticUpdates

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Misc_ModuleManager.automaticUpdates
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_Misc_ModuleManager.automaticUpdates)
          Cpp_Misc_ModuleManager.automaticUpdates = checked
      }

      Connections {
        target: Cpp_Misc_ModuleManager
        function onAutomaticUpdatesChanged() {
          _automaticUpdates.checked = Cpp_Misc_ModuleManager.automaticUpdates
        }
      }
    }

    Label {
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Check for Extension Updates")
    } Switch {
      id: _extensionUpdateCheck

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_ExtensionManager.updateCheckEnabled
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_ExtensionManager.updateCheckEnabled)
          Cpp_ExtensionManager.updateCheckEnabled = checked
      }

      Connections {
        target: Cpp_ExtensionManager
        function onUpdatePolicyChanged() {
          _extensionUpdateCheck.checked = Cpp_ExtensionManager.updateCheckEnabled
        }
      }
    }

    Label {
      color: Cpp_ThemeManager.colors["text"]
      opacity: Cpp_ExtensionManager.updateCheckEnabled ? 1 : 0.5
      text: qsTr("Install Extension Updates Automatically")
    } Switch {
      id: _extensionAutoUpdate

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_ExtensionManager.automaticUpdates
      enabled: Cpp_ExtensionManager.updateCheckEnabled
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_ExtensionManager.automaticUpdates)
          Cpp_ExtensionManager.automaticUpdates = checked
      }

      Connections {
        target: Cpp_ExtensionManager
        function onUpdatePolicyChanged() {
          _extensionAutoUpdate.checked = Cpp_ExtensionManager.automaticUpdates
        }
      }
    }

    Label {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.topMargin: -2
      opacity: 0.7
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
      text: qsTr("Installed extensions and themes are checked when Serial Studio "
                 + "starts, together with application updates. Serial Studio asks "
                 + "before installing them unless automatic installation is enabled.")
    }

    Item { Layout.fillHeight: true }
    Item { Layout.fillHeight: true }
  }
}
