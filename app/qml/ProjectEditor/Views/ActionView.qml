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

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_JSON_ProjectModel.selectedIcon
  title: Cpp_JSON_ProjectModel.selectedText

  //
  // User interface elements
  //
  Page {
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

    anchors {
      fill: parent
      topMargin: -16
      leftMargin: -9
      rightMargin: -9
      bottomMargin: -10
    }

    //
    // Actions panel
    //
    Rectangle {
      z: 2
      id: header
      height: layout.implicitHeight + 12
      color: Cpp_ThemeManager.colors["groupbox_background"]
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

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
        // Change icon
        //
        Widgets.ToolbarButton {
          iconSize: 24
          toolbarButton: false
          text: qsTr("Change Icon")
          Layout.alignment: Qt.AlignVCenter
          ToolTip.text: qsTr("Change the icon used for this action")
          icon.source: "qrc:/rcc/actions/" + Cpp_JSON_ProjectModel.actionIcon + ".svg"
          onClicked: {
            actionIconPicker.selectedIcon = Cpp_JSON_ProjectModel.actionIcon
            actionIconPicker.showNormal()
          }
        }

        //
        // Spacer
        //
        Item {
          Layout.fillWidth: true
        }

        //
        // Duplicate action
        //
        Widgets.ToolbarButton {
          iconSize: 24
          toolbarButton: false
          text: qsTr("Duplicate")
          Layout.alignment: Qt.AlignVCenter
          ToolTip.text: qsTr("Duplicate this action with all its settings")
          onClicked: Cpp_JSON_ProjectModel.duplicateCurrentAction()
          icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
        }

        //
        // Delete action
        //
        Widgets.ToolbarButton {
          iconSize: 24
          text: qsTr("Delete")
          toolbarButton: false
          Layout.alignment: Qt.AlignVCenter
          onClicked: Cpp_JSON_ProjectModel.deleteCurrentAction()
          ToolTip.text: qsTr("Delete this action from the project")
          icon.source: "qrc:/rcc/icons/project-editor/actions/delete.svg"
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
    // Action model editor
    //
    ScrollView {
      id: view
      anchors.fill: parent
      contentWidth: width
      anchors.topMargin: header.height
      contentHeight: delegate.implicitHeight
      ScrollBar.vertical.policy: delegate.implicitHeight > view.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

      TableDelegate {
        id: delegate
        width: parent.width
        headerVisible: false
        parameterWidth: Math.min(delegate.width * 0.3, 256)

        Binding {
          target: delegate
          property: "modelPointer"
          value: Cpp_JSON_ProjectModel.actionModel
        }
      }
    }
  }
}
