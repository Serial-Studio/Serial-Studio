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

import "../../Widgets" as Widgets

//
// The repository list: the configured catalog URLs and the add/remove controls.
//
Item {
  id: root

  required property bool showRepos

  width: parent.width
  height: parent.height
  x: root.showRepos ? 0 : width

  Behavior on x {
    NumberAnimation {
      duration: 300
      easing.type: Easing.OutCubic
    }
  }

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  ColumnLayout {
    spacing: 4
    anchors.margins: 12
    anchors.fill: parent

    Item {
      implicitHeight: 2
    }

    Label {
      text: qsTr("Repositories")
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    Rectangle {
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    Label {
      text: qsTr("Add URLs to remote repositories or local folder paths.")
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["placeholder_text"]
    }

    Item {
      implicitHeight: 4
    }

    ListView {
      id: repoList

      clip: true
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true
      model: Cpp_ExtensionManager.repositories

      delegate: Rectangle {
        width: repoList.width
        height: repoRow.implicitHeight + 8
        radius: 4
        color: Cpp_ThemeManager.colors["base"]
        border.width: 1
        border.color: Cpp_ThemeManager.colors["mid"]

        RowLayout {
          id: repoRow

          spacing: 8
          anchors.margins: 4
          anchors.fill: parent

          Label {
            visible: Cpp_ExtensionManager.isLocalRepo(modelData)
            text: qsTr("LOCAL")
            font: Cpp_Misc_CommonFonts.customUiFont(0.7, true)
            color: Cpp_ThemeManager.colors["highlighted_text"]

            background: Rectangle {
              radius: 2
              color: Cpp_ThemeManager.colors["accent"]
              anchors.fill: parent
              anchors.margins: -2
            }
          }

          Label {
            text: modelData
            elide: Text.ElideMiddle
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.customMonoFont(0.85, false)
            color: Cpp_ThemeManager.colors["text"]
          }

          ToolButton {
            icon.width: 12
            icon.height: 12
            background: Item {}
            icon.color: Cpp_ThemeManager.colors["text"]
            icon.source: "qrc:/icons/buttons/close.svg"
            onClicked: Cpp_ExtensionManager.removeRepository(index)

            HoverHandler {
              cursorShape: Qt.PointingHandCursor
            }
          }
        }
      }
    }

    Rectangle {
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      implicitHeight: 36
      color: Cpp_ThemeManager.colors["base"]
      border.color: newRepoField.activeFocus ? Cpp_ThemeManager.colors["highlight"]
                                             : Cpp_ThemeManager.colors["mid"]

      RowLayout {
        spacing: 4
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4

        Widgets.LineField {
          id: newRepoField

          background: Item {}
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.customMonoFont(0.85, false)
          placeholderText: qsTr("URL or local path…")
          onAccepted: addRepoButton.clicked()
        }

        ToolButton {
          id: addRepoButton

          background: Item {}
          text: qsTr("Add")
          enabled: newRepoField.text.length > 0
          icon.color: Cpp_ThemeManager.colors["text"]
          onClicked: {
            Cpp_ExtensionManager.addRepository(newRepoField.text)
            newRepoField.text = ""
          }

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

        Rectangle {
          implicitWidth: 1
          Layout.topMargin: 6
          Layout.bottomMargin: 6
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["mid"]
        }

        ToolButton {
          background: Item {}
          text: qsTr("Browse…")
          icon.width: 12
          icon.height: 12
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: Cpp_Misc_IconRegistry.icon("commands", "open-project", 16)
          onClicked: Cpp_ExtensionManager.browseLocalRepo()

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

      }
    }
  }
}
