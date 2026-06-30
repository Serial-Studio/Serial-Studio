/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "Sections" as Sections
import "../Widgets" as Widgets
import "../Dialogs" as Dialogs

Widgets.SmartWindow {
  id: root

  category: "ProjectEditor"
  minimumWidth: layout.implicitWidth + 32
  minimumHeight: layout.implicitHeight + 32
  property alias preferredWidth: layout.implicitWidth
  property alias preferredHeight: layout.implicitHeight
  title: Cpp_JSON_ProjectModel.title + (Cpp_JSON_ProjectModel.modified ? " (" + qsTr("modified") + ")" : "")

  //
  // Ask user to save changes when closing the dialog
  //
  onClosing: (close) => close.accepted = Cpp_JSON_ProjectModel.askSave()

  //
  // Ensure that current JSON file is shown
  //
  onVisibleChanged: {
    if (visible) {
      Cpp_NativeWindow.addWindow(root)
      Cpp_JSON_ProjectModel.openJsonFile(Cpp_AppState.projectFilePath)
    }

    else {
      Cpp_NativeWindow.removeWindow(root)
      actionIconPicker.close()
      onlineIconPicker.close()
    }
  }

  //
  // Icon picker dialogs (owned here so transientParent is the ProjectEditor window).
  //
  Dialogs.IconPicker {
    id: actionIconPicker
  }

  Dialogs.OnlineIconPicker {
    id: onlineIconPicker

    onIconSelected: function(icon) {
      actionIconPicker.iconSelected(icon)
    }
  }

  //
  // Load project file on start
  //
  Component.onCompleted: Cpp_JSON_ProjectModel.openJsonFile(Cpp_AppState.projectFilePath)

  //
  // Backup recovery dialog (parented here so transientParent is the ProjectEditor window).
  //
  DialogLoader {
    id: backupRecoveryDialog

    source: "qrc:/serial-studio.com/gui/qml/Dialogs/BackupRecovery.qml"
  }

  function showBackupRecovery() {
    backupRecoveryDialog.activate()
  }

  //
  // Frame parser test dialog (hosted here so source edits can't destroy the window)
  //
  DialogLoader {
    id: parserTestLoader

    property int requestedSourceId: 0

    source: "qrc:/serial-studio.com/gui/qml/Dialogs/FrameParserTest.qml"
    onLoaded: item.openForSource(requestedSourceId)

    function openTester(sourceId) {
      requestedSourceId = sourceId
      if (active && dialog)
        dialog.openForSource(sourceId)
      else
        activate()
    }
  }

  //
  // Shortcuts
  //
  Shortcut {
    sequences: [StandardKey.Open]
    onActivated: Cpp_JSON_ProjectModel.openJsonFile()
  } Shortcut {
    sequences: [StandardKey.New]
    onActivated: Cpp_JSON_ProjectModel.newJsonFile()
  } Shortcut {
    sequences: [StandardKey.Save]
    onActivated: Cpp_JSON_ProjectModel.saveJsonFile()
  } Shortcut {
    sequences: [StandardKey.Quit]
    onActivated: app.quitApplication()
  }

  //
  // Use page item to set application palette
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

    //
    // Always rendered so lock/mode overlays have something to blur; disabled when locked.
    //
    ColumnLayout {
      id: layout

      spacing: 0
      anchors.fill: parent
      enabled: Cpp_AppState.operationMode === SerialStudio.ProjectFile
               && !Cpp_JSON_ProjectModel.locked

      //
      // Toolbar
      //
      Sections.ProjectToolbar {
        id: toolbar

        z: 2
        Layout.fillWidth: true
        Layout.minimumWidth: 860
      }

      //
      // Main Layout: kept visible when locked; the lock overlay sits on top
      //
      Widgets.PaneSplitter {
        id: splitter

        minLeftWidth: 256
        minRightWidth: 400
        Layout.topMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 480
        settingsKey: "ProjectEditor"

        leftPanel: Component {
          Sections.ProjectStructure {
            id: projectStructure
          }
        }

        rightPanel: Component {
          //
          // Only the active view loads; tab switches destroy and reinstantiate.
          //
          Loader {
            asynchronous: false
            anchors.fill: parent
            source: {
              switch (Cpp_JSON_ProjectEditor.currentView) {
              case ProjectEditor.ActionView:            return "Views/ActionView.qml"
              case ProjectEditor.ProjectView:           return "Views/ProjectView.qml"
              case ProjectEditor.GroupView:             return "Views/GroupView.qml"
              case ProjectEditor.DatasetView:           return "Views/DatasetView.qml"
              case ProjectEditor.SourceView:            return "Views/SourceView.qml"
              case ProjectEditor.SourceFrameParserView: return "Views/SourceFrameParserView.qml"
              case ProjectEditor.OutputWidgetView:      return "Views/OutputWidgetView.qml"
              case ProjectEditor.DataTablesView:        return "Views/DataTablesView.qml"
              case ProjectEditor.SystemDatasetsView:    return "Views/SystemDatasetsView.qml"
              case ProjectEditor.UserTableView:         return "Views/UserTableView.qml"
              case ProjectEditor.WorkspacesView:        return "Views/WorkspacesView.qml"
              case ProjectEditor.WorkspaceView:         return "Views/WorkspaceView.qml"
              case ProjectEditor.WorkspaceFolderView:   return "Views/WorkspaceFolderView.qml"
              case ProjectEditor.GroupsView:            return "Views/GroupsView.qml"
              case ProjectEditor.GroupFolderView:       return "Views/GroupFolderView.qml"
              case ProjectEditor.TableFolderView:       return "Views/TableFolderView.qml"
              case ProjectEditor.MqttPublisherView:     return "Views/MqttPublisherView.qml"
              case ProjectEditor.ControlScriptView:     return "Views/ControlScriptView.qml"
              }
              return ""
            }
          }
        }
      }
    }

    //
    // Lock + project mode-required overlay
    //
    Item {
      id: editorOverlay

      z: 3
      anchors.fill: parent
      visible: Cpp_JSON_ProjectModel.locked
               || Cpp_AppState.operationMode !== SerialStudio.ProjectFile

      readonly property bool lockMode: Cpp_JSON_ProjectModel.locked
                                       && Cpp_AppState.operationMode === SerialStudio.ProjectFile

      //
      // Blur effect
      //
      MultiEffect {
        blur: 1.0
        blurMax: 64
        source: layout
        blurEnabled: true
        anchors.fill: parent
        autoPaddingEnabled: false
        enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
        visible: Cpp_Misc_GraphicsBackend.effectsEnabled
      }

      //
      // Opaque fallback when the scene graph has no RHI (MultiEffect is a no-op there)
      //
      Rectangle {
        anchors.fill: parent
        color: Cpp_ThemeManager.colors["toolbar_top"]
        visible: !Cpp_Misc_GraphicsBackend.effectsEnabled
      }

      //
      // Swallow clicks/wheel so the toolbar/splitter underneath stay quiet
      //
      MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        onWheel: function(wheel) { wheel.accepted = true }
      }

      //
      // Preserve native-style window drag on the opaque top strip. The
      // toolbar's own DragHandler is unreachable while the overlay is up
      //
      DragHandler {
        target: null
        onActiveChanged: {
          if (active)
            projectEditor.startSystemMove()
        }
      }

      //
      // Subtle gradient to transition into the window caption
      //
      Rectangle {
        height: toolbar.height
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
        }

        gradient: Gradient {
          GradientStop {
            position: 0.0
            color: Cpp_ThemeManager.colors["toolbar_top"]
          }

          GradientStop {
            position: 0.5
            color: "transparent"
          }
        }
      }

      //
      // Software-mode card backdrop sized to the call-to-action column
      //
      Rectangle {
        radius: 6
        border.width: 1
        anchors.fill: cta
        anchors.margins: -24
        color: Cpp_ThemeManager.colors["widget_base"]
        border.color: Cpp_ThemeManager.colors["widget_border"]
        visible: !Cpp_Misc_GraphicsBackend.effectsEnabled
      }

      //
      // Centered lock/mode call-to-action; offset to align with the perceived editor body.
      //
      ColumnLayout {
        id: cta

        spacing: 16
        anchors.centerIn: parent
        width: Math.min(parent.width - 96, 520)
        anchors.verticalCenterOffset: toolbar.titlebarHeight / 2

        Image {
          sourceSize: Qt.size(144, 144)
          Layout.alignment: Qt.AlignHCenter
          source: editorOverlay.lockMode
                  ? "qrc:/images/locked.svg"
                  : "qrc:/images/project-mode-required.svg"
        }

        Item {
          implicitHeight: 4
        }

        Label {
          wrapMode: Label.WordWrap
          Layout.alignment: Qt.AlignHCenter
          Layout.fillWidth: true
          horizontalAlignment: Label.AlignHCenter
          font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
          text: editorOverlay.lockMode
                ? qsTr("This project is password protected")
                : qsTr("Editing is available in Project mode")
        }

        Label {
          opacity: 0.8
          wrapMode: Label.WordWrap
          Layout.alignment: Qt.AlignHCenter
          Layout.fillWidth: true
          horizontalAlignment: Label.AlignHCenter
          font: Cpp_Misc_CommonFonts.customUiFont(1.2, false)
          text: editorOverlay.lockMode
                ? qsTr("Enter the password to make changes, or open a different project.")
                : qsTr("Switch to Project mode to load and edit a project.")
        }

        Item {
          implicitHeight: 8
        }

        RowLayout {
          spacing: 8
          Layout.alignment: Qt.AlignHCenter

          Widgets.IconButton {
            highlighted: true
            topPadding: 8
            bottomPadding: 8
            leftPadding: 16
            rightPadding: 12
            icon.color: palette.buttonText
            icon.source: editorOverlay.lockMode
                         ? "qrc:/icons/buttons/unlock.svg"
                         : "qrc:/icons/buttons/switch.svg"
            text: editorOverlay.lockMode
                  ? qsTr("Unlock")
                  : qsTr("Switch to Project Mode")
            onClicked: {
              if (editorOverlay.lockMode)
                Cpp_JSON_ProjectModel.unlockProject()
              else
                Cpp_AppState.operationMode = SerialStudio.ProjectFile
            }
          }

          Widgets.IconButton {
            topPadding: 8
            bottomPadding: 8
            leftPadding: 16
            rightPadding: 16
            icon.color: palette.buttonText
            icon.source: editorOverlay.lockMode
                         ? "qrc:/icons/buttons/open.svg"
                         : "qrc:/icons/buttons/close.svg"
            text: editorOverlay.lockMode
                  ? qsTr("Open Other Project")
                  : qsTr("Close")
            onClicked: {
              if (editorOverlay.lockMode)
                Cpp_JSON_ProjectModel.openJsonFile()
              else
                root.close()
            }
          }

          Widgets.IconButton {
            topPadding: 8
            leftPadding: 16
            bottomPadding: 8
            rightPadding: 16
            icon.color: palette.buttonText
            visible: editorOverlay.lockMode
            text: qsTr("Create New Project")
            icon.source: "qrc:/icons/buttons/new.svg"
            onClicked: Cpp_JSON_ProjectModel.newJsonFile()
          }
        }
      }
    }
  }
}
