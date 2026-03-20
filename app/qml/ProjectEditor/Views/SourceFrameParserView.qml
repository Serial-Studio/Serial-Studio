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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import SerialStudio as SerialStudio

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  focus: true
  icon: Cpp_JSON_ProjectEditor.selectedIcon
  title: Cpp_JSON_ProjectEditor.selectedText

  onVisibleChanged: {
    if (visible) {
      frameParser.sourceId = Cpp_JSON_ProjectEditor.selectedSourceId
      Qt.callLater(frameParser.forceActiveFocus)
    }
  }

  Connections {
    target: Cpp_JSON_ProjectEditor

    function onSelectedSourceFrameParserCodeChanged() {
      if (root.visible)
        frameParser.sourceId = Cpp_JSON_ProjectEditor.selectedSourceId
    }
  }

  //
  // Shortcuts
  //
  Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.selectAll()
    sequences: [StandardKey.SelectAll]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.reload(true)
    sequences: [StandardKey.New]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.import()
    sequences: [StandardKey.Open]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.undo()
    sequences: [StandardKey.Undo]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.redo()
    sequences: [StandardKey.Redo]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.cut()
    sequences: [StandardKey.Cut]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.copy()
    sequences: [StandardKey.Copy]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.paste()
    sequences: [StandardKey.Paste]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: Cpp_JSON_ProjectModel.saveJsonFile()
    sequences: [StandardKey.Save, StandardKey.SaveAs]
  }

  //
  // Right-click context menu
  //
  Menu {
    id: contextMenu

    onClosed: frameParser.forceActiveFocus()

    MenuItem {
      text: qsTr("Undo")
      opacity: enabled ? 1 : 0.5
      onClicked: frameParser.undo()
      enabled: frameParser.isModified
    }

    MenuItem {
      text: qsTr("Redo")
      opacity: enabled ? 1 : 0.5
      onClicked: frameParser.redo()
      enabled: frameParser.isModified
    }

    MenuSeparator {}

    MenuItem {
      text: qsTr("Cut")
      onClicked: frameParser.cut()
    }

    MenuItem {
      text: qsTr("Copy")
      onClicked: frameParser.copy()
    }

    MenuItem {
      text: qsTr("Paste")
      onClicked: frameParser.paste()
    }

    MenuSeparator {}

    MenuItem {
      text: qsTr("Select All")
      opacity: enabled ? 1 : 0.5
      onTriggered: frameParser.selectAll()
      enabled: frameParser.text.length > 0
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
      spacing: 0
      anchors.margins: 0
      anchors.fill: parent
      anchors.topMargin: -16
      anchors.leftMargin: -10
      anchors.rightMargin: -10
      anchors.bottomMargin: -9

      Rectangle {
        Layout.fillWidth: true
        Layout.maximumHeight: Layout.minimumHeight
        color: Cpp_ThemeManager.colors["groupbox_background"]
        Layout.minimumHeight: toolbarLayout.implicitHeight + 12

        RowLayout {
          id: toolbarLayout

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
            Layout.alignment: Qt.AlignVCenter
            onClicked: frameParser.reload(true)
            icon.source: "qrc:/rcc/icons/code-editor/reload.svg"
            ToolTip.text: qsTr("Reset to the default parsing script")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Open")
            toolbarButton: false
            onClicked: frameParser.import()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/open.svg"
            ToolTip.text: qsTr("Import a JavaScript file for data parsing")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Undo")
            toolbarButton: false
            onClicked: frameParser.undo()
            Layout.alignment: Qt.AlignVCenter
            enabled: frameParser.undoAvailable
            ToolTip.text: qsTr("Undo the last code edit")
            icon.source: "qrc:/rcc/icons/code-editor/undo.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Redo")
            toolbarButton: false
            onClicked: frameParser.redo()
            Layout.alignment: Qt.AlignVCenter
            enabled: frameParser.redoAvailable
            icon.source: "qrc:/rcc/icons/code-editor/redo.svg"
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
            onClicked: frameParser.cut()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/cut.svg"
            ToolTip.text: qsTr("Cut selected code to clipboard")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Copy")
            toolbarButton: false
            onClicked: frameParser.copy()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/copy.svg"
            ToolTip.text: qsTr("Copy selected code to clipboard")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Paste")
            toolbarButton: false
            onClicked: frameParser.paste()
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Paste code from clipboard")
            icon.source: "qrc:/rcc/icons/code-editor/paste.svg"
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
            onClicked: app.showHelpCenter("javascript-api")
            icon.source: "qrc:/rcc/icons/code-editor/help.svg"
            ToolTip.text: qsTr("Open help documentation for JavaScript data parsing")
          }

          Item {
            Layout.fillWidth: true
          }
        }
      }

      Rectangle {
        implicitHeight: 1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      //
      // Template selector + evaluate row
      //
      Rectangle {
        implicitHeight: 32
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_background"]
        Layout.minimumHeight: templateLayout.implicitHeight + 12

        RowLayout {
          id: templateLayout

          spacing: 4

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Button {
            icon.width: 18
            icon.height: 18
            horizontalPadding: 12
            text: qsTr("Select Template...")
            Layout.alignment: Qt.AlignVCenter
            onClicked: frameParser.selectTemplate()
            icon.source: "qrc:/rcc/icons/buttons/code.svg"
            icon.color: Cpp_ThemeManager.colors["button_text"]
          }

          Item {
            Layout.fillWidth: true
          }

          Button {
            icon.width: 18
            icon.height: 18
            horizontalPadding: 12
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Test With Sample Data")
            onClicked: frameParser.testWithSampleData()
            icon.source: "qrc:/rcc/icons/buttons/test.svg"
            icon.color: Cpp_ThemeManager.colors["button_text"]
          }

          Button {
            icon.width: 18
            icon.height: 18
            horizontalPadding: 12
            text: qsTr("Evaluate")
            onClicked: frameParser.evaluate()
            Layout.alignment: Qt.AlignVCenter
            icon.color: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/buttons/media-play.svg"
          }
        }
      }

      Rectangle {
        z: 2
        implicitHeight: 1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      SerialStudio.JsCodeEditor {
        id: frameParser

        Layout.topMargin: -1
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
