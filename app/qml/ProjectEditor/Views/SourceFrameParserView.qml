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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import SerialStudio

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  focus: true
  icon: Cpp_JSON_ProjectEditor.selectedIcon
  title: Cpp_JSON_ProjectEditor.selectedText

  actionComponent: EditorNavActions {}

  //
  // Native mode swaps the code editor for the template configuration pane
  //
  readonly property bool nativeMode: frameParser.language === SerialStudio.Native

  onVisibleChanged: {
    if (visible) {
      frameParser.sourceId = Cpp_JSON_ProjectEditor.selectedSourceId
      if (!root.nativeMode)
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
  // Shortcuts (editing keys are native via ShortcutOverride; New/Open/Save bind window-level)
  //
  Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.formatSelection()
    sequences: ["Ctrl+I"]
  } Shortcut {
    enabled: frameParser.activeFocus
    onActivated: frameParser.formatDocument()
    sequences: ["Ctrl+Shift+I"]
  }

  //
  // Right-click context menu
  //
  CodeEditorMenu {
    id: contextMenu

    codeEditor: frameParser
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

      //
      // Template selector
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

          Label {
            id: languageLabel

            text: qsTr("Platform:")
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.uiFont
          }

          Widgets.Combo {
            id: languageSelector

            model: ["JavaScript", "Lua", qsTr("Built-In")]
            Layout.alignment: Qt.AlignVCenter
            currentIndex: frameParser.language

            onActivated: {
              frameParser.switchLanguage(currentIndex)
              currentIndex = Qt.binding(function() {
                return frameParser.language
              })
            }
          }

          Item {
            Layout.fillWidth: true
          }

          Widgets.IconButton {
            horizontalPadding: 12
            text: qsTr("Select Template…")
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/buttons/code.svg"
            onClicked: {
              if (root.nativeMode)
                nativePane.editor.selectTemplate()
              else
                frameParser.selectTemplate()
            }
          }

          Widgets.IconButton {
            id: testButton

            horizontalPadding: 12
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Test With Sample Data")
            icon.source: "qrc:/icons/buttons/test.svg"
            onClicked: {
              if (root.nativeMode || frameParser.prepareParserTest())
                parserTestLoader.openTester(frameParser.sourceId)
            }
          }

          Widgets.IconButton {
            text: qsTr("Help")
            horizontalPadding: 12
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/buttons/help.svg"
            onClicked: app.showHelpCenter("javascript-api")
          }
        }
      }

      //
      // Separator
      //
      Rectangle {
        z: 2
        implicitHeight: 1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      //
      // Frame parser editor
      //
      StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: root.nativeMode ? 1 : 0

        ColumnLayout {
          spacing: -1

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
                ToolTip.text: qsTr("Reset to the default parsing script")
                icon.source: Cpp_Misc_IconRegistry.icon("code", "reload", 24)
              }

              Widgets.ToolbarButton {
                iconSize: 24
                text: qsTr("Open")
                toolbarButton: false
                onClicked: frameParser.importFile()
                Layout.alignment: Qt.AlignVCenter
                icon.source: Cpp_Misc_IconRegistry.icon("code", "open", 24)
                ToolTip.text: qsTr("Import a script file for data parsing")
              }

              Widgets.ToolbarButton {
                iconSize: 24
                text: qsTr("Undo")
                toolbarButton: false
                onClicked: frameParser.undo()
                Layout.alignment: Qt.AlignVCenter
                enabled: frameParser.undoAvailable
                ToolTip.text: qsTr("Undo the last code edit")
                icon.source: Cpp_Misc_IconRegistry.icon("code", "undo", 24)
              }

              Widgets.ToolbarButton {
                iconSize: 24
                text: qsTr("Redo")
                toolbarButton: false
                onClicked: frameParser.redo()
                Layout.alignment: Qt.AlignVCenter
                enabled: frameParser.redoAvailable
                ToolTip.text: qsTr("Redo the previously undone edit")
                icon.source: Cpp_Misc_IconRegistry.icon("code", "redo", 24)
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
                ToolTip.text: qsTr("Cut selected code to clipboard")
                icon.source: Cpp_Misc_IconRegistry.icon("code", "cut", 24)
              }

              Widgets.ToolbarButton {
                iconSize: 24
                text: qsTr("Copy")
                toolbarButton: false
                onClicked: frameParser.copy()
                Layout.alignment: Qt.AlignVCenter
                ToolTip.text: qsTr("Copy selected code to clipboard")
                icon.source: Cpp_Misc_IconRegistry.icon("code", "copy", 24)
              }

              Widgets.ToolbarButton {
                iconSize: 24
                text: qsTr("Paste")
                toolbarButton: false
                onClicked: frameParser.paste()
                Layout.alignment: Qt.AlignVCenter
                ToolTip.text: qsTr("Paste code from clipboard")
                icon.source: Cpp_Misc_IconRegistry.icon("code", "paste", 24)
              }

              Item {
                Layout.fillWidth: true
              }

              Widgets.ToolbarButton {
                iconSize: 24
                toolbarButton: false
                text: qsTr("Validate")
                Layout.alignment: Qt.AlignVCenter
                onClicked: frameParser.evaluate()
                icon.source: Cpp_Misc_IconRegistry.icon("code", "test", 24)
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

          JsCodeEditor {
            id: frameParser

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

        NativeParserPane {
          id: nativePane

          sourceId: frameParser.sourceId
        }
      }
    }
  }
}
