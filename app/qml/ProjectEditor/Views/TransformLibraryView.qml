/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
import SerialStudio

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  focus: true
  title: root.lua ? qsTr("Lua Library") : qsTr("JavaScript Library")

  icon: Cpp_JSON_ProjectEditor.selectedIcon
  readonly property bool lua: Cpp_JSON_ProjectEditor.currentView === ProjectEditor.TransformLibraryView

  actionComponent: EditorNavActions {}

  //
  // Right-click context menu
  //
  CodeEditorMenu {
    id: contextMenu

    codeEditor: editor
  }

  //
  // User interface elements
  //
  Widgets.ThemedPage {
    anchors.fill: parent

    ColumnLayout {
      spacing: -1
      anchors.fill: parent
      anchors.topMargin: -16
      anchors.leftMargin: -10
      anchors.rightMargin: -10
      anchors.bottomMargin: -9

      //
      // Editor toolbar
      //
      CodeEditorToolbar {
        codeEditor: editor
        helpPage: "Dataset-Transforms"
        resetTooltip: qsTr("Reset to the commented starter")
        helpTooltip: qsTr("Open the dataset transforms documentation")
        validateTooltip: qsTr("Load the library in a sandbox and report the first error")
        importTooltip: root.lua ? qsTr("Import a Lua file") : qsTr("Import a JavaScript file")
      }

      Rectangle {
        z: 2
        implicitHeight: 1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      //
      // Code editor
      //
      TransformLibraryEditor {
        id: editor

        lua: root.lua

        Layout.fillWidth: true
        Layout.fillHeight: true

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.IBeamCursor
          propagateComposedEvents: true
          acceptedButtons: Qt.RightButton

          onClicked: (mouse) => {
                       if (mouse.button === Qt.RightButton) {
                         contextMenu.popup()
                         mouse.accepted = true
                       }
                     }
        }
      }
    }
  }
}
