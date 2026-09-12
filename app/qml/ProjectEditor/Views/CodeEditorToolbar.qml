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

Rectangle {
  id: root

  //
  // The code editor the buttons act on, plus the per-view strings
  //
  required property Item codeEditor
  required property string helpPage
  required property string resetTooltip
  required property string importTooltip
  required property string helpTooltip
  required property string validateTooltip

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
      ToolTip.text: root.resetTooltip
      Layout.alignment: Qt.AlignVCenter
      onClicked: root.codeEditor.reload()
      icon.source: Cpp_Misc_IconRegistry.icon("code", "reload", 24)
    }

    Widgets.ToolbarButton {
      iconSize: 24
      text: qsTr("Open")
      toolbarButton: false
      ToolTip.text: root.importTooltip
      Layout.alignment: Qt.AlignVCenter
      onClicked: root.codeEditor.importFile()
      icon.source: Cpp_Misc_IconRegistry.icon("code", "open", 24)
    }

    Widgets.ToolbarButton {
      iconSize: 24
      text: qsTr("Undo")
      toolbarButton: false
      onClicked: root.codeEditor.undo()
      Layout.alignment: Qt.AlignVCenter
      enabled: root.codeEditor.undoAvailable
      ToolTip.text: qsTr("Undo the last code edit")
      icon.source: Cpp_Misc_IconRegistry.icon("code", "undo", 24)
    }

    Widgets.ToolbarButton {
      iconSize: 24
      text: qsTr("Redo")
      toolbarButton: false
      onClicked: root.codeEditor.redo()
      Layout.alignment: Qt.AlignVCenter
      enabled: root.codeEditor.redoAvailable
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
      onClicked: root.codeEditor.cut()
      Layout.alignment: Qt.AlignVCenter
      ToolTip.text: qsTr("Cut selected code to clipboard")
      icon.source: Cpp_Misc_IconRegistry.icon("code", "cut", 24)
    }

    Widgets.ToolbarButton {
      iconSize: 24
      text: qsTr("Copy")
      toolbarButton: false
      onClicked: root.codeEditor.copy()
      Layout.alignment: Qt.AlignVCenter
      ToolTip.text: qsTr("Copy selected code to clipboard")
      icon.source: Cpp_Misc_IconRegistry.icon("code", "copy", 24)
    }

    Widgets.ToolbarButton {
      iconSize: 24
      text: qsTr("Paste")
      toolbarButton: false
      onClicked: root.codeEditor.paste()
      Layout.alignment: Qt.AlignVCenter
      ToolTip.text: qsTr("Paste code from clipboard")
      icon.source: Cpp_Misc_IconRegistry.icon("code", "paste", 24)
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
      icon.source: Cpp_Misc_IconRegistry.icon("code", "help", 24)
      onClicked: app.showHelpCenter(root.helpPage)
      ToolTip.text: root.helpTooltip
    }

    Item {
      Layout.fillWidth: true
    }

    Widgets.ToolbarButton {
      iconSize: 24
      toolbarButton: false
      text: qsTr("Validate")
      Layout.alignment: Qt.AlignVCenter
      ToolTip.text: root.validateTooltip
      onClicked: root.codeEditor.evaluate()
      icon.source: Cpp_Misc_IconRegistry.icon("code", "test", 24)
    }
  }
}
