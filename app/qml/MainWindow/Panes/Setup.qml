/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

import QtCore
import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "SetupPanes" as SetupPanes
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  title: qsTr("Device Setup")
  icon: "qrc:/rcc/icons/panes/setup.svg"
  implicitHeight: layout.implicitHeight + 32

  //
  // Custom properties
  //
  property int setupMargin: 0
  property int userPaneWidth: 340
  readonly property int kMinPaneWidth: 280
  readonly property int maxItemWidth: layout.width - 8
  readonly property int displayedWidth: Math.max(kMinPaneWidth, userPaneWidth)
  readonly property bool dataExportAllowed:
    Cpp_AppState.operationMode !== SerialStudio.ConsoleOnly

  //
  // Displays the setup panel
  //
  function show() {
    setupMargin = 0
  }

  //
  // Hides the setup panel
  //
  function hide() {
    setupMargin = -1 * displayedWidth
  }

  //
  // Animations
  //
  visible: setupMargin > -1 * displayedWidth
  Behavior on setupMargin {NumberAnimation{}}

  //
  // Save settings
  //
  Settings {
    category: "SetupPanel"
    property alias csvExport: csvLogging.checked
    property alias paneWidth: root.userPaneWidth
  }

  //
  // API Server status indicator
  //
  actionComponent: Component {
    Item {
      opacity: Cpp_API_Server.enabled ?
                 (Cpp_API_Server.clientCount > 0 ? 1 : 0.5) :
                 0.0
      implicitWidth: label.implicitWidth
      implicitHeight: label.implicitHeight
      Behavior on opacity { NumberAnimation { duration: 200 } }

      MultiEffect {
        id: glow

        source: label
        shadowBlur: 2.0
        shadowEnabled: true
        anchors.fill: label
        shadowVerticalOffset: 0
        shadowHorizontalOffset: 0
        visible: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0
        shadowColor: Cpp_API_Server.enabled ? Cpp_ThemeManager.colors["highlight"] :
                                              Cpp_ThemeManager.colors["pane_caption_border"]

        SequentialAnimation on opacity {
          loops: Animation.Infinite
          running: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0

          NumberAnimation {
            to: 1.00
            from: 0.4
            duration: 800
            easing.type: Easing.InOutSine
          }
          NumberAnimation {
            to: 0.4
            from: 1.00
            duration: 800
            easing.type: Easing.InOutSine
          }
        }

        SequentialAnimation on brightness {
          loops: Animation.Infinite
          running: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0

          NumberAnimation {
            to: 0.6
            from: 0.15
            duration: 800
            easing.type: Easing.InOutSine
          }
          NumberAnimation {
            to: 0.15
            from: 0.6
            duration: 800
            easing.type: Easing.InOutSine
          }
        }
      }

      Label {
        id: label

        visible: opacity > 0
        text: Cpp_API_Server.enabled ?
                (Cpp_API_Server.clientCount > 0 ? qsTr("API Server Active (%1)").arg(Cpp_API_Server.clientCount) :
                                                  qsTr("API Server Ready")) :
                qsTr("API Server Off")
        font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
        color: Cpp_ThemeManager.colors["pane_caption_foreground"]
      }
    }
  }

  //
  // Use page item to set application palette
  //
  Page {
    implicitWidth: 0
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

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

    //
    // Control arrangement
    //
    ColumnLayout {
      id: layout

      anchors {
        fill: parent
        topMargin: 10
        leftMargin: 9
        rightMargin: 9
        bottomMargin: 9
      }

      spacing: 4

      //
      // Comm mode selector
      //
      Label {
        text: qsTr("Frame Parsing") + ":"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      } RadioButton {
        Layout.leftMargin: -6
        enabled: app.ioEnabled
        Layout.maximumHeight: 18
        opacity: enabled ? 1 : 0.5
        Layout.maximumWidth: root.maxItemWidth
        text: qsTr("Console Only (No Parsing)")
        checked: Cpp_AppState.operationMode === SerialStudio.ConsoleOnly
        onCheckedChanged: {
          const shouldChange = Cpp_AppState.operationMode !== SerialStudio.ConsoleOnly
          if (checked && shouldChange)
            Cpp_AppState.operationMode = SerialStudio.ConsoleOnly
        }
      } RadioButton {
        Layout.leftMargin: -6
        enabled: app.ioEnabled
        Layout.maximumHeight: 18
        opacity: enabled ? 1 : 0.5
        Layout.maximumWidth: root.maxItemWidth
        text: qsTr("Quick Plot (Comma Separated Values)")
        checked: Cpp_AppState.operationMode === SerialStudio.QuickPlot
        onCheckedChanged: {
          const shouldChange = Cpp_AppState.operationMode !== SerialStudio.QuickPlot
          if (checked && shouldChange)
            Cpp_AppState.operationMode = SerialStudio.QuickPlot
        }
      } RadioButton {
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        opacity: enabled ? 1 : 0.5
        text: qsTr("Parse via Project File")
        Layout.maximumWidth: root.maxItemWidth
        checked: Cpp_AppState.operationMode === SerialStudio.ProjectFile
        enabled: !Cpp_IO_Manager.isConnected
                 && !Cpp_CSV_Player.isOpen
                 && !Cpp_MDF4_Player.isOpen
                 && (!Cpp_CommercialBuild || !Cpp_Sessions_Player.isOpen)
        onCheckedChanged: {
          const shouldChange = Cpp_AppState.operationMode !== SerialStudio.ProjectFile
          if (checked && shouldChange)
            Cpp_AppState.operationMode = SerialStudio.ProjectFile
        }
      }

      //
      // Map file selector button
      //
      Button {
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        Layout.maximumWidth: root.maxItemWidth
        onClicked: Cpp_JSON_ProjectModel.openJsonFile()
        text: (Cpp_AppState.projectFileName.length ?
                 qsTr("Change Project File (%1)").arg(Cpp_AppState.projectFileName) :
                 qsTr("Select Project File") + "...")
        enabled: Cpp_AppState.operationMode === SerialStudio.ProjectFile
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Data export switches
      //
      Label {
        text: qsTr("Data Export") + ":"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      //
      // CSV generator
      //
      CheckBox {
        id: csvLogging
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        opacity: enabled ? 1 : 0.5
        text: qsTr("Create CSV File")
        Layout.alignment: Qt.AlignLeft
        enabled: root.dataExportAllowed
        Layout.maximumWidth: root.maxItemWidth
        checked: root.dataExportAllowed && Cpp_CSV_Export.exportEnabled

        onCheckedChanged:  {
          if (Cpp_CSV_Export.exportEnabled !== checked)
            Cpp_CSV_Export.exportEnabled = checked
        }
      }

      //
      // MDF4 generator
      //
      CheckBox {
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        opacity: enabled ? 1 : 0.5
        text: qsTr("Create MDF4 File")
        Layout.alignment: Qt.AlignLeft
        enabled: root.dataExportAllowed
        Layout.maximumWidth: root.maxItemWidth
        checked: root.dataExportAllowed && Cpp_MDF4_Export.exportEnabled

        onCheckedChanged: {
          if (Cpp_MDF4_Export.exportEnabled !== checked)
            Cpp_MDF4_Export.exportEnabled = checked
        }
      }

      //
      // Session log (Pro)
      //
      CheckBox {
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        opacity: enabled ? 1 : 0.5
        visible: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        text: qsTr("Create Session Log")
        enabled: root.dataExportAllowed
        Layout.maximumWidth: root.maxItemWidth
        checked: Cpp_CommercialBuild
                 && root.dataExportAllowed
                 && Cpp_Sessions_Export.exportEnabled

        onCheckedChanged: {
          if (!Cpp_CommercialBuild)
            return

          if (Cpp_Sessions_Export.exportEnabled !== checked)
            Cpp_Sessions_Export.exportEnabled = checked
        }
      }

      //
      // Console data export
      //
      CheckBox {
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        Layout.alignment: Qt.AlignLeft
        text: qsTr("Export Console Data")
        Layout.maximumWidth: root.maxItemWidth
        checked: Cpp_Console_Export.exportEnabled

        onCheckedChanged:  {
          if (Cpp_Console_Export.exportEnabled !== checked)
            Cpp_Console_Export.exportEnabled = checked
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Multi-source redirect or driver selection
      //
      StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        currentIndex: {
          const isProjectFile = Cpp_AppState.operationMode === SerialStudio.ProjectFile
          const multiSource   = Cpp_JSON_ProjectModel.sourceCount > 1
          return (isProjectFile && multiSource) ? 1 : 0
        }

        //
        // Normal hardware panel with drivers
        //
        ColumnLayout {
          spacing: 4

          Label {
            text: qsTr("Device Setup") + ":"
            font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          ComboBox {
            Layout.fillWidth: true
            enabled: app.ioEnabled
            opacity: enabled ? 1 : 0.5
            model: Cpp_IO_Manager.availableBuses
            currentIndex: Cpp_IO_Manager.busType
            displayText: qsTr("I/O Interface: %1").arg(currentText)

            onCurrentIndexChanged: {
              if (Cpp_IO_Manager.busType !== currentIndex)
                Cpp_IO_Manager.busType = currentIndex
            }
          }

          SetupPanes.Hardware {
            id: hardware
            Layout.fillWidth: true
            Layout.fillHeight: true
          }
        }

        //
        // Multi-source redirect panel
        //
        ColumnLayout {
          spacing: 0

          Item {
            implicitHeight: 8
          }

          Item {
            Layout.fillWidth: true
            implicitHeight: _panelLayout.implicitHeight + 24

            Rectangle {
              radius: 2
              border.width: 1
              anchors.fill: parent
              border.color: Cpp_ThemeManager.colors["groupbox_border"]
              color: Cpp_ThemeManager.colors["groupbox_background"]
            }

            ColumnLayout {
              id: _panelLayout

              spacing: 0
              anchors {
                margins: 12
                fill: parent
              }

              Image {
                fillMode: Image.PreserveAspectFit
                Layout.alignment: Qt.AlignHCenter
                source: "qrc:/rcc/images/multi-device.svg"
              }

              Item {
                implicitHeight: 12
              }

              Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(1.15, true)
                text: qsTr("Multi-Device Project")
              }

              Item {
                implicitHeight: 6
              }

              Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
                text: qsTr("This project streams data from %1 independent devices.").arg(Cpp_JSON_ProjectModel.sourceCount)
              }

              Item {
                implicitHeight: 4
              }

              Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: Cpp_ThemeManager.colors["pane_section_label"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                text: qsTr("Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.")
              }

              Item {
                implicitHeight: 16
              }

              Rectangle {
                implicitHeight: 1
                Layout.fillWidth: true
                color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              Item {
                implicitHeight: 12
              }

              Button {
                icon.width: 18
                icon.height: 18
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Open Project Editor")
                onClicked: app.showProjectEditor()
                icon.source: "qrc:/rcc/icons/buttons/wrench.svg"
                icon.color: Cpp_ThemeManager.colors["button_text"]
              }
            }
          }

          Item {
            Layout.fillHeight: true
          }
        }
      }
    }
  }
}
