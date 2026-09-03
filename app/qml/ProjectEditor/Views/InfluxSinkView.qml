/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  implicitWidth: 0
  implicitHeight: 0
  title: qsTr("InfluxDB Sink")
  icon: Cpp_Misc_IconRegistry.icon("editor", "influx", 16)

  actionComponent: EditorNavActions {}

  Page {
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

    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -10
    }

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      //
      // Action header: endpoint status + session counters
      //
      Rectangle {
        id: header

        z: 2
        Layout.fillWidth: true
        height: headerLayout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        RowLayout {
          id: headerLayout

          spacing: 4
          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Widgets.LedIndicator {
            diameter: 14
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            on: Cpp_InfluxDB_Export.isOpen
            Layout.alignment: Qt.AlignVCenter
            offColor: Cpp_ThemeManager.colors["alarm"]
            onColor: Cpp_ThemeManager.colors["alarm_ok"]
          }

          Label {
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
            text: Cpp_InfluxDB_Export.isOpen
                  ? qsTr("Writing to InfluxDB")
                  : (Cpp_InfluxDB_Export.lastError.length > 0
                     ? Cpp_InfluxDB_Export.lastError
                     : qsTr("Not writing"))
          }

          Item {
            Layout.fillWidth: true
          }

          Label {
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: qsTr("%1 written, %2 dropped, %3 errors")
                  .arg(Cpp_InfluxDB_Export.pointsWritten)
                  .arg(Cpp_InfluxDB_Export.pointsDropped)
                  .arg(Cpp_InfluxDB_Export.httpErrors)
          }
        }

        Rectangle {
          height: 1
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }
      }

      //
      // Sink settings form
      //
      ScrollView {
        id: view

        contentWidth: width
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentHeight: form.implicitHeight + 32
        ScrollBar.vertical.policy: form.implicitHeight > view.height
                                   ? ScrollBar.AlwaysOn
                                   : ScrollBar.AsNeeded

        GridLayout {
          id: form

          columns: 2
          rowSpacing: 8
          columnSpacing: 8

          anchors {
            margins: 16
            top: parent.top
            left: parent.left
            right: parent.right
          }

          //
          // Enabled
          //
          Label {
            text: qsTr("Enabled") + ":"
          } Switch {
            Layout.leftMargin: -6
            Layout.alignment: Qt.AlignLeft
            checked: Cpp_InfluxDB_Export.exportEnabled
            onCheckedChanged: {
              if (Cpp_InfluxDB_Export.exportEnabled !== checked)
                Cpp_InfluxDB_Export.exportEnabled = checked
            }
          }

          //
          // Server URL
          //
          Label {
            text: qsTr("Server URL") + ":"
          } Widgets.BoundField {
            Layout.fillWidth: true
            externalValue: Cpp_InfluxDB_Export.url
            placeholderText: qsTr("e.g. http://localhost:8086")
            onEdited: text => Cpp_InfluxDB_Export.url = text
          }

          //
          // Organization
          //
          Label {
            text: qsTr("Organization") + ":"
          } Widgets.BoundField {
            Layout.fillWidth: true
            placeholderText: qsTr("e.g. my-org")
            externalValue: Cpp_InfluxDB_Export.organization
            onEdited: text => Cpp_InfluxDB_Export.organization = text
          }

          //
          // Bucket
          //
          Label {
            text: qsTr("Bucket") + ":"
          } Widgets.BoundField {
            Layout.fillWidth: true
            placeholderText: qsTr("e.g. telemetry")
            externalValue: Cpp_InfluxDB_Export.bucket
            onEdited: text => Cpp_InfluxDB_Export.bucket = text
          }

          //
          // Measurement
          //
          Label {
            text: qsTr("Measurement") + ":"
          } Widgets.BoundField {
            Layout.fillWidth: true
            placeholderText: qsTr("e.g. serial_studio")
            externalValue: Cpp_InfluxDB_Export.measurement
            onEdited: text => Cpp_InfluxDB_Export.measurement = text
          }

          //
          // API token: write-only, never read back from the vault into this field
          //
          Label {
            text: qsTr("API Token") + ":"
          } Widgets.LineField {
            id: tokenField

            Layout.fillWidth: true
            echoMode: TextInput.Password
            onEditingFinished: {
              if (text.length > 0) {
                Cpp_InfluxDB_Export.setToken(text)
                text = ""
              }
            }
            placeholderText: Cpp_InfluxDB_Export.hasToken
                             ? qsTr("Stored; type a new token to replace it")
                             : qsTr("Paste the InfluxDB API token")
          }

          //
          // Hint
          //
          Item {
            implicitHeight: 1
          } Label {
            Layout.fillWidth: true
            wrapMode: Label.WordWrap
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: qsTr("Points are written to the InfluxDB 2.x HTTP API with nanosecond precision. The token is stored obfuscated in this machine's settings and never saved into the project file.")
          }

          Item {
            Layout.fillHeight: true
          }
        }
      }
    }
  }
}
