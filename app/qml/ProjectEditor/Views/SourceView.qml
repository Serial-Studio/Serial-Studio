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
  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_JSON_ProjectEditor.selectedIcon
  title: Cpp_JSON_ProjectEditor.selectedText

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
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -10
    }

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      //
      // Source actions panel (commercial builds only)
      //
      Rectangle {
        id: header

        z: 2
        visible: Cpp_CommercialBuild
        Layout.fillWidth: true
        height: headerLayout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        RowLayout {
          id: headerLayout

          spacing: 4

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Item {
            Layout.fillWidth: true
          }

          //
          // Duplicate source
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Duplicate")
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
            ToolTip.text: qsTr("Create a copy of this data source")
            onClicked: Cpp_JSON_ProjectModel.duplicateSource(
                         Cpp_JSON_ProjectEditor.selectedSourceId)
          }

          //
          // Delete source (disabled for source 0)
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Delete")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectEditor.selectedSourceId > 0
            icon.source: "qrc:/rcc/icons/project-editor/actions/delete.svg"
            ToolTip.text: Cpp_JSON_ProjectEditor.selectedSourceId > 0
                          ? qsTr("Remove this data source")
                          : qsTr("The primary data source cannot be removed")
            onClicked: Cpp_JSON_ProjectModel.deleteSource(
                         Cpp_JSON_ProjectEditor.selectedSourceId)
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
      // Source settings form
      //
      ScrollView {
        id: view

        contentWidth: width
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentHeight: delegate.implicitHeight
        ScrollBar.vertical.policy: delegate.implicitHeight > view.height
                                   ? ScrollBar.AlwaysOn
                                   : ScrollBar.AsNeeded

        TableDelegate {
          id: delegate

          width: parent.width
          headerVisible: false
          parameterWidth: Math.min(delegate.width * 0.3, 256)

          Binding {
            target: delegate
            property: "modelPointer"
            value: Cpp_JSON_ProjectEditor.sourceModel
          }
        }
      }
    }
  }
}
