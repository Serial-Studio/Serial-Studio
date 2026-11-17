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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "SetupPanes" as SetupPanes
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  title: qsTr("Device Setup")
  implicitHeight: layout.implicitHeight + 32
  icon: Cpp_Misc_IconRegistry.icon("panes", "setup", 16)

  //
  // Custom properties
  //
  property int userPaneWidth: 340
  readonly property int kMinPaneWidth: 280
  readonly property int maxItemWidth: layout.width - 8
  readonly property int displayedWidth: Math.max(kMinPaneWidth, userPaneWidth)
  readonly property bool dataExportAllowed:
    Cpp_AppState.operationMode !== SerialStudio.ConsoleOnly

  //
  // Save settings
  //
  Settings {
    category: "SetupPanel"
    property alias paneWidth: root.userPaneWidth
  }

  //
  // API server / remote attach status indicator
  //
  actionComponent: Component {
    Widgets.ApiStatusIndicator {}
  }

  //
  // Use page item to set application palette
  //
  Widgets.ThemedPage {
    implicitWidth: 0
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

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
      // Operation mode selector
      //
      Label {
        text: qsTr("Operation Mode") + ":"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      } Widgets.Radio {
        objectName: "setup.operationMode.consoleOnly"

        ToolTip.delay: 500
        Layout.leftMargin: -6
        enabled: app.ioEnabled
        Layout.maximumHeight: 18
        ToolTip.visible: hovered
        opacity: enabled ? 1 : 0.5
        Layout.maximumWidth: root.maxItemWidth
        text: qsTr("Console Only (No Parsing)")
        checked: Cpp_AppState.operationMode === SerialStudio.ConsoleOnly
        ToolTip.text: qsTr("Shows incoming data as text or hex. Nothing is parsed or plotted.")
        onCheckedChanged: {
          const shouldChange = Cpp_AppState.operationMode !== SerialStudio.ConsoleOnly
          if (checked && shouldChange)
            Cpp_AppState.operationMode = SerialStudio.ConsoleOnly
        }
      } Widgets.Radio {
        objectName: "setup.operationMode.quickPlot"

        Layout.leftMargin: -6
        enabled: app.ioEnabled
        Layout.maximumHeight: 18
        text: qsTr("Quick Plot (Auto-Detected Format)")
        ToolTip.delay: 500
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Plots every line the device sends. Values separated by commas, tabs, "
                           + "or spaces, name and value pairs, units, and one-line JSON are "
                           + "detected automatically.")
        opacity: enabled ? 1 : 0.5
        Layout.maximumWidth: root.maxItemWidth
        checked: Cpp_AppState.operationMode === SerialStudio.QuickPlot
        onCheckedChanged: {
          const shouldChange = Cpp_AppState.operationMode !== SerialStudio.QuickPlot
          if (checked && shouldChange)
            Cpp_AppState.operationMode = SerialStudio.QuickPlot
        }
      } Widgets.Radio {
        objectName: "setup.operationMode.projectFile"

        ToolTip.delay: 500
        Layout.leftMargin: -6
        enabled: app.ioEnabled
        Layout.maximumHeight: 18
        ToolTip.visible: hovered
        opacity: enabled ? 1 : 0.5
        text: qsTr("Parse via Project File")
        Layout.maximumWidth: root.maxItemWidth
        checked: Cpp_AppState.operationMode === SerialStudio.ProjectFile
        ToolTip.text: qsTr("Builds the dashboard from a project made in the Project Editor.")
        onCheckedChanged: {
          const shouldChange = Cpp_AppState.operationMode !== SerialStudio.ProjectFile
          if (checked && shouldChange)
            Cpp_AppState.operationMode = SerialStudio.ProjectFile
        }
      }

      //
      // Map file selector button
      //
      Widgets.PushButton {
        objectName: "setup.openProject"

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
      // Device declaration offer
      //
      Widgets.PushButton {
        objectName: "setup.declaration.adopt"

        Layout.fillWidth: true
        text: qsTr("Use Device Dashboard")
        Layout.maximumWidth: root.maxItemWidth
        onClicked: Cpp_JSON_DeclarationWatch.adopt()
        visible: Cpp_AppState.operationMode === SerialStudio.ProjectFile
                 && Cpp_JSON_DeclarationWatch.pendingDifference
      } Widgets.PushButton {
        objectName: "setup.declaration.keep"

        Layout.fillWidth: true
        text: qsTr("Keep Project")
        Layout.maximumWidth: root.maxItemWidth
        onClicked: Cpp_JSON_DeclarationWatch.keep()
        visible: Cpp_AppState.operationMode === SerialStudio.ProjectFile
                 && Cpp_JSON_DeclarationWatch.pendingDifference
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
      // Data export grid
      //
      GridLayout {
        columns: 1
        rowSpacing: 4
        columnSpacing: 4
        Layout.fillWidth: true
        Layout.maximumWidth: root.maxItemWidth

        Widgets.Toggle {
          id: csvLogging

          objectName: "setup.csv.exportEnabled"

          Layout.leftMargin: -6
          Layout.maximumHeight: 18
          palette.accent: "#1f6b3a"
          opacity: enabled ? 1 : 0.5
          palette.highlight: "#2e9c52"
          text: qsTr("CSV Spreadsheet")
          Layout.alignment: Qt.AlignLeft
          enabled: root.dataExportAllowed
          checked: root.dataExportAllowed && Cpp_CSV_Export.exportEnabled

          onCheckedChanged:  {
            if (Cpp_CSV_Export.exportEnabled !== checked)
              Cpp_CSV_Export.exportEnabled = checked
          }
        }

        Widgets.Toggle {
          objectName: "setup.sessions.exportEnabled"

          Layout.leftMargin: -6
          Layout.maximumHeight: 18
          palette.accent: "#1f4f8a"
          opacity: enabled ? 1 : 0.5
          visible: Cpp_CommercialBuild
          palette.highlight: "#2e7dd1"
          Layout.alignment: Qt.AlignLeft
          enabled: root.dataExportAllowed
          text: qsTr("Historian Database")
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

        Widgets.Toggle {
          objectName: "setup.mdf4.exportEnabled"

          Layout.leftMargin: -6
          Layout.maximumHeight: 18
          palette.accent: "#8a1a1a"
          opacity: enabled ? 1 : 0.5
          text: qsTr("MDF4 Recording")
          palette.highlight: "#c62828"
          Layout.alignment: Qt.AlignLeft
          enabled: root.dataExportAllowed
          checked: root.dataExportAllowed && Cpp_MDF4_Export.exportEnabled

          onCheckedChanged: {
            if (Cpp_MDF4_Export.exportEnabled !== checked)
              Cpp_MDF4_Export.exportEnabled = checked
          }
        }

        Widgets.Toggle {
          objectName: "setup.console.exportEnabled"

          Layout.leftMargin: -6
          Layout.maximumHeight: 18
          palette.accent: "#4a5663"
          text: qsTr("Console Log")
          opacity: enabled ? 1 : 0.5
          palette.highlight: "#7d8a98"
          Layout.alignment: Qt.AlignLeft
          checked: Cpp_Console_Export.exportEnabled

          onCheckedChanged:  {
            if (Cpp_Console_Export.exportEnabled !== checked)
              Cpp_Console_Export.exportEnabled = checked
          }
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

          Widgets.Combo {
            objectName: "setup.busType"

            Layout.fillWidth: true
            enabled: app.ioEnabled
            opacity: enabled ? 1 : 0.5
            model: Cpp_IO_Manager.availableBuses
            currentIndex: Cpp_IO_Manager.busType
            displayText: qsTr("Connection: %1").arg(currentText)

            onActivated: (index) => {
              if (Cpp_IO_Manager.busType !== index)
                Cpp_IO_Manager.busType = index
            }
          }

          SetupPanes.Hardware {
            id: hardware

            Layout.fillWidth: true
            Layout.fillHeight: true
          }
        }

        //
        // Multi-source redirect panel (loaded only when needed)
        //
        Loader {
          Layout.fillWidth: true
          Layout.fillHeight: true
          active: Cpp_AppState.operationMode === SerialStudio.ProjectFile
                  && Cpp_JSON_ProjectModel.sourceCount > 1

          sourceComponent: ColumnLayout {
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
                  source: Cpp_Misc_IconRegistry.icon("illustrations", "multi-device", 48)
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
                  text: qsTr("Each device has its own connection settings. Configure them in the Project Editor under Devices.")
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

                Widgets.IconButton {
                  objectName: "setup.openProjectEditor"

                  Layout.alignment: Qt.AlignHCenter
                  text: qsTr("Open Project Editor")
                  onClicked: app.showProjectEditor()
                  icon.source: "qrc:/icons/buttons/wrench.svg"
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
}
