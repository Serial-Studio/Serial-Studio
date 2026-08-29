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

Item {
  id: root

  Layout.fillWidth: true
  Layout.fillHeight: true
  implicitHeight: taskbarLayout.implicitHeight + 16

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
    color: Cpp_ThemeManager.colors["groupbox_background"]
  }

  ColumnLayout {
    id: taskbarLayout

    spacing: 6
    anchors.margins: 8
    anchors.fill: parent

    //
    // Section: Behavior
    //
    Label {
      Layout.fillWidth: true
      text: qsTr("Behavior")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    Rectangle {
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    GridLayout {
      columns: 2
      rowSpacing: 4
      columnSpacing: 8
      Layout.topMargin: 4
      Layout.fillWidth: true

      Label {
        Layout.fillWidth: true
        text: qsTr("Always Show Taskbar Buttons")
        color: Cpp_ThemeManager.colors["text"]
      } Switch {
        id: _showTaskbarButtons

        Layout.rightMargin: -8
        Layout.alignment: Qt.AlignRight
        checked: Cpp_UI_TaskbarSettings.showTaskbarButtons
        palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
        onCheckedChanged: {
          if (checked !== Cpp_UI_TaskbarSettings.showTaskbarButtons)
            Cpp_UI_TaskbarSettings.showTaskbarButtons = checked
        }

        Connections {
          target: Cpp_UI_TaskbarSettings
          function onShowTaskbarButtonsChanged() {
            _showTaskbarButtons.checked = Cpp_UI_TaskbarSettings.showTaskbarButtons
          }
        }
      }

      Label {
        Layout.fillWidth: true
        text: qsTr("Show Search Field")
        color: Cpp_ThemeManager.colors["text"]
      } Switch {
        id: _searchEnabled

        Layout.rightMargin: -8
        Layout.alignment: Qt.AlignRight
        checked: Cpp_UI_TaskbarSettings.searchEnabled
        palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
        onCheckedChanged: {
          if (checked !== Cpp_UI_TaskbarSettings.searchEnabled)
            Cpp_UI_TaskbarSettings.searchEnabled = checked
        }

        Connections {
          target: Cpp_UI_TaskbarSettings
          function onSearchEnabledChanged() {
            _searchEnabled.checked = Cpp_UI_TaskbarSettings.searchEnabled
          }
        }
      }

      Label {
        Layout.fillWidth: true
        text: qsTr("Auto-hide Taskbar")
        color: Cpp_ThemeManager.colors["text"]
      } Switch {
        id: _taskbarAutohide

        Layout.rightMargin: -8
        Layout.alignment: Qt.AlignRight
        checked: Cpp_UI_TaskbarSettings.autohide
        palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
        onCheckedChanged: {
          if (checked !== Cpp_UI_TaskbarSettings.autohide)
            Cpp_UI_TaskbarSettings.autohide = checked
        }

        Connections {
          target: Cpp_UI_TaskbarSettings
          function onAutohideChanged() {
            _taskbarAutohide.checked = Cpp_UI_TaskbarSettings.autohide
          }
        }
      }

      Label {
        Layout.fillWidth: true
        Layout.leftMargin: 16
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_UI_TaskbarSettings.autohide
        text: qsTr("Hide Delay (ms)")
        color: Cpp_ThemeManager.colors["text"]
      } SpinBox {
        id: _autohideDelay

        from: 200
        to: 10000
        stepSize: 100
        editable: true
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignRight
        enabled: Cpp_UI_TaskbarSettings.autohide
        value: Cpp_UI_TaskbarSettings.autohideDelayMs
        onValueModified: {
          if (value !== Cpp_UI_TaskbarSettings.autohideDelayMs)
            Cpp_UI_TaskbarSettings.autohideDelayMs = value
        }

        Connections {
          target: Cpp_UI_TaskbarSettings
          function onAutohideDelayMsChanged() {
            _autohideDelay.value = Cpp_UI_TaskbarSettings.autohideDelayMs
          }
        }
      }
    }

    //
    // Section: Pinned buttons
    //
    Label {
      Layout.fillWidth: true
      Layout.topMargin: 12
      text: qsTr("Pinned Buttons")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    Rectangle {
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    Label {
      Layout.fillWidth: true
      wrapMode: Text.Wrap
      opacity: 0.7
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Drag a pinned button on the taskbar to reorder it.")
    }

    //
    // Pin list
    //
    ListView {
      id: pinList

      clip: true
      spacing: 1
      interactive: true
      Layout.fillWidth: true
      implicitHeight: 32 * count
      boundsBehavior: Flickable.StopAtBounds

      readonly property var rows: {
        void Cpp_UI_TaskbarSettings.pinnedButtons
        const all    = Cpp_UI_TaskbarSettings.availableButtons
        const pinned = Cpp_UI_TaskbarSettings.pinnedButtons
        const labels = {
          "settings": qsTr("Settings"),
          "console": qsTr("Console"),
          "notifications": qsTr("Notifications"),
          "clock": qsTr("Clock"),
          "stopwatch": qsTr("Stopwatch"),
          "pause": qsTr("Pause / Resume"),
          "file_transmission": qsTr("File Transmission"),
          "ai_assistant": qsTr("AI Assistant")
        }
        const icons = {
          "settings": Cpp_Misc_IconRegistry.icon("commands", "settings", 24),
          "console": Cpp_Misc_IconRegistry.icon("commands", "console", 24),
          "notifications": Cpp_Misc_IconRegistry.icon("widgets", "notification-log", 24),
          "clock": Cpp_Misc_IconRegistry.icon("widgets", "clock", 24),
          "stopwatch": Cpp_Misc_IconRegistry.icon("widgets", "stopwatch", 24),
          "pause": Cpp_Misc_IconRegistry.icon("commands", "pause", 24),
          "file_transmission":
            Cpp_Misc_IconRegistry.icon("commands", "file-transmission", 24),
          "ai_assistant": Cpp_Misc_IconRegistry.icon("commands", "ai", 24)
        }

        function shouldShow(id) {
          if (id === "notifications"
              || id === "file_transmission"
              || id === "ai_assistant")
            return Cpp_CommercialBuild

          return true
        }

        const out = []
        for (let i = 0; i < all.length; ++i) {
          const id = all[i]
          if (!shouldShow(id))
            continue

          out.push({
            id: id,
            pinned: pinned.indexOf(id) >= 0,
            label: labels[id] || id,
            icon: icons[id] || ""
          })
        }
        return out
      }

      model: rows

      delegate: Rectangle {
        id: pinRow_

        required property int index
        required property var modelData

        width: ListView.view.width
        height: 30
        color: _ma.containsMouse
               ? Cpp_ThemeManager.colors["start_menu_highlight"]
               : "transparent"

        MouseArea {
          id: _ma

          hoverEnabled: true
          anchors.fill: parent
          acceptedButtons: Qt.NoButton
        }

        RowLayout {
          spacing: 8
          anchors.fill: parent
          anchors.leftMargin: 6
          anchors.rightMargin: 6

          CheckBox {
            Layout.alignment: Qt.AlignVCenter
            checked: pinRow_.modelData.pinned
            onToggled: Cpp_UI_TaskbarSettings.setButtonPinned(
                         pinRow_.modelData.id, checked)
          }

          Image {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            sourceSize: Qt.size(24, 24)
            source: pinRow_.modelData.icon
            Layout.alignment: Qt.AlignVCenter
          }

          Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.uiFont
            text: pinRow_.modelData.label
            color: Cpp_ThemeManager.colors["text"]
          }
        }
      }
    }

    Item { Layout.fillHeight: true }
  }
}
