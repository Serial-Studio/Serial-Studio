/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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
  title: qsTr("MQTT Publisher")
  icon: Cpp_Misc_IconRegistry.icon("editor", "mqtt-publisher", 16)

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
      // Action header: connection status + test/connect/CA-cert buttons
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

          //
          // Status indicator (LED with bloom glow, matches LED Panel widget)
          //
          Widgets.LedIndicator {
            diameter: 14
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            Layout.alignment: Qt.AlignVCenter
            on: Cpp_MQTT_Publisher.isConnected
            onColor: "#15803d"
            offColor: Cpp_ThemeManager.colors["alarm"]
          }

          Label {
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
            text: Cpp_MQTT_Publisher.isConnected
                  ? qsTr("Connected to broker")
                  : qsTr("Not connected")
          }

          Item {
            Layout.fillWidth: true
          }

          //
          // Test connection (probes broker, shows messagebox with result)
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Test Connection")
            enabled: Cpp_MQTT_Publisher.enabled
            Layout.alignment: Qt.AlignVCenter
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "test-connection", 24)
            ToolTip.text: Cpp_MQTT_Publisher.enabled
                          ? qsTr("Probe the broker with the current settings")
                          : qsTr("Enable publishing first")
            onClicked: Cpp_MQTT_Publisher.testConnection()
          }

          //
          // Edit script (only visible when mode == ScriptDriven == 1)
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Edit Script")
            Layout.alignment: Qt.AlignVCenter
            visible: Cpp_MQTT_Publisher.mode === 1
            enabled: Cpp_MQTT_Publisher.enabled
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "edit-code", 24)
            ToolTip.text: Cpp_MQTT_Publisher.enabled
                          ? qsTr("Edit the publisher script (Lua or JavaScript)")
                          : qsTr("Enable publishing first")
            onClicked: Cpp_JSON_ProjectEditor.openMqttScriptEditor()
          }

          //
          // Load CA certificates from a folder (TLS only)
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Load CA Certs")
            enabled: Cpp_MQTT_Publisher.enabled && Cpp_MQTT_Publisher.sslEnabled
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "load-ca-certs", 24)
            ToolTip.text: !Cpp_MQTT_Publisher.enabled
                          ? qsTr("Enable publishing first")
                          : (Cpp_MQTT_Publisher.sslEnabled
                             ? qsTr("Add PEM certificates from a folder")
                             : qsTr("Enable SSL/TLS first"))
            onClicked: Cpp_MQTT_Publisher.addCaCertificates()
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
      // Publisher settings form
      //
      ScrollView {
        id: view

        contentWidth: width
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentHeight: delegate.implicitHeight
        ScrollBar.vertical.policy: delegate.implicitHeight > view.height
                                   ? ScrollBar.AlwaysOn
                                   : ScrollBar.AsNeeded

        TableDelegate {
          id: delegate

          width: parent.width
          headerVisible: false
          parameterWidth: Math.min(delegate.width * 0.3, 256)

          Binding {
            target: delegate
            property: "modelPointer"
            value: Cpp_JSON_ProjectEditor.mqttPublisherModel
          }
        }
      }
    }
  }
}
