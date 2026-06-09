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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import SerialStudio

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  focus: true
  title: qsTr("Control Script")
  icon: Cpp_JSON_ProjectEditor.selectedIcon

  //
  // Right-click context menu
  //
  Menu {
    id: contextMenu

    onClosed: editor.forceActiveFocus()

    MenuItem {
      text: qsTr("Undo")
      opacity: enabled ? 1 : 0.5
      onClicked: editor.undo()
      enabled: editor.undoAvailable
    }

    MenuItem {
      text: qsTr("Redo")
      opacity: enabled ? 1 : 0.5
      onClicked: editor.redo()
      enabled: editor.redoAvailable
    }

    MenuSeparator {}

    MenuItem { text: qsTr("Cut");   onClicked: editor.cut() }
    MenuItem { text: qsTr("Copy");  onClicked: editor.copy() }
    MenuItem { text: qsTr("Paste"); onClicked: editor.paste() }

    MenuSeparator {}

    MenuItem {
      text: qsTr("Select All")
      opacity: enabled ? 1 : 0.5
      onTriggered: editor.selectAll()
      enabled: editor.text.length > 0
    }

    MenuSeparator {}

    MenuItem {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Format Document")
      onTriggered: editor.formatDocument()
      enabled: editor.text.length > 0
    }

    MenuItem {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Format Selection")
      onTriggered: editor.formatSelection()
      enabled: editor.text.length > 0
    }
  }

  //
  // User interface elements
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
      spacing: -1
      anchors.fill: parent
      anchors.topMargin: -16
      anchors.leftMargin: -10
      anchors.rightMargin: -10
      anchors.bottomMargin: -9

      //
      // Editor toolbar
      //
      Rectangle {
        Layout.fillWidth: true
        Layout.maximumHeight: Layout.minimumHeight
        color: Cpp_ThemeManager.colors["groupbox_background"]
        Layout.minimumHeight: editorToolbar.implicitHeight + 12

        RowLayout {
          id: editorToolbar

          spacing: 4

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Reset")
            toolbarButton: false
            onClicked: editor.reload()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code-editor/reload.svg"
            ToolTip.text: qsTr("Reset to the default control script")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Open")
            toolbarButton: false
            onClicked: editor.import()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code-editor/open.svg"
            ToolTip.text: qsTr("Import a control script file")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Undo")
            toolbarButton: false
            onClicked: editor.undo()
            enabled: editor.undoAvailable
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Undo the last code edit")
            icon.source: "qrc:/icons/code-editor/undo.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Redo")
            toolbarButton: false
            onClicked: editor.redo()
            enabled: editor.redoAvailable
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code-editor/redo.svg"
            ToolTip.text: qsTr("Redo the previously undone edit")
          }

          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            Layout.maximumHeight: 48
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Cut")
            toolbarButton: false
            onClicked: editor.cut()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code-editor/cut.svg"
            ToolTip.text: qsTr("Cut selected code to clipboard")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Copy")
            toolbarButton: false
            onClicked: editor.copy()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code-editor/copy.svg"
            ToolTip.text: qsTr("Copy selected code to clipboard")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Paste")
            toolbarButton: false
            onClicked: editor.paste()
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Paste code from clipboard")
            icon.source: "qrc:/icons/code-editor/paste.svg"
          }

          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            Layout.maximumHeight: 48
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Help")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code-editor/help.svg"
            onClicked: app.showHelpCenter("Control-Script")
            ToolTip.text: qsTr("Open the control script documentation")
          }

          Item {
            Layout.fillWidth: true
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Validate")
            onClicked: editor.evaluate()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code-editor/test.svg"
            ToolTip.text: qsTr("Verify that the script compiles correctly")
          }
        }
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
