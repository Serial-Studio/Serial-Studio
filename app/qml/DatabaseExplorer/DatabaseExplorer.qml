/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartWindow {
  id: root

  minimumWidth: 880
  minimumHeight: 560
  category: "DatabaseExplorer"
  property alias preferredWidth: layout.implicitWidth
  property alias preferredHeight: layout.implicitHeight
  title: Cpp_Sessions_Manager.isOpen
         ? qsTr("Sessions") + " — " + Cpp_Sessions_Manager.fileName
         : qsTr("Sessions")

  onVisibleChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root)
    else
      Cpp_NativeWindow.removeWindow(root)
  }

  Component.onCompleted: Qt.callLater(Cpp_Sessions_Manager.restoreLastDatabase)

  Page {
    clip: true
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
      id: layout
      spacing: 0
      anchors.fill: parent

      //
      // Toolbar — titlebar band + gradient background + RibbonToolbar,
      // same structure as ProjectEditor/Sections/ProjectToolbar.qml.
      //
      Rectangle {
        id: toolbar
        z: 2
        Layout.fillWidth: true
        Layout.minimumHeight: titlebarHeight + 80
        Layout.maximumHeight: titlebarHeight + 80
        Layout.minimumWidth: 720

        property int titlebarHeight: Cpp_NativeWindow.titlebarHeight(root)

        Connections {
          target: root
          function onVisibilityChanged() {
            toolbar.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
          }
        }

        //
        // Titlebar band
        //
        Rectangle {
          height: toolbar.titlebarHeight
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
          text: root.title
          visible: toolbar.titlebarHeight > 0
          color: Cpp_ThemeManager.colors["titlebar_text"]
          font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
          anchors {
            topMargin: 6
            top: parent.top
            horizontalCenter: parent.horizontalCenter
          }
        }

        //
        // Toolbar gradient background
        //
        Rectangle {
          anchors.fill: parent
          anchors.topMargin: toolbar.titlebarHeight
          gradient: Gradient {
            GradientStop { position: 0; color: Cpp_ThemeManager.colors["toolbar_top"] }
            GradientStop { position: 1; color: Cpp_ThemeManager.colors["toolbar_bottom"] }
          }
        }

        //
        // Bottom border
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
        // Window drag
        //
        DragHandler {
          target: null
          onActiveChanged: {
            if (active)
              root.startSystemMove()
          }
        }

        //
        // Ribbon toolbar content
        //
        Widgets.RibbonToolbar {
          height: 80
          anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: toolbar.titlebarHeight / 2
          }

          //
          // Database section
          //
          Widgets.RibbonSection {
            Widgets.ToolbarButton {
              text: qsTr("Open")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Open a session file")
              onClicked: Cpp_Sessions_Manager.openDatabase()
              icon.source: "qrc:/rcc/icons/database/open.svg"
            }

            Widgets.ToolbarButton {
              text: qsTr("Close")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Close session file")
              enabled: Cpp_Sessions_Manager.isOpen
              onClicked: Cpp_Sessions_Manager.closeDatabase()
              icon.source: "qrc:/rcc/icons/database/close.svg"
            }
          }

          //
          // Session section
          //
          Widgets.RibbonSection {
            Widgets.ToolbarButton {
              text: qsTr("Replay")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_Sessions_Manager.isOpen
                       && Cpp_Sessions_Manager.selectedSessionId >= 0
              ToolTip.text: qsTr("Replay selected session on the dashboard")
              onClicked: Cpp_Sessions_Manager.replaySelectedSession()
              icon.source: "qrc:/rcc/icons/database/replay.svg"
            }

            Widgets.ToolbarButton {
              text: qsTr("Delete")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_Sessions_Manager.isOpen
                       && Cpp_Sessions_Manager.selectedSessionId >= 0
              ToolTip.text: qsTr("Delete the selected session")
              onClicked: Cpp_Sessions_Manager.confirmDeleteSession(
                           Cpp_Sessions_Manager.selectedSessionId)
              icon.source: "qrc:/rcc/icons/database/delete.svg"
            }

            Widgets.ToolbarButton {
              text: qsTr("Export")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_Sessions_Manager.isOpen
                       && Cpp_Sessions_Manager.selectedSessionId >= 0
                       && !Cpp_Sessions_Manager.csvExportBusy
              ToolTip.text: qsTr("Export selected session to CSV")
              onClicked: Cpp_Sessions_Manager.exportSessionToCsv(
                           Cpp_Sessions_Manager.selectedSessionId)
              icon.source: "qrc:/rcc/icons/database/export.svg"
            }
          }

          //
          // Project section
          //
          Widgets.RibbonSection {
            showSeparator: false

            Widgets.ToolbarButton {
              text: qsTr("Restore Project")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_Sessions_Manager.isOpen
              ToolTip.text: qsTr("Restore the project file from this session file")
              onClicked: Cpp_Sessions_Manager.restoreProjectFromDb()
              icon.source: "qrc:/rcc/icons/database/restore.svg"
            }
          }

          Widgets.RibbonSpacer {}
        }
      }

      //
      // Splitter: session list (left) + detail (right)
      //
      Widgets.PaneSplitter {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.topMargin: -1
        settingsKey: "DatabaseExplorer"
        minLeftWidth: 300
        minRightWidth: 360

        leftPanel: Component {
          SessionList {}
        }

        rightPanel: Component {
          SessionDetail {}
        }
      }

    }
  }
}
