/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_JSON_ProjectModel.selectedIcon
  title: Cpp_JSON_ProjectModel.selectedText
  Component.onCompleted: Cpp_JSON_ProjectModel.buildProjectModel()

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

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      Widgets.ProNotice {
        Layout.margins: -1
        Layout.bottomMargin: 0
        Layout.fillWidth: true
        closeButtonEnabled: false
        titleText: qsTr("Pro features detected in this project.")
        activationFlag: Cpp_JSON_ProjectModel.containsCommercialFeatures
        subtitleText: qsTr("Fallback widgets will be used. Buy a license to unlock full functionality.")
      }

      ScrollView {
        id: view
        contentWidth: width
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentHeight: delegate.implicitHeight

        ScrollBar.vertical: ScrollBar {
          policy: delegate.implicitHeight > view.height ? ScrollBar.AlwaysOn :
                                                          ScrollBar.AsNeeded
        }

        TableDelegate {
          id: delegate
          width: parent.width
          modelPointer: Cpp_JSON_ProjectModel.projectModel

          footerItem: ColumnLayout {
            spacing: 0

            Image {
              sourceSize: Qt.size(96, 96)
              Layout.alignment: Qt.AlignHCenter
              source: "qrc:/rcc/images/tip.svg"
            }

            Item {
              implicitHeight: 16
            }

            Label {
              Layout.alignment: Qt.AlignHCenter
              text: qsTr("First, set up frame detection and data decoding.")
              horizontalAlignment: Label.AlignHCenter
              font: Cpp_Misc_CommonFonts.customUiFont(1.6, true)
            }

            Item {
              implicitHeight: 8
            }

            Label {
              opacity: 0.8
              Layout.alignment: Qt.AlignHCenter
              horizontalAlignment: Label.AlignHCenter
              Layout.maximumWidth: delegate.width * 0.9
              wrapMode: Label.WrapAtWordBoundaryOrAnywhere
              font: Cpp_Misc_CommonFonts.customUiFont(1.4, false)
              text: qsTr("Frames are parsed into arrays, and datasets link each element to a widget.")
            }
          }
        }
      }
    }
  }
}
