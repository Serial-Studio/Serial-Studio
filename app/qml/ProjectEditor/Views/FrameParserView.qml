/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import SerialStudio as SerialStudio

import "../../Widgets" as Widgets

Widgets.Pane {
  icon: Cpp_JSON_ProjectModel.selectedIcon
  Component.onCompleted: Cpp_JSON_FrameBuilder.setFrameParser(frameParser)
  title: Cpp_JSON_ProjectModel.selectedText + (frameParser.isModified ? " (" + qsTr("modified") + ")" : "")

  //
  // Super important to allow the user to type on a C++ widget from QML
  //
  onVisibleChanged: {
    if (visible)
      frameParser.forceActiveFocus()
  }

  //
  // Shortcuts
  //
  Shortcut {
    onActivated: frameParser.selectAll()
    sequences: [StandardKey.SelectAll]
  } Shortcut {
    onActivated: frameParser.reload()
    sequences: [StandardKey.New]
  } Shortcut {
    onActivated: frameParser.import()
    sequences: [StandardKey.Open]
  } Shortcut {
    onActivated: frameParser.undo()
    sequences: [StandardKey.Undo]
  } Shortcut {
    onActivated: frameParser.redo()
    sequences: [StandardKey.Redo]
  } Shortcut {
    onActivated: frameParser.cut()
    sequences: [StandardKey.Cut]
  } Shortcut {
    onActivated: frameParser.copy()
    sequences: [StandardKey.Copy]
  } Shortcut {
    onActivated: frameParser.paste()
    sequences: [StandardKey.Paste]
  } Shortcut {
    onActivated: frameParser.apply()
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
      spacing: -1
      anchors.margins: 0
      anchors.fill: parent
      anchors.topMargin: -16
      anchors.leftMargin: -10
      anchors.rightMargin: -10
      anchors.bottomMargin: -9

      //
      // Group actions panel
      //
      Rectangle {
        z: 2
        Layout.fillWidth: true
        Layout.maximumHeight: Layout.minimumHeight
        Layout.minimumHeight: layout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        //
        // Buttons
        //
        RowLayout {
          id: layout
          spacing: 4

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          //
          // Load default document
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Reset")
            toolbarButton: false
            onClicked: frameParser.reload()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/reload.svg"
          }

          //
          // Import code file
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Import")
            toolbarButton: false
            onClicked: frameParser.import()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/import.svg"
          }

          //
          // Save changes
          //
          Widgets.BigButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Validate \& Save")
            onClicked: frameParser.apply()
            enabled: frameParser.isModified
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/apply.svg"
          }

          //
          // Spacer
          //
          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            Layout.maximumHeight: 48
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          //
          // Undo
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Undo")
            toolbarButton: false
            onClicked: frameParser.undo()
            enabled: frameParser.isModified
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/undo.svg"
          }

          //
          // Redo
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Redo")
            toolbarButton: false
            onClicked: frameParser.redo()
            enabled: frameParser.isModified
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/redo.svg"
          }

          //
          // Spacer
          //
          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            Layout.maximumHeight: 48
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          //
          // Cut
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Cut")
            toolbarButton: false
            onClicked: frameParser.cut()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/cut.svg"
          }

          //
          // Copy
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Copy")
            toolbarButton: false
            onClicked: frameParser.copy()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/copy.svg"
          }

          //
          // Paste
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Paste")
            toolbarButton: false
            onClicked: frameParser.paste()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/paste.svg"
          }

          //
          // Spacer
          //
          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            Layout.maximumHeight: 48
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          //
          // Help
          //
          Widgets.BigButton {
            iconSize: 24
            text: qsTr("Help")
            toolbarButton: false
            onClicked: frameParser.help()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/help.svg"
          }

          //
          // Spacer
          //
          Item {
            Layout.fillWidth: true
          }
        }

        //
        // Bottom border
        //
        Rectangle {
          height: 1
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }
      }

      //
      // Code editor widget
      //
      SerialStudio.FrameParser {
        id: frameParser
        Layout.fillWidth: true
        Layout.fillHeight: true

        MouseArea {
          id: mouseArea
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
