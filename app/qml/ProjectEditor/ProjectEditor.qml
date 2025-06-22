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

  //
  // Window options
  //
  category: "ProjectEditor"
  minimumWidth: layout.implicitWidth + 32
  minimumHeight: layout.implicitHeight + 32
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
      Cpp_JSON_ProjectModel.enableProjectMode()
      Cpp_JSON_ProjectModel.openJsonFile(Cpp_JSON_FrameBuilder.jsonMapFilepath)
    }

    else
      Cpp_NativeWindow.removeWindow(root)
  }

  //
  // Load project file on start
  //
  Component.onCompleted: Cpp_JSON_ProjectModel.openJsonFile(Cpp_JSON_FrameBuilder.jsonMapFilepath)

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
  }

  //
  // Use page item to set application palette
  //
  Page {
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

    ColumnLayout {
      id: layout
      spacing: 0
      anchors.fill: parent

      //
      // Toolbar
      //
      Sections.ProjectToolbar {
        z: 2
        Layout.fillWidth: true
      }

      //
      // Main Layout
      //
      RowLayout {
        spacing: 0
        Layout.topMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 480

        //
        // Project structure
        //
        Sections.ProjectStructure {
          id: projectStructure
          Layout.fillHeight: true
          Layout.minimumWidth: 256
          Layout.maximumWidth: 256
        }

        //
        // Panel border
        //
        Rectangle {
          z: 2
          implicitWidth: 1
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["setup_border"]

          Rectangle {
            width: 1
            height: 32
            anchors.top: parent.top
            anchors.left: parent.left
            color: Cpp_ThemeManager.colors["pane_caption_border"]
          }
        }

        //
        // Action view
        //
        Views.ActionView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          visible: Cpp_JSON_ProjectModel.currentView === ProjectModel.ActionView
        }

        //
        // Project setup
        //
        Views.ProjectView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          visible: Cpp_JSON_ProjectModel.currentView === ProjectModel.ProjectView
        }

        //
        // Group view
        //
        Views.GroupView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          visible: Cpp_JSON_ProjectModel.currentView === ProjectModel.GroupView
        }

        //
        // Dataset view
        //
        Views.DatasetView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          visible: Cpp_JSON_ProjectModel.currentView === ProjectModel.DatasetView
        }

        //
        // Frame parser function
        //
        Views.FrameParserView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          visible: Cpp_JSON_ProjectModel.currentView === ProjectModel.FrameParserView
        }
      }
    }
  }
}
