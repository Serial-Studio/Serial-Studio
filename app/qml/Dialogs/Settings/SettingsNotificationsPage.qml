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

// Notifications page (Pro only): zero implicit height in GPL builds keeps the tab indices aligned.
Item {
  id: root

  Layout.fillWidth: true
  Layout.fillHeight: true
  implicitHeight: Cpp_CommercialBuild ? notificationsLayout.implicitHeight + 16 : 0

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    visible: Cpp_CommercialBuild
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  GridLayout {
    id: notificationsLayout

    columns: 2
    rowSpacing: 4
    columnSpacing: 8
    anchors.margins: 8
    anchors.fill: parent
    visible: Cpp_CommercialBuild

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 2
      text: qsTr("Delivery")
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
      text: qsTr("System Notifications")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Notifications.systemNotificationsEnabled
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_Notifications.systemNotificationsEnabled)
          Cpp_Notifications.systemNotificationsEnabled = checked
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
      text: qsTr("Show Warning/Critical events as OS desktop notifications "
                 + "when Serial Studio is not the foreground window.")
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      text: qsTr("Application Logs")
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
      text: qsTr("Route Warnings to Notifications")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_Notifications.routeWarningsToNotifications
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_Notifications.routeWarningsToNotifications)
          Cpp_Notifications.routeWarningsToNotifications = checked
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
      text: qsTr("Off by default — Qt and QML emit warnings frequently "
                 + "and enabling this can drown out real alarms. Critical "
                 + "messages are always routed regardless of this setting.")
    }

    Item { Layout.fillHeight: true }
    Item { Layout.fillHeight: true }
  }
}
