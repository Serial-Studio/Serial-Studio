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
  title: qsTr("Control Loop")
  icon: Cpp_JSON_ProjectEditor.selectedIcon

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
        helpPage: "control-script"
        importTooltip: qsTr("Import a control loop file")
        resetTooltip: qsTr("Reset to the default control loop")
        helpTooltip: qsTr("Open the control loop documentation")
        validateTooltip: qsTr("Verify that the script compiles correctly")
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
      ControlScriptEditor {
        id: editor

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

      //
      // Error banner
      //
      Rectangle {
        z: 2
        implicitHeight: 1
        Layout.fillWidth: true
        visible: errorBar.visible
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      Rectangle {
        id: errorBar

        Layout.fillWidth: true
        Layout.minimumHeight: 28
        Layout.maximumHeight: 28
        visible: errorLabel.text.length > 0
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Connections {
          target: Cpp_ControlScript
          function onError(message) { errorLabel.text = message }
          function onRunningChanged() {
            if (Cpp_ControlScript.running)
              errorLabel.text = ""
          }
        }

        Label {
          id: errorLabel

          elide: Label.ElideRight
          color: Cpp_ThemeManager.colors["alarm"]
          font: Cpp_Misc_CommonFonts.uiFont

          anchors {
            leftMargin: 12
            rightMargin: 12
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }
        }
      }
    }
  }
}
