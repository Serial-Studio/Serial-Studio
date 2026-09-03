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
// The selected extension: screenshot, README, plugin log and the install/remove actions.
//
Item {
  id: root

  required property bool showDetail

  width: parent.width
  height: parent.height
  x: root.showDetail ? 0 : width

  Behavior on x {
    NumberAnimation {
      duration: 300
      easing.type: Easing.OutCubic
    }
  }

  RowLayout {
    spacing: 8
    anchors.fill: parent

    //
    // Left: README markdown viewer
    //
    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true

      Rectangle {
        radius: 2
        border.width: 1
        anchors.fill: parent
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      Loader {
        id: readmeLoader

        anchors.margins: 2
        anchors.fill: parent

        Component.onCompleted: {
          if (Cpp_HasWebEngine) {
            readmeLoader.setSource(
              "qrc:/serial-studio.com/gui/qml/Widgets/MarkdownWebView.qml",
              {
                "markdown": Qt.binding(function() { return Cpp_ExtensionManager.selectedReadme }),
                "emitCopyToast": false
              })
          } else {
            readmeLoader.setSource(
              "qrc:/serial-studio.com/gui/qml/Widgets/MarkdownTextView.qml",
              {
                "markdown": Qt.binding(function() { return Cpp_ExtensionManager.selectedReadme }),
                "placeholderText": qsTr("No description available.")
              })
          }
        }
      }
    }

    //
    // Right sidebar: details + screenshot/plugin log
    //
    ColumnLayout {
      spacing: 8
      Layout.fillHeight: true
      Layout.minimumWidth: 280
      Layout.maximumWidth: 280

      //
      // Details panel
      //
      Item {
        Layout.fillWidth: true
        Layout.minimumHeight: detailsCol.implicitHeight + 24
        Layout.maximumHeight: detailsCol.implicitHeight + 24

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ColumnLayout {
          id: detailsCol

          spacing: 4
          anchors.margins: 8
          anchors.fill: parent

          Item { implicitHeight: 2 }

          Label {
            text: qsTr("Details")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Item { implicitHeight: 2 }

          GridLayout {
            columns: 2
            rowSpacing: 4
            columnSpacing: 8
            Layout.fillWidth: true

            Label {
              text: qsTr("Type:")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["text"]
            }
            Label {
              text: Cpp_ExtensionManager.friendlyTypeName(
                      Cpp_ExtensionManager.selectedExtension.type || "")
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["text"]
              Layout.fillWidth: true
            }

            Label {
              text: qsTr("Author:")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["text"]
            }
            Label {
              text: Cpp_ExtensionManager.selectedExtension.author || ""
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["text"]
              Layout.fillWidth: true
            }

            Label {
              text: qsTr("Version:")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["text"]
            }
            Label {
              text: Cpp_ExtensionManager.selectedExtension.version || ""
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["text"]
              Layout.fillWidth: true
            }

            Label {
              text: qsTr("License:")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["text"]
            }
            Label {
              text: Cpp_ExtensionManager.selectedExtension.license || ""
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["text"]
              Layout.fillWidth: true
            }
          }
        }
      }

      //
      // Screenshot
      //
      Item {
        Layout.fillWidth: true
        Layout.minimumHeight: extensionScreenshot.status === Image.Ready
                              ? Math.min(extensionScreenshot.sourceSize.height
                                         * (268 / extensionScreenshot.sourceSize.width), 200)
                              : 140
        Layout.maximumHeight: Layout.minimumHeight
        visible: (Cpp_ExtensionManager.selectedExtension.screenshot || "") !== ""

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Image {
          id: extensionScreenshot

          mipmap: true
          smooth: true
          anchors.margins: 1
          asynchronous: true
          anchors.fill: parent
          visible: status === Image.Ready
          fillMode: Image.PreserveAspectFit
          source: {
            const ss = Cpp_ExtensionManager.selectedExtension.screenshot || ""
            if (ss === "")
              return ""

            const base = Cpp_ExtensionManager.selectedExtension._repoBase || ""
            const isLocal = Cpp_ExtensionManager.selectedExtension._isLocal || false
            if (isLocal)
              return "file://" + base + ss

            return base + ss
          }
        }

        Label {
          anchors.centerIn: parent
          visible: !extensionScreenshot.visible
          text: qsTr("No preview")
          color: Cpp_ThemeManager.colors["placeholder_text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
        }
      }

      //
      // Spacer that keeps the details panel and screenshot top-aligned
      //
      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: (Cpp_ExtensionManager.selectedExtension.screenshot || "") !== ""
                 && (Cpp_ExtensionManager.selectedExtension.type || "") !== "plugin"
      }

      //
      // Plugin output log
      //
      Item {
        visible: opacity > 0
        Layout.fillWidth: true
        Layout.fillHeight: true
        opacity: (Cpp_ExtensionManager.selectedExtension.type || "") === "plugin" ? 1 : 0

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["console_base"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ColumnLayout {
          spacing: 0
          anchors.margins: 1
          anchors.fill: parent

          Label {
            text: qsTr("  PLUGIN OUTPUT")
            font: Cpp_Misc_CommonFonts.customUiFont(0.7, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Layout.fillWidth: true
            Layout.topMargin: 6
            Layout.bottomMargin: 4
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          ScrollView {
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
              id: pluginLogArea

              readOnly: true
              wrapMode: TextEdit.Wrap
              color: Cpp_ThemeManager.colors["console_text"]
              font: Cpp_Misc_CommonFonts.customMonoFont(0.8, false)
              background: Item {}
              text: {
                const id = Cpp_ExtensionManager.selectedExtension.id || ""
                return id !== "" ? (Cpp_ExtensionManager.pluginOutput(id) || qsTr("No output yet. Run the plugin to see its log here."))
                                 : ""
              }

              Connections {
                target: Cpp_ExtensionManager
                function onPluginOutputChanged(pluginId) {
                  if (pluginId === (Cpp_ExtensionManager.selectedExtension.id || ""))
                    pluginLogArea.text = Cpp_ExtensionManager.pluginOutput(pluginId)
                }
              }
            }
          }
        }
      }

      //
      // Spacer when no screenshot and not a plugin
      //
      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: (Cpp_ExtensionManager.selectedExtension.screenshot || "") === ""
                 && (Cpp_ExtensionManager.selectedExtension.type || "") !== "plugin"

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Label {
          anchors.centerIn: parent
          text: qsTr("No preview available")
          color: Cpp_ThemeManager.colors["placeholder_text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
        }
      }
    }
  }
}
