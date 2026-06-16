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

  //
  // Window options
  //
  title: qsTr("Preferences")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Window controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent

    //
    // Tab bar
    //
    TabBar {
      id: _tab

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        text: qsTr("General")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Startup")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Dashboard")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Taskbar")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Console")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        height: _tab.height + 3
        text: qsTr("Notifications")
        visible: Cpp_CommercialBuild
        width: visible ? implicitWidth + 2 * 8 : 0
      }
    }

    //
    // Tab contents
    //
    StackLayout {
      id: stack

      clip: true
      Layout.fillWidth: true
      Layout.minimumWidth: 480
      currentIndex: _tab.currentIndex
      Layout.topMargin: -parent.spacing - 1
      implicitHeight: Math.max(
                        generalTab.implicitHeight,
                        startupTab.implicitHeight,
                        dashboardTab.implicitHeight,
                        taskbarTab.implicitHeight,
                        consoleTab.implicitHeight,
                        Cpp_CommercialBuild ? notificationsTab.implicitHeight : 0
                        )

      //
      // General tab
      //
      Item {
        id: generalTab

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
            text: qsTr("Advanced")
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
          }

          Label {
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Enable API Server (Port 7777)")
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
          }

          Label {
            enabled: _apiServer.checked
            opacity: enabled ? 1 : 0.5
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Allow External API Connections")
          } Switch {
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

      //
      // Startup tab
      //
      Item {
        id: startupTab

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
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Misc_ModuleManager.performanceMode
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Misc_ModuleManager.performanceMode)
                Cpp_Misc_ModuleManager.performanceMode = checked
            }
          }

          Label {
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Keep Display Awake")
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Misc_ModuleManager.inhibitIdleSleep
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Misc_ModuleManager.inhibitIdleSleep)
                Cpp_Misc_ModuleManager.inhibitIdleSleep = checked
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
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Misc_ModuleManager.automaticUpdates
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Misc_ModuleManager.automaticUpdates)
                Cpp_Misc_ModuleManager.automaticUpdates = checked
            }
          }

          Label {
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Show What's New on Startup")
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Misc_WhatsNew.showWhatsNewOnStartup
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Misc_WhatsNew.showWhatsNewOnStartup)
                Cpp_Misc_WhatsNew.showWhatsNewOnStartup = checked
            }
          }

          Item { Layout.fillHeight: true }
          Item { Layout.fillHeight: true }
        }
      }

      //
      // Dashboard tab
      //
      Item {
        id: dashboardTab

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
            Layout.topMargin: 2
            Layout.columnSpan: 2
            text: qsTr("Data Plotting")
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
            text: qsTr("Time Range")
            color: Cpp_ThemeManager.colors["text"]
          } SpinBox {
            id: _timeRange

            readonly property var presets: [0.001, 0.002, 0.005, 0.01, 0.02, 0.05,
                                            0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 30, 60, 120, 300]

            function nearestIndex(seconds) {
              var best = 0
              for (var i = 0; i < presets.length; ++i)
                if (Math.abs(presets[i] - seconds) < Math.abs(presets[best] - seconds))
                  best = i

              return best
            }

            function formatSeconds(s) {
              return s < 1 ? (Math.round(s * 1000) + " ms") : (parseFloat(s.toFixed(3)) + " s")
            }

            from: 0
            to: presets.length - 1
            editable: true
            Layout.fillWidth: true
            Component.onCompleted: value = nearestIndex(Cpp_UI_Dashboard.plotTimeRange)
            textFromValue: function(value, locale) { return _timeRange.formatSeconds(presets[value]) }
            valueFromText: function(text, locale) {
              var t = String(text).toLowerCase()
              var num = parseFloat(t.replace(/[^0-9.]/g, ""))
              if (isNaN(num))
                return _timeRange.value

              var secs = (t.indexOf("ms") >= 0) ? num / 1000 : num
              return _timeRange.nearestIndex(secs)
            }
            onValueModified: {
              if (presets[value] !== Cpp_UI_Dashboard.plotTimeRange)
                Cpp_UI_Dashboard.plotTimeRange = presets[value]
            }

            Connections {
              target: Cpp_UI_Dashboard
              function onPlotTimeRangeChanged() {
                const idx = _timeRange.nearestIndex(Cpp_UI_Dashboard.plotTimeRange)
                if (_timeRange.value !== idx)
                  _timeRange.value = idx
              }
            }
          }

          Label {
            text: qsTr("Point Count")
            color: Cpp_ThemeManager.colors["text"]
          } SpinBox {
            id: _pointCount

            from: 10
            to: 100000
            stepSize: 10
            editable: true
            Layout.fillWidth: true
            value: Cpp_UI_Dashboard.points
            onValueChanged: {
              if (value !== Cpp_UI_Dashboard.points)
                Cpp_UI_Dashboard.points = value
            }

            Connections {
              target: Cpp_UI_Dashboard
              function onPointsChanged() {
                _pointCount.value = Cpp_UI_Dashboard.points
              }
            }
          }

          Label {
            text: qsTr("UI Refresh Rate (Hz)")
            color: Cpp_ThemeManager.colors["text"]
          } SpinBox {
            id: _refreshRate

            from: 1
            to: 240
            editable: true
            Layout.fillWidth: true
            value: Cpp_Misc_TimerEvents.fps
            onValueChanged: {
              if (value !== Cpp_Misc_TimerEvents.fps)
                Cpp_Misc_TimerEvents.fps = value
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
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

      //
      // Taskbar tab
      //
      Item {
        id: taskbarTab

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
              Layout.rightMargin: -8
              Layout.alignment: Qt.AlignRight
              checked: Cpp_UI_TaskbarSettings.showTaskbarButtons
              palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
              onCheckedChanged: {
                if (checked !== Cpp_UI_TaskbarSettings.showTaskbarButtons)
                  Cpp_UI_TaskbarSettings.showTaskbarButtons = checked
              }
            }

            Label {
              Layout.fillWidth: true
              text: qsTr("Show Search Field")
              color: Cpp_ThemeManager.colors["text"]
            } Switch {
              Layout.rightMargin: -8
              Layout.alignment: Qt.AlignRight
              checked: Cpp_UI_TaskbarSettings.searchEnabled
              palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
              onCheckedChanged: {
                if (checked !== Cpp_UI_TaskbarSettings.searchEnabled)
                  Cpp_UI_TaskbarSettings.searchEnabled = checked
              }
            }

            Label {
              Layout.fillWidth: true
              text: qsTr("Auto-hide Taskbar")
              color: Cpp_ThemeManager.colors["text"]
            } Switch {
              Layout.rightMargin: -8
              Layout.alignment: Qt.AlignRight
              checked: Cpp_UI_TaskbarSettings.autohide
              palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
              onCheckedChanged: {
                if (checked !== Cpp_UI_TaskbarSettings.autohide)
                  Cpp_UI_TaskbarSettings.autohide = checked
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
                "settings": "qrc:/icons/taskbar/settings.svg",
                "console": "qrc:/icons/taskbar/console.svg",
                "notifications": "qrc:/icons/taskbar/notifications.svg",
                "clock": "qrc:/icons/taskbar/clock.svg",
                "stopwatch": "qrc:/icons/taskbar/stopwatch.svg",
                "pause": "qrc:/icons/taskbar/pause.svg",
                "file_transmission": "qrc:/icons/taskbar/file-transmission.svg",
                "ai_assistant": "qrc:/icons/taskbar/ai.svg"
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
                  Layout.preferredWidth: 16
                  Layout.preferredHeight: 16
                  sourceSize: Qt.size(16, 16)
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

      //
      // Console tab
      //
      Item {
        id: consoleTab

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
            Layout.fillWidth: true
            model: Cpp_Console_Handler.displayModes
            currentIndex: Cpp_Console_Handler.displayMode
            onActivated: (index) => {
              if (Cpp_Console_Handler.displayMode !== index)
                Cpp_Console_Handler.displayMode = index
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
          }

          Label {
            text: qsTr("Show Timestamps")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Console_Handler.showTimestamp
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Console_Handler.showTimestamp)
                Cpp_Console_Handler.showTimestamp = checked
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
            Layout.fillWidth: true
            model: Cpp_Console_Handler.lineEndings
            currentIndex: Cpp_Console_Handler.lineEnding
            onActivated: (index) => {
              if (Cpp_Console_Handler.lineEnding !== index)
                Cpp_Console_Handler.lineEnding = index
            }
          }

          Label {
            text: qsTr("Input Mode")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Combo {
            Layout.fillWidth: true
            model: Cpp_Console_Handler.dataModes
            currentIndex: Cpp_Console_Handler.dataMode
            onActivated: (index) => {
              if (Cpp_Console_Handler.dataMode !== index)
                Cpp_Console_Handler.dataMode = index
            }
          }

          Label {
            text: qsTr("Text Encoding")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Combo {
            Layout.fillWidth: true
            model: Cpp_Console_Handler.textEncodings
            currentIndex: Cpp_Console_Handler.encoding
            onActivated: (index) => {
              if (Cpp_Console_Handler.encoding !== index)
                Cpp_Console_Handler.encoding = index
            }
          }

          Label {
            text: qsTr("Checksum")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Combo {
            Layout.fillWidth: true
            model: Cpp_Console_Handler.checksumMethods
            currentIndex: Cpp_Console_Handler.checksumMethod
            onActivated: (index) => {
              if (Cpp_Console_Handler.checksumMethod !== index)
                Cpp_Console_Handler.checksumMethod = index
            }
          }

          Label {
            text: qsTr("Echo Sent Data")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Console_Handler.echo
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Console_Handler.echo)
                Cpp_Console_Handler.echo = checked
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
          }

          Label {
            text: qsTr("ANSI Colors")
            enabled: _vt100Emulation.checked
            color: Cpp_ThemeManager.colors["text"]
            opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : (enabled ? 1 : 0.5)
          } Switch {
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
          }

          Item { Layout.fillHeight: true }
          Item { Layout.fillHeight: true }
        }
      }

      //
      // Notifications tab (Pro only): empty Item in GPL builds so the
      // StackLayout indices still line up with the visible TabButtons.
      //
      Item {
        id: notificationsTab

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
    }

    //
    // Buttons
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.IconButton {
        text: qsTr("Reset")
        horizontalPadding: 8
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/refresh.svg"
        onClicked: {
          Cpp_ThemeManager.theme = 0
          Cpp_UI_Dashboard.plotTimeRange = 10
          Cpp_UI_Dashboard.points = 1000
          Cpp_Misc_TimerEvents.fps = 60
          Cpp_API_Server.enabled = false
          Cpp_API_Server.externalConnections = false
          Cpp_Misc_ModuleManager.automaticUpdates = true
          Cpp_Misc_ModuleManager.performanceMode = true
          Cpp_Misc_ModuleManager.inhibitIdleSleep = true
          Cpp_UI_Dashboard.autoHideToolbar = false
          Cpp_UI_TaskbarSettings.resetToDefaults()
          Cpp_Console_Handler.fontFamily = Cpp_Misc_CommonFonts.monoFont.family
          Cpp_Console_Handler.fontSize = Cpp_Misc_CommonFonts.monoFont.pointSize
          Cpp_Console_Handler.echo = false
          Cpp_Console_Handler.showTimestamp = false
          Cpp_Console_Handler.vt100Emulation = true
          Cpp_Console_Handler.ansiColors = true
          Cpp_Console_Handler.dataMode = 0
          Cpp_Console_Handler.displayMode = 0
          Cpp_Console_Handler.lineEnding = 1
          Cpp_Console_Handler.encoding = 0
          Cpp_Console_Handler.checksumMethod = 0
          Cpp_Misc_CommonFonts.widgetFontScale = 1.0
          Cpp_Misc_CommonFonts.widgetFontFamily = Cpp_Misc_CommonFonts.monoFont.family
        }
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("Close")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        text: qsTr("Apply")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/apply.svg"
      }
    }
  }
}
