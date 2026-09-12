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

  implicitWidth: 0
  implicitHeight: 0
  title: qsTr("Project Scripts")
  icon: Cpp_Misc_IconRegistry.icon("commands", "macro", 16)

  actionComponent: EditorNavActions {}

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  //
  // The three project-level scripts, in tree order
  //
  readonly property var entries: [
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "code", 16),
      title: qsTr("Control Loop"),
      summary: qsTr("JavaScript setup() and loop() that run while the project is connected: "
                    + "timers, watchdogs, automated commands."),
      open: function() { Cpp_JSON_ProjectEditor.selectControlScript() }
    },
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "code", 16),
      title: qsTr("Lua Library"),
      summary: qsTr("Functions every Lua value transform of this project can call, so a "
                    + "formula lives in one place."),
      open: function() { Cpp_JSON_ProjectEditor.selectTransformLibrary() }
    },
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "code", 16),
      title: qsTr("JavaScript Library"),
      summary: qsTr("Functions every JavaScript value transform of this project can call."),
      open: function() { Cpp_JSON_ProjectEditor.selectJsLibrary() }
    }
  ]

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
      anchors.fill: parent
      anchors.topMargin: -16
      anchors.leftMargin: -10
      anchors.rightMargin: -10
      anchors.bottomMargin: -9

      ListView {
        id: list

        clip: true
        spacing: 0
        model: root.entries
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        delegate: Widgets.ProjectTableRow {
          id: scriptRow

          rowHeight: 44

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: modelData.open()
          }

          RowLayout {
            spacing: 0
            anchors.fill: parent
            LayoutMirroring.enabled: root.rtl
            LayoutMirroring.childrenInherit: true

            Item {
              Layout.fillHeight: true
              Layout.preferredWidth: 220

              RowLayout {
                spacing: 6
                anchors.fill: parent
                anchors.leftMargin: 8

                Image {
                  source: modelData.icon
                  sourceSize: Qt.size(16, 16)
                  Layout.alignment: Qt.AlignVCenter
                }

                Label {
                  text: modelData.title
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                  color: scriptRow.textColor
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.boldUiFont
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: scriptRow.separatorColor
            }

            Label {
              Layout.leftMargin: 8
              Layout.rightMargin: 8
              Layout.fillWidth: true
              text: modelData.summary
              wrapMode: Text.WordWrap
              color: scriptRow.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.uiFont
            }
          }
        }
      }
    }
  }
}
