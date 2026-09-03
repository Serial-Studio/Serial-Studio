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

//
// The extension grid: the catalog cards, their install/run actions and the section headers.
//
Item {
  id: root

  required property bool fetchingData
  required property bool showDetail
  required property bool showRepos

  width: parent.width
  height: parent.height
  visible: !root.fetchingData
  x: (root.showDetail || root.showRepos) ? -width : 0

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

  Flickable {
    clip: true
    anchors.margins: 8
    anchors.fill: parent
    boundsBehavior: Flickable.StopAtBounds
    contentHeight: gridColumn.implicitHeight
    flickableDirection: Flickable.VerticalFlick

    Column {
      id: gridColumn

      spacing: 4
      width: parent.width

      Repeater {
        id: sectionRepeater

        model: {
          // Build ordered list of unique types from extensions
          var types = []
          var seen = {}
          var exts = Cpp_ExtensionManager.extensions
          for (var i = 0; i < exts.length; ++i) {
            var t = exts[i].type || ""
            if (t !== "" && !seen[t]) {
              seen[t] = true
              types.push(t)
            }
          }
          return types
        }

        delegate: Column {
          spacing: 4
          width: gridColumn.width

          required property string modelData
          required property int index

          //
          // Section header
          //
          Item {
            width: parent.width
            height: 28
            visible: Cpp_ExtensionManager.filterType === ""
                     || Cpp_ExtensionManager.filterType === "All"

            RowLayout {
              spacing: 8
              anchors.fill: parent
              anchors.leftMargin: 4
              anchors.rightMargin: 4

              Label {
                text: Cpp_ExtensionManager.friendlyTypeName(modelData)
                color: Cpp_ThemeManager.colors["pane_section_label"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
                Component.onCompleted: font.capitalization = Font.AllUppercase
              }

              Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: Cpp_ThemeManager.colors["groupbox_border"]
              }
            }
          }

          //
          // Cards grid for this type
          //
          Flow {
            spacing: 10
            width: parent.width

            Repeater {
              model: {
                var items = []
                var exts = Cpp_ExtensionManager.extensions
                for (var i = 0; i < exts.length; ++i) {
                  if ((exts[i].type || "") === modelData) {
                    var item = exts[i]
                    item._flatIndex = i
                    items.push(item)
                  }
                }
                return items
              }

              delegate: Rectangle {
                id: card

                width: 200
                height: 186
        radius: 6
        color: cardMouse.containsMouse ? Cpp_ThemeManager.colors["highlight"]
                                       : Cpp_ThemeManager.colors["base"]
        border.width: 1
        border.color: cardMouse.containsMouse ? Cpp_ThemeManager.colors["accent"]
                                              : Cpp_ThemeManager.colors["mid"]

        Behavior on color {
          ColorAnimation { duration: 150 }
        }

        Behavior on border.color {
          ColorAnimation { duration: 150 }
        }

        ColumnLayout {
          spacing: 4
          anchors.margins: 6
          anchors.fill: parent

          //
          // Screenshot or gradient placeholder
          //
          Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
              radius: 4
              clip: true
              anchors.fill: parent
              color: "transparent"

              gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                  position: 0
                  color: {
                    var p = [
                          "#2c3e50", "#1a5276", "#1e8449", "#7d3c98",
                          "#a04000", "#1b4f72", "#196f3d", "#6c3483",
                          "#922b21", "#117a65", "#7e5109", "#2e4053"
                        ]
                    return p[index % p.length]
                  }
                }
                GradientStop {
                  position: 1
                  color: {
                    var p = [
                          "#1a252f", "#0e2f43", "#12522c", "#4a2460",
                          "#612600", "#0f2d42", "#0e4124", "#3f1f4e",
                          "#561914", "#0a493d", "#4b3006", "#1b2631"
                        ]
                    return p[index % p.length]
                  }
                }
              }

              Image {
                smooth: true
                mipmap: true
                asynchronous: true
                anchors.fill: parent
                visible: status === Image.Ready
                fillMode: Image.PreserveAspectCrop
                source: {
                  const ss = modelData.screenshot || ""
                  if (ss === "")
                    return ""

                  const base = modelData._repoBase || ""
                  if (modelData._isLocal)
                    return "file://" + base + ss

                  return base + ss
                }
              }

              Label {
                opacity: 0.6
                color: Cpp_ThemeManager.colors["bright_text"]
                anchors.centerIn: parent
                text: (modelData.title || "?").charAt(0)
                visible: (modelData.screenshot || "") === ""
                font: Cpp_Misc_CommonFonts.customMonoFont(2.5, true)
              }
            }

            //
            // Status badges (top-right overlay)
            //
            Row {
              spacing: 4
              anchors.margins: 6
              anchors.top: parent.top
              anchors.right: parent.right

              Rectangle {
                radius: 3
                color: Cpp_ThemeManager.colors["alarm_ok"]
                width: runLabel.implicitWidth + 8
                height: runLabel.implicitHeight + 4
                visible: modelData.pluginRunning || false

                Label {
                  id: runLabel

                  color: Cpp_ThemeManager.colors["highlighted_text"]
                  text: qsTr("Running")
                  anchors.centerIn: parent
                  font: Cpp_Misc_CommonFonts.customUiFont(0.7, true)
                }
              }

              Rectangle {
                visible: (modelData.installed || false)
                         && !(modelData.pluginRunning || false)
                width: installedLabel.implicitWidth + 8
                height: installedLabel.implicitHeight + 4
                radius: 3
                color: modelData.updateAvailable
                       ? Cpp_ThemeManager.colors["error"]
                       : Cpp_ThemeManager.colors["accent"]

                Label {
                  id: installedLabel

                  color: Cpp_ThemeManager.colors["highlighted_text"]
                  anchors.centerIn: parent
                  font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
                  text: modelData.updateAvailable ? qsTr("Update") : qsTr("Installed")
                }
              }

              Rectangle {
                visible: modelData.platformAvailable === false
                         && !(modelData.installed || false)
                width: unavailLabel.implicitWidth + 8
                height: unavailLabel.implicitHeight + 4
                radius: 3
                color: Cpp_ThemeManager.colors["placeholder_text"]

                Label {
                  id: unavailLabel

                  color: Cpp_ThemeManager.colors["highlighted_text"]
                  anchors.centerIn: parent
                  text: qsTr("Unavailable")
                  font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
                }
              }
            }

            //
            // Hover description overlay (slides up)
            //
            Item {
              clip: true
              anchors.fill: parent
              height: parent.height
              anchors.bottom: parent.bottom

              Rectangle {
                width: parent.width
                height: parent.height
                radius: 4
                opacity: 0.92
                y: cardMouse.containsMouse ? 0 : parent.height
                color: Cpp_ThemeManager.colors["groupbox_background"]
                border.width: 1
                border.color: Cpp_ThemeManager.colors["groupbox_border"]

                Behavior on y {
                  NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutCubic
                  }
                }

                ColumnLayout {
                  spacing: 4
                  anchors.margins: 8
                  anchors.fill: parent

                  Label {
                    text: modelData.title || ""
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    color: Cpp_ThemeManager.colors["text"]
                    font: Cpp_Misc_CommonFonts.boldUiFont
                  }

                  Rectangle {
                    implicitHeight: 1
                    Layout.fillWidth: true
                    color: Cpp_ThemeManager.colors["groupbox_border"]
                  }

                  Label {
                    text: modelData.description || ""
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    maximumLineCount: 5
                    color: Cpp_ThemeManager.colors["text"]
                    font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                  }
                }
              }
            }
          }

          //
          // Title
          //
          Label {
            text: modelData.title || ""
            elide: Text.ElideRight
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: cardMouse.containsMouse
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : Cpp_ThemeManager.colors["text"]
          }

          //
          // Type + author
          //
          RowLayout {
            spacing: 4
            Layout.fillWidth: true

            Label {
              text: Cpp_ExtensionManager.friendlyTypeName(modelData.type || "")
              font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }

            Item { Layout.fillWidth: true }

            Label {
              text: modelData.author || ""
              font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              elide: Text.ElideRight
              Layout.maximumWidth: 100
            }
          }
        }

        MouseArea {
          id: cardMouse

          hoverEnabled: true
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor
          onClicked: Cpp_ExtensionManager.setSelectedIndex(modelData._flatIndex)
        }
      }
    }
  }

  Item { implicitHeight: 6 }

        }
      }
    }
  }
}
