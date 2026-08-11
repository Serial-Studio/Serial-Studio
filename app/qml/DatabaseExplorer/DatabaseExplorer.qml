/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets
import "../Commands" as Commands

Widgets.SmartWindow {
  id: root

  //
  // Window geometry & title. The width floor tracks the detail pane's own
  // minimum so the action buttons can never be resized out of view.
  //
  property real detailPaneMinWidth: 360
  minimumWidth: Math.max(880, 300 + 12 + detailPaneMinWidth)
  minimumHeight: 560
  category: "DatabaseExplorer"
  property alias preferredWidth: layout.implicitWidth
  property alias preferredHeight: layout.implicitHeight
  title: Cpp_Sessions_Manager.isOpen
         ? Cpp_Sessions_Manager.fileName
         : qsTr("Sessions")

  //
  // Operator mode pins the explorer to the project DB and hides destructive controls
  //
  readonly property bool operatorMode: typeof app !== "undefined" && app.runtimeMode

  //
  // Database busy indicator
  //
  readonly property bool busy: Cpp_Sessions_Manager.busy || (Cpp_CommercialBuild && Cpp_Sessions_Player.loading)

  //
  // Native window management: author mode uses integrated titlebar; operator mode keeps OS chrome
  //
  onVisibleChanged: {
    if (visible && !root.operatorMode)
      Cpp_NativeWindow.addWindow(root)
    else
      Cpp_NativeWindow.removeWindow(root)
  }

  //
  // Load database when window opens
  //
  Component.onCompleted: Qt.callLater(Cpp_Sessions_Manager.restoreLastDatabase)

  //
  // Report options dialog
  //
  ReportOptionsDialog {
    id: _reportDialog
  }

  //
  // Progress dialog, auto-opens on the pdfExportBusy signal, closes when done
  //
  ReportProgressDialog {
    id: _reportProgressDialog
  }

  //
  // User interface controls
  //
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
      // Toolbar: hidden in operator mode (deployed shortcut) so the explorer
      // becomes a read-only viewer using the standard OS window chrome.
      //
      Rectangle {
        id: toolbar

        z: 2
        Layout.fillWidth: true
        Layout.minimumWidth: 720
        visible: !root.operatorMode
        Layout.minimumHeight: visible ? titlebarHeight + 80 : 0
        Layout.maximumHeight: visible ? titlebarHeight + 80 : 0

        //
        // Display window title on macOS
        //
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
        DatabaseCommandBindings {
          id: _dbBindings

          reportDialog: _reportDialog
        }

        Commands.CommandModel {
          id: _dbModel

          context: "database"
          bindingSets: [_dbBindings]
        }

        Widgets.CommandToolbar {
          height: 80
          model: _dbModel
          surface: "database-toolbar"
          anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: toolbar.titlebarHeight / 2
          }
        }
      }

      //
      // Splitter: session list (left) + detail (right)
      //
      Item {
        Layout.topMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true

        Widgets.PaneSplitter {
          minLeftWidth: 300
          anchors.fill: parent
          settingsKey: "DatabaseExplorer"
          minRightWidth: root.detailPaneMinWidth
          captionSeparatorVisible: !root.operatorMode

          leftPanel: Component {
            SessionList {}
          }

          rightPanel: Component {
            SessionDetail {
              id: detailPane

              Binding {
                target: root
                property: "detailPaneMinWidth"
                value: Math.max(360, detailPane.minimumUsableWidth)
              }
            }
          }
        }

        //
        // Busy overlay covers the splitter while the worker thread is loading
        //
        Rectangle {
          id: busyOverlay

          z: 100
          visible: root.busy
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["window"]

          MouseArea {
            anchors.fill: parent
            preventStealing: true
            cursorShape: Qt.WaitCursor
            acceptedButtons: Qt.AllButtons
            onWheel: function(w) { w.accepted = true }
          }

          ColumnLayout {
            spacing: 12
            anchors.centerIn: parent

            BusyIndicator {
              Layout.preferredWidth: 48
              Layout.preferredHeight: 48
              running: busyOverlay.visible
              Layout.alignment: Qt.AlignHCenter
            }

            Label {
              Layout.alignment: Qt.AlignHCenter
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.uiFont
              text: (Cpp_CommercialBuild && Cpp_Sessions_Player.loading)
                    ? qsTr("Loading session…")
                    : qsTr("Working…")
            }
          }
        }
      }
    }
  }
}
