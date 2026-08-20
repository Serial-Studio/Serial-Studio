/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "."
import "../../Widgets" as Widgets

Window {
  id: root

  Widgets.WindowMirror {}

  width: 1024
  height: 720
  minimumWidth: 720
  minimumHeight: 480
  flags: Qt.Dialog
  modality: Qt.ApplicationModal
  title: qsTr("Canvas Widget Code Editor")
  color: Cpp_ThemeManager.colors["window"]

  function showDialog() {
    show()
    raise()
    requestActivate()
  }

  //
  // Commit pending edits on every close path before the editor is destroyed.
  //
  onClosing: {
    if (editor)
      editor.commit()
  }
  Component.onDestruction: {
    if (editor)
      editor.commit()
  }

  ColumnLayout {
    spacing: -1
    anchors.margins: 0
    anchors.fill: parent

    Rectangle {
      Layout.fillWidth: true
      height: header.implicitHeight + 12
      color: Cpp_ThemeManager.colors["table_bg_header_top"]

      RowLayout {
        id: header

        spacing: 4
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        Image {
          sourceSize: Qt.size(18, 18)
          Layout.alignment: Qt.AlignVCenter
          source: Cpp_Misc_IconRegistry.icon("editor", "code", 16)
        }

        Label {
          text: qsTr("paint(ctx, w, h)")
          Layout.alignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["table_fg_header"]
        }

        Item { Layout.fillWidth: true }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Import")
          toolbarButton: false
          horizontalLayout: true
          onClicked: editor.importFile()
          Layout.alignment: Qt.AlignVCenter
          ToolTip.text: qsTr("Import canvas code from a .js file")
          icon.source: Cpp_Misc_IconRegistry.icon("code", "open", 16)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          toolbarButton: false
          horizontalLayout: true
          text: qsTr("Template")
          Layout.alignment: Qt.AlignVCenter
          onClicked: editor.selectTemplate()
          ToolTip.text: qsTr("Select a built-in canvas template")
          icon.source: Cpp_Misc_IconRegistry.icon("code", "template", 16)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Format")
          toolbarButton: false
          horizontalLayout: true
          onClicked: editor.formatDocument()
          Layout.alignment: Qt.AlignVCenter
          ToolTip.text: qsTr("Reformat the canvas code")
          icon.source: Cpp_Misc_IconRegistry.icon("code", "reload", 16)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Test")
          toolbarButton: false
          horizontalLayout: true
          Layout.alignment: Qt.AlignVCenter
          icon.source: Cpp_Misc_IconRegistry.icon("code", "test", 16)
          ToolTip.text: qsTr("Open a live preview with simulated dataset values")
          onClicked: {
            const ds = Cpp_JSON_ProjectEditor.currentGroupDatasetsForPreview()
            testDialog.showDialog(editor.text, qsTr("Preview"), ds)
          }
        }
      }
    }

    PainterTestDialog {
      id: testDialog
    }

    Rectangle {
      Layout.fillWidth: true
      height: 1
      color: Cpp_ThemeManager.colors["table_border_header"]
    }

    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true

      PainterCodeEditor {
        id: editor

        anchors.fill: parent
      }

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

    Rectangle {
      Layout.fillWidth: true
      height: 1
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    RowLayout {
      spacing: 8
      Layout.margins: 8
      Layout.fillWidth: true

      Item { Layout.fillWidth: true }

      Button {
        text: qsTr("Close")
        onClicked: root.close()
      }
    }
  }

  Menu {
    id: contextMenu

    MenuItem { text: qsTr("Cut");        onTriggered: editor.cut() }
    MenuItem { text: qsTr("Copy");       onTriggered: editor.copy() }
    MenuItem { text: qsTr("Paste");      onTriggered: editor.paste() }
    MenuItem { text: qsTr("Select All"); onTriggered: editor.selectAll() }
    MenuSeparator {}
    MenuItem { text: qsTr("Undo");       onTriggered: editor.undo() }
    MenuItem { text: qsTr("Redo");       onTriggered: editor.redo() }
    MenuSeparator {}
    MenuItem { text: qsTr("Format Document");  onTriggered: editor.formatDocument() }
    MenuItem { text: qsTr("Format Selection"); onTriggered: editor.formatSelection() }
  }
}
