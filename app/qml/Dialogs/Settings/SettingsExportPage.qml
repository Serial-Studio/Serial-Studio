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
  implicitHeight: exportLayout.implicitHeight + 16

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
    color: Cpp_ThemeManager.colors["groupbox_background"]
  }

  GridLayout {
    id: exportLayout

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
      text: qsTr("CSV Export")
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
      text: qsTr("Row Interval (ms)")
      color: Cpp_ThemeManager.colors["text"]
    } ComboBox {
      id: _csvIntervalCombo

      editable: true
      Layout.fillWidth: true

      property bool initializing: true

      validator: IntValidator { bottom: 0 }

      model: ["0", "10", "100", "1000"]

      function syncFromBackend() {
        const current = String(Cpp_CSV_Export.exportInterval)
        const idx = model.indexOf(current)
        if (idx !== -1)
          _csvIntervalCombo.currentIndex = idx
        else {
          _csvIntervalCombo.currentIndex = -1
          _csvIntervalCombo.editText = current
        }
      }

      function commit(text) {
        if (initializing)
          return

        const value = parseInt(text)
        if (!isNaN(value) && value >= 0
            && Cpp_CSV_Export.exportInterval !== value)
          Cpp_CSV_Export.exportInterval = value
      }

      Component.onCompleted: {
        Qt.callLater(() => {
          syncFromBackend()
          initializing = false
        })
      }

      Connections {
        target: Cpp_CSV_Export
        function onIntervalChanged() {
          if (!_csvIntervalCombo.initializing)
            _csvIntervalCombo.syncFromBackend()
        }
      }

      onAccepted: commit(editText)
      onActivated: (index) => commit(model[index])
    }

    Label {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.topMargin: -2
      opacity: 0.7
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
      text: qsTr("0 writes one row per received frame. A positive value logs one "
                 + "snapshot row of every channel at that interval, which keeps file "
                 + "size bounded for multi-source or high-rate projects.")
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
      visible: Cpp_CommercialBuild
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      visible: Cpp_CommercialBuild
      text: qsTr("Video Export")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } Rectangle {
      implicitHeight: 1
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: Cpp_CommercialBuild
      color: Cpp_ThemeManager.colors["groupbox_border"]
    } Item {
      implicitHeight: 2
      Layout.columnSpan: 2
      visible: Cpp_CommercialBuild
    }

    Label {
      visible: Cpp_CommercialBuild
      text: qsTr("Save Videos by Default")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      id: _saveImages

      Layout.rightMargin: -8
      visible: Cpp_CommercialBuild
      Layout.alignment: Qt.AlignRight
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      checked: Cpp_CommercialBuild && Cpp_Image_Export.exportEnabled
      onCheckedChanged: {
        if (Cpp_CommercialBuild && checked !== Cpp_Image_Export.exportEnabled)
          Cpp_Image_Export.exportEnabled = checked
      }
    }

    Item { Layout.fillHeight: true }
    Item { Layout.fillHeight: true }
  }
}
