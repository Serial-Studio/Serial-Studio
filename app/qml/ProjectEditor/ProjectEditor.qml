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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "Views" as Views
import "Sections" as Sections
import "../Widgets" as Widgets

Widgets.SmartWindow {
  id: root

  category: "ProjectEditor"
  minimumWidth: layout.implicitWidth + 32
  minimumHeight: layout.implicitHeight + 32
  property alias preferredWidth: layout.implicitWidth
  property alias preferredHeight: layout.implicitHeight
  title: Cpp_JSON_ProjectModel.title + (Cpp_JSON_ProjectModel.modified ? " (" + qsTr("modified") + ")" : "")

  //
  // Ask user to save changes when closing the dialog
  //
  onClosing: (close) => close.accepted = Cpp_JSON_ProjectModel.askSave()

  //
  // Ensure that current JSON file is shown
  //
  onVisibleChanged: {
    if (visible) {
      Cpp_NativeWindow.addWindow(root)
      Cpp_JSON_ProjectModel.openJsonFile(Cpp_AppState.projectFilePath)
    }

    else
      Cpp_NativeWindow.removeWindow(root)
  }

  //
  // Load project file on start
  //
  Component.onCompleted: Cpp_JSON_ProjectModel.openJsonFile(Cpp_AppState.projectFilePath)

  //
  // Dummy string to increase width of buttons
  //
  readonly property string _btSpacer: "  "

  //
  // Shortcuts
  //
  Shortcut {
    sequences: [StandardKey.Open]
    onActivated: Cpp_JSON_ProjectModel.openJsonFile()
  } Shortcut {
    sequences: [StandardKey.New]
    onActivated: Cpp_JSON_ProjectModel.newJsonFile()
  } Shortcut {
    sequences: [StandardKey.Save]
    onActivated: Cpp_JSON_ProjectModel.saveJsonFile()
  } Shortcut {
    sequences: [StandardKey.Quit]
    onActivated: app.quitApplication()
  }

  //
  // Use page item to set application palette
  //
  Page {
    clip: true
    anchors.fill: parent
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
    // Project mode required placeholder — the Project Editor edits the loaded
    // project file, which has no meaning while the dashboard is running
    // QuickPlot or ConsoleOnly. Show a hint instead of half-working UI.
    //
    ColumnLayout {
      spacing: 16
      anchors.centerIn: parent
      visible: Cpp_AppState.operationMode !== SerialStudio.ProjectFile

      Image {
        sourceSize: Qt.size(144, 144)
        Layout.alignment: Qt.AlignHCenter
        source: "qrc:/rcc/images/project-mode-required.svg"
      }

      Item {
        implicitHeight: 4
      }

      Label {
        wrapMode: Label.WordWrap
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: root.width - 96
        horizontalAlignment: Label.AlignHCenter
        text: qsTr("Project Editor Unavailable")
        font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
      }

      Label {
        opacity: 0.8
        wrapMode: Label.WordWrap
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: root.width - 96
        horizontalAlignment: Label.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(1.2, false)
        text: qsTr("The Project Editor is only available in Project File mode. Switch modes to load and edit a project.")
      }

      Item {
        implicitHeight: 8
      }

      RowLayout {
        spacing: 8
        Layout.alignment: Qt.AlignHCenter

        Button {
          highlighted: true
          text: qsTr("Switch to Project Mode") + root._btSpacer
          icon.source: "qrc:/rcc/icons/project-editor/treeview/project-setup.svg"
          onClicked: {
            Cpp_AppState.operationMode = SerialStudio.ProjectFile
          }
        }

        Button {
          text: qsTr("Close") + root._btSpacer
          onClicked: root.close()
        }
      }
    }

    ColumnLayout {
      id: layout

      spacing: 0
      anchors.fill: parent
      visible: Cpp_AppState.operationMode === SerialStudio.ProjectFile

      //
      // Toolbar
      //
      Sections.ProjectToolbar {
        z: 2
        Layout.fillWidth: true
        Layout.minimumWidth: 860
      }

      //
      // Main Layout
      //
      Widgets.PaneSplitter {
        id: splitter

        minLeftWidth: 256
        minRightWidth: 400
        Layout.topMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 480
        settingsKey: "ProjectEditor"

        leftPanel: Component {
          Sections.ProjectStructure {
            id: projectStructure
          }
        }

        rightPanel: Component {
          Item {
            //
            // Action view
            //
            Views.ActionView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.ActionView
            }

            //
            // Project setup
            //
            Views.ProjectView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.ProjectView
            }

            //
            // Group view
            //
            Views.GroupView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.GroupView
            }

            //
            // Dataset view
            //
            Views.DatasetView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.DatasetView
            }

            //
            // Source view
            //
            Views.SourceView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.SourceView
            }

            //
            // Per-source frame parser
            //
            Views.SourceFrameParserView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.SourceFrameParserView
            }

            //
            // Output widget view
            //
            Views.OutputWidgetView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.OutputWidgetView
            }

            //
            // Data tables overview
            //
            Views.DataTablesView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.DataTablesView
            }

            //
            // System __datasets__ table viewer
            //
            Views.SystemDatasetsView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.SystemDatasetsView
            }

            //
            // User-defined shared memory table editor
            //
            Views.UserTableView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.UserTableView
            }

            //
            // Workspaces overview
            //
            Views.WorkspacesView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.WorkspacesView
            }

            //
            // Single workspace editor
            //
            Views.WorkspaceView {
              anchors.fill: parent
              visible: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.WorkspaceView
            }
          }
        }
      }
    }
  }
}
