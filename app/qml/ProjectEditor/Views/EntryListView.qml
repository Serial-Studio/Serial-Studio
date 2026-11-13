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

  actionComponent: EditorNavActions {}

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  //
  // Rows of {icon, title, summary, open()} in tree order, one per project-level page
  //
  property var entries: []

  Widgets.ThemedPage {
    anchors.fill: parent

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
          id: entryRow

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
                  color: entryRow.textColor
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.boldUiFont
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: entryRow.separatorColor
            }

            Label {
              Layout.leftMargin: 8
              Layout.rightMargin: 8
              Layout.fillWidth: true
              text: modelData.summary
              wrapMode: Text.WordWrap
              color: entryRow.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.uiFont
            }
          }
        }
      }
    }
  }
}
