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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  title: qsTr("Recovery Options")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Action handler: applies the recovery and quits so the next launch is clean
  //
  function applyAndQuit(action) {
    Cpp_Misc_CrashTracker.applyRecovery(action)
    Qt.quit()
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12

    //
    // Header
    //
    Label {
      Layout.fillWidth: true
      Layout.maximumWidth: 460
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.boldUiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Serial Studio has closed unexpectedly several times in a row.")
    }

    //
    // Context block
    //
    Rectangle {
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
      Layout.preferredHeight: detailsLayout.implicitHeight + 16

      ColumnLayout {
        id: detailsLayout

        spacing: 4
        anchors.margins: 8
        anchors.fill: parent

        Label {
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("Consecutive crashes: %1").arg(Cpp_Misc_CrashTracker.consecutiveCrashes)
        }

        Label {
          Layout.fillWidth: true
          visible: Cpp_Misc_CrashTracker.lastCheckpoint.length > 0
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("Last reported stage: %1").arg(Cpp_Misc_CrashTracker.lastCheckpoint)
        }

        Label {
          Layout.fillWidth: true
          visible: Cpp_Misc_CrashTracker.lastCrashAt.length > 0
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("Detected at: %1").arg(Cpp_Misc_CrashTracker.lastCrashAt)
        }
      }
    }

    //
    // Action label
    //
    Label {
      Layout.fillWidth: true
      Layout.maximumWidth: 460
      Layout.topMargin: 4
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.")
    }

    //
    // Recovery options
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.IconButton {
        horizontalPadding: 8
        Layout.fillWidth: true
        icon.source: "qrc:/icons/buttons/refresh.svg"
        text: qsTr("Reset Rendering Backend to Default")
        onClicked: root.applyAndQuit(1)
      }

      Widgets.IconButton {
        horizontalPadding: 8
        Layout.fillWidth: true
        icon.source: "qrc:/icons/buttons/refresh.svg"
        text: qsTr("Skip Restoring the Last Opened Project")
        onClicked: root.applyAndQuit(2)
      }

      Widgets.IconButton {
        horizontalPadding: 8
        Layout.fillWidth: true
        text: qsTr("Reset all Preferences")
        icon.source: "qrc:/icons/buttons/refresh.svg"
        onClicked: root.applyAndQuit(3)
      }
    }

    //
    // Continue
    //
    RowLayout {
      spacing: 4
      Layout.topMargin: 8
      Layout.fillWidth: true

      Item { Layout.fillWidth: true }

      Widgets.IconButton {
        horizontalPadding: 8
        text: qsTr("Continue Anyway")
        icon.source: "qrc:/icons/buttons/apply.svg"
        onClicked: {
          Cpp_Misc_CrashTracker.acknowledge()
          root.close()
          app.continueBoot()
        }
      }
    }
  }
}
