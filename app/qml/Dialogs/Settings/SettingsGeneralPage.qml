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
  implicitHeight: generalLayout.implicitHeight + 16

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  GridLayout {
    id: generalLayout

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
      text: qsTr("Appearance")
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
      text: qsTr("Language")
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      currentIndex: Cpp_Misc_Translator.language
      model: Cpp_Misc_Translator.availableLanguages
      onActivated: {
        Cpp_Misc_Translator.language = currentIndex
      }
    }

    Label {
      text: qsTr("Theme")
      color: Cpp_ThemeManager.colors["text"]
    } Widgets.Combo {
      id: _themeCombo

      Layout.fillWidth: true
      currentIndex: Cpp_ThemeManager.theme
      model: Cpp_ThemeManager.availableThemes
      onActivated: {
        Cpp_ThemeManager.theme = currentIndex
      }

      Connections {
        target: Cpp_ThemeManager
        function onThemeChanged() {
          _themeCombo.currentIndex = Cpp_ThemeManager.theme
        }
      }
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
      visible: Cpp_NativeWindow.csdAvailable
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      text: qsTr("Window")
      visible: Cpp_NativeWindow.csdAvailable
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } Rectangle {
      implicitHeight: 1
      Layout.columnSpan: 2
      Layout.fillWidth: true
      visible: Cpp_NativeWindow.csdAvailable
      color: Cpp_ThemeManager.colors["groupbox_border"]
    } Item {
      implicitHeight: 2
      Layout.columnSpan: 2
      visible: Cpp_NativeWindow.csdAvailable
    }

    Label {
      visible: Cpp_NativeWindow.csdAvailable
      text: qsTr("Custom Window Decorations")
      color: Cpp_ThemeManager.colors["text"]
    } Switch {
      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      visible: Cpp_NativeWindow.csdAvailable
      checked: Cpp_NativeWindow.csdEnabled
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_NativeWindow.csdEnabled)
          Cpp_NativeWindow.csdEnabled = checked
      }
    }

    Label {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.topMargin: -2
      opacity: 0.7
      wrapMode: Text.WordWrap
      visible: Cpp_NativeWindow.csdAvailable
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
      text: qsTr("Window decoration changes apply after restarting %1.").arg(Cpp_AppName)
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      text: qsTr("Files")
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
      opacity: enabled ? 1 : 0.5
      text: qsTr("Workspace Folder")
      enabled: !Cpp_IO_Manager.isConnected
      color: Cpp_ThemeManager.colors["text"]
    } RowLayout {
      spacing: 2
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected

      Widgets.LineField {
        readOnly: true
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        text: Cpp_Misc_WorkspaceManager.shortPath
      }

      Widgets.IconButton {
        Layout.fillWidth: false
        Layout.maximumWidth: 24
        Layout.maximumHeight: 24
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_Misc_WorkspaceManager.selectPath()
        icon.source: "qrc:/icons/buttons/open.svg"
      }
    }

    Item {
      implicitHeight: 2
      Layout.columnSpan: 2
    } Label {
      Layout.columnSpan: 2
      Layout.topMargin: 6
      text: qsTr("API & Plugins")
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
      text: qsTr("Enable API Server")
    } Switch {
      id: _apiServer

      Layout.rightMargin: -8
      Layout.alignment: Qt.AlignRight
      checked: Cpp_API_Server.enabled
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_API_Server.enabled)
          Cpp_API_Server.enabled = checked
      }

      Connections {
        target: Cpp_API_Server
        function onEnabledChanged() {
          _apiServer.checked = Cpp_API_Server.enabled
        }
      }
    }

    Label {
      enabled: _apiServer.checked
      opacity: enabled ? 1 : 0.5
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("API Server Port")
    } SpinBox {
      id: _apiPort

      to: 65535
      from: 1024
      editable: true
      value: Cpp_API_Server.port
      opacity: enabled ? 1 : 0.5
      enabled: _apiServer.checked
      Layout.alignment: Qt.AlignRight
      onValueModified: Cpp_API_Server.port = value

      Connections {
        target: Cpp_API_Server
        function onPortChanged() {
          _apiPort.value = Cpp_API_Server.port
        }
      }
    }

    Label {
      enabled: _apiServer.checked
      opacity: enabled ? 1 : 0.5
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Allow External API Connections")
    } Switch {
      id: _apiExternal

      Layout.rightMargin: -8
      opacity: enabled ? 1 : 0.5
      enabled: _apiServer.checked
      Layout.alignment: Qt.AlignRight
      checked: Cpp_API_Server.externalConnections
      palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
      onCheckedChanged: {
        if (checked !== Cpp_API_Server.externalConnections)
          Cpp_API_Server.externalConnections = checked
      }

      Connections {
        target: Cpp_API_Server
        function onExternalConnectionsChanged() {
          _apiExternal.checked = Cpp_API_Server.externalConnections
        }
      }
    }

    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("API Access Token")
      color: Cpp_ThemeManager.colors["text"]
      enabled: _apiServer.checked && Cpp_API_Server.externalConnections
    } RowLayout {
      spacing: 2
      opacity: enabled ? 1 : 0.5
      enabled: _apiServer.checked && Cpp_API_Server.externalConnections

      Widgets.LineField {
        readOnly: true
        Layout.fillWidth: true
        text: Cpp_API_Server.authToken
        Layout.alignment: Qt.AlignVCenter
      }

      Widgets.IconButton {
        Layout.fillWidth: false
        Layout.maximumWidth: 24
        Layout.maximumHeight: 24
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/refresh.svg"
        onClicked: Cpp_API_Server.regenerateAuthToken()
      }
    }

    Label {
      visible: Cpp_GrpcAvailable
      opacity: enabled ? 1 : 0.5
      enabled: _apiServer.checked
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Export Protobuf File")
    } Button {
      text: qsTr("Export…")
      visible: Cpp_GrpcAvailable
      opacity: enabled ? 1 : 0.5
      enabled: _apiServer.checked
      Layout.alignment: Qt.AlignRight
      onClicked: {
        if (Cpp_GrpcAvailable && Cpp_GRPC_Server)
          Cpp_GRPC_Server.exportProto()
      }
    }

    Item { Layout.fillHeight: true }
    Item { Layout.fillHeight: true }
  }
}
