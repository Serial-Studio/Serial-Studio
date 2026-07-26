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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets
import "../../Commands" as Commands
import "../Dialogs" as Dialogs

Rectangle {
  id: root

  //
  // Calculate offset based on platform
  //
  property int titlebarHeight: Cpp_NativeWindow.titlebarHeight(projectEditor)
  Connections {
    target: projectEditor
    function onVisibilityChanged() {
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(projectEditor)
    }
  }

  //
  // Set toolbar height
  //
  Layout.minimumHeight: titlebarHeight + 64 + 16
  Layout.maximumHeight: titlebarHeight + 64 + 16

  //
  // Command registry plumbing
  //
  Commands.ProjectEditorCommandBindings {
    id: _ptBindings

    editorWindow: projectEditor
  }

  Commands.CommandModel {
    id: _ptModel

    context: "editor"
    bindingSets: [_ptBindings]
  }

  //
  // Top toolbar section
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["toolbar_top"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  //
  // Titlebar text
  //
  Label {
    text: projectEditor.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["titlebar_text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Toolbar background
  //
  Rectangle {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
    gradient: Gradient {
      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["toolbar_top"]
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["toolbar_bottom"]
      }
    }
  }

  //
  // Toolbar bottom border
  //
  Rectangle {
    height: 1
    color: Cpp_ThemeManager.colors["toolbar_border"]

    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }

  //
  // Move window with toolbar
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        projectEditor.startSystemMove()
    }
  }

  //
  // Command toolbar (registry-driven ribbon)
  //
  Widgets.CommandToolbar {
    anchors {
      left: parent.left
      right: parent.right
      verticalCenter: parent.verticalCenter
      verticalCenterOffset: root.titlebarHeight / 2
    }
    height: 64 + 16

    model: _ptModel
    surface: "project-toolbar"
  }

  //
  // Protobuf preview dialog (shown when Cpp_JSON_ProtoImporter emits previewReady)
  //
  Dialogs.ProtoPreviewDialog {
    id: protoPreviewDialog
  }
}
