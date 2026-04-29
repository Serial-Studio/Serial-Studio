/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  fixedSize: false
  preferredWidth: 560
  preferredHeight: 480
  title: qsTr("New Deployment")

  property string shortcutTitle: ""
  property string iconPathValue: ""
  property string lastErrorText: ""
  property bool userPickedIcon: false

  readonly property string projectPath: Cpp_AppState.projectFilePath
  readonly property string projectTitle: Cpp_JSON_ProjectModel.title

  onVisibleChanged: {
    if (visible) {
      shortcutTitle = projectTitle
      iconPathValue = Cpp_ShortcutGenerator.defaultIconPath
      userPickedIcon = false
      lastErrorText = ""
    }
  }

  // Keep the title in sync whenever a new project is loaded
  Connections {
    target: Cpp_JSON_ProjectModel
    function onTitleChanged() {
      root.shortcutTitle = root.projectTitle
    }
  }

  Connections {
    target: Cpp_ShortcutGenerator

    function onShortcutGenerated(path) {
      lastErrorText = ""
      root.close()
    }

    function onShortcutFailed(message) {
      lastErrorText = message
    }
  }

  FileDialog {
    id: iconPicker
    title: qsTr("Choose an Icon")
    fileMode: FileDialog.OpenFile
    currentFolder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)
    nameFilters: [Cpp_ShortcutGenerator.iconFileFilter]
    onAccepted: {
      root.iconPathValue = selectedFile
      root.userPickedIcon = true
    }
  }

  FileDialog {
    id: outputPicker
    title: qsTr("Save Deployment")
    fileMode: FileDialog.SaveFile
    currentFolder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)
    defaultSuffix: Cpp_ShortcutGenerator.shortcutDefaultSuffix
    nameFilters: [Cpp_ShortcutGenerator.shortcutFileFilter]
    onAccepted: {
      Cpp_ShortcutGenerator.generate(
            selectedFile,
            root.shortcutTitle,
            root.iconPathValue,
            root.projectPath,
            fullscreen.checked,
            csvExport.checked,
            mdfExport.checked,
            sessionExport.checked,
            consoleExport.checked)
    }
  }

  dialogContent: ColumnLayout {
    id: layout
    spacing: 12
    anchors.centerIn: parent

    //
    // Tab bar
    //
    TabBar {
      id: _tab

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        text: qsTr("General")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Logging")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }
    }

    //
    // Tab contents
    //
    StackLayout {
      id: stack

      clip: true
      Layout.fillWidth: true
      Layout.minimumWidth: 520
      currentIndex: _tab.currentIndex
      Layout.topMargin: -parent.spacing - 1
      implicitHeight: Math.max(generalTab.implicitHeight, loggingTab.implicitHeight)

      //
      // General tab
      //
      Item {
        id: generalTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: generalLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: generalLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.margins: 8
          anchors.fill: parent

          //
          // Section: Identity
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Identity")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          //
          // Icon + Name row
          //
          RowLayout {
            spacing: 12
            Layout.columnSpan: 2
            Layout.fillWidth: true

            Item {
              id: iconTile
              implicitWidth: 96
              implicitHeight: 96
              Layout.alignment: Qt.AlignVCenter


              Image {
                anchors.fill: parent
                anchors.margins: 8
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
                asynchronous: true
                cache: false
                sourceSize: Qt.size(192, 192)
                source: root.userPickedIcon
                        ? root.iconPathValue
                        : "qrc:/rcc/images/shortcut.svg"
              }

              HoverHandler {
                id: iconHover
                cursorShape: Qt.PointingHandCursor
              }

              TapHandler {
                onTapped: iconPicker.open()
              }

              ToolTip.delay: 600
              ToolTip.visible: iconHover.hovered
              ToolTip.text: qsTr("Click to choose an icon")
            }

            ColumnLayout {
              spacing: 4
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter

              Label {
                text: qsTr("Name:")
                color: Cpp_ThemeManager.colors["text"]
              }

              TextField {
                id: titleField
                Layout.fillWidth: true
                text: root.shortcutTitle
                placeholderText: qsTr("Deployment Name")
                onTextEdited: root.shortcutTitle = text
              }

              Item {
                implicitHeight: 4
              }

              Button {
                text: qsTr("Change Icon…")
                font: Cpp_Misc_CommonFonts.uiFont
                onClicked: iconPicker.open()
                Layout.alignment: Qt.AlignLeft
              }
            }
          }

          //
          // Section: Project
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Project")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          //
          // Project selector
          //
          Label {
            text: qsTr("Project")
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 2

            TextField {
              readOnly: true
              Layout.fillWidth: true
              text: Cpp_AppState.projectFileName
              Layout.alignment: Qt.AlignVCenter
              placeholderText: qsTr("Choose a project file to begin")
            }

            Button {
              icon.width: 18
              icon.height: 18
              Layout.fillWidth: false
              Layout.maximumWidth: 24
              Layout.maximumHeight: 24
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.openJsonFile()
              icon.color: Cpp_ThemeManager.colors["button_text"]
              icon.source: "qrc:/rcc/icons/buttons/open.svg"
            }
          }

          //
          // Section: Behavior
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Behavior")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Fullscreen")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: fullscreen
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            Layout.columnSpan: 2
            Layout.topMargin: 4
            Layout.fillWidth: true
            wrapMode: Label.Wrap
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: qsTr("Double-clicking this deployment takes someone straight to "
                       + "the live dashboard for this project. There's no toolbar "
                       + "or setup pane, just the data, and Serial Studio quits "
                       + "as soon as the device disconnects.")
          }

          Item { Layout.fillHeight: true }
        }
      }

      //
      // Logging tab
      //
      Item {
        id: loggingTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: loggingLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: loggingLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.margins: 8
          anchors.fill: parent

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Recorders")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("CSV File")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: csvExport
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("MDF4 File")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: mdfExport
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Session Database")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: sessionExport
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Console Log")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: consoleExport
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            Layout.columnSpan: 2
            Layout.topMargin: 8
            Layout.fillWidth: true
            wrapMode: Label.Wrap
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: qsTr("Recordings are saved in the Serial Studio workspace folder")
          }

          Item { Layout.fillHeight: true }
        }
      }
    }

    //
    // Inline error banner
    //
    Label {
      Layout.fillWidth: true
      visible: root.lastErrorText.length > 0
      color: Cpp_ThemeManager.colors["alarm"]
      wrapMode: Label.Wrap
      text: root.lastErrorText
    }

    //
    // Footer buttons
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Item { Layout.fillWidth: true }

      Button {
        icon.width: 18
        icon.height: 18
        text: qsTr("Cancel")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/close.svg"
      }

      Button {
        highlighted: true
        icon.width: 18
        icon.height: 18
        text: qsTr("Save")
        horizontalPadding: 8
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/apply.svg"
        enabled: app.proVersion
                 && root.projectPath.length > 0
                 && root.shortcutTitle.trim().length > 0
        onClicked: {
          const sanitized = root.shortcutTitle.replace(/[\\/:*?"<>|]/g, "_").trim()
          const desktop = StandardPaths.writableLocation(StandardPaths.DesktopLocation)
          const suffix = "." + Cpp_ShortcutGenerator.shortcutDefaultSuffix
          const fileName = (sanitized.length > 0 ? sanitized : "Untitled Deployment") + suffix
          outputPicker.selectedFile = desktop + "/" + fileName
          outputPicker.open()
        }
      }
    }
  }
}
