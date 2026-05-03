/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Item {
  id: root

  //
  // Custom input properties
  //
  required property SS_Ui.TaskBar taskBar
  property var taskbarView: null
  onTaskBarChanged: {
    taskBar.windowManager = _wm
  }

  //
  // Window manager access
  //
  property alias windowManager: _wm
  property alias backgroundImage: _wm.backgroundImage

  //
  // Desktop context menu
  //
  Menu {
    id: contextMenu

    MenuItem {
      text: qsTr("Set Wallpaper…")
      onTriggered: _wm.selectBackgroundImage()
    }

    MenuItem {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Clear Wallpaper")
      onTriggered: _wm.clearBackgroundImage()
      enabled: _wm.backgroundImage.length > 0
    }

    MenuSeparator {

    }

    MenuItem {
      text: qsTr("Tile Windows")
      onTriggered: _wm.autoLayout()
    }
  }

  //
  // Commercial features notification
  //
  Widgets.ProNotice {
    id: commercialNotification

    anchors {
      margins: -1
      top: parent.top
      left: parent.left
      right: parent.right
    }

    z: 2
    titleText: qsTr("Pro features detected in this project.")
    activationFlag: Cpp_UI_Dashboard.containsCommercialFeatures
    subtitleText: qsTr("Fallback widgets are active. Purchase a license for full functionality.")
  }

  //
  // Window canvas
  //
  Item {
    id: windowCanvas

    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      topMargin: commercialNotification.visible ? -1 : 0
      top: commercialNotification.visible ? commercialNotification.bottom :
                                            parent.top
    }

    //
    // Set window manager
    //
    SS_Ui.WindowManager {
      id: _wm

      anchors.fill: parent
      onRightClicked: (x, y) => contextMenu.popup(x, y)
    }

    //
    // Initialize widget windows
    //
    Instantiator {
      id: loader

      model: taskBar.taskbarButtons

      delegate: WidgetDelegate {
        required property var model

        windowManager: _wm
        parent: windowCanvas
        taskBar: root.taskBar
        title: model.widgetName
        widgetIndex: model.windowId

        Component.onDestruction: {
          if (root.taskBar) {
            root.taskBar.unregisterWindow(this)
            if (root.taskBar.activeWindow === this)
              root.taskBar.activeWindow = null
          }
        }

        Component.onCompleted: root.taskBar.registerWindow(widgetIndex, this)
      }
    }

    //
    // Snap indicator
    //
    Rectangle {
      border.width: 1
      z: _wm.zCounter
      x: _wm.snapIndicator.x
      y: _wm.snapIndicator.y
      width: _wm.snapIndicator.width
      height: _wm.snapIndicator.height
      visible: _wm.snapIndicatorVisible
      color: Cpp_ThemeManager.colors["snap_indicator_background"]
      border.color: Cpp_ThemeManager.colors["snap_indicator_border"]
    }

    //
    // Empty workspace placeholder
    //
    Item {
      z: _wm.zCounter + 20
      anchors.fill: parent
      visible: taskBar.activeGroupId >= 1000
               && taskBar.taskbarButtons.rowCount() === 0

      ColumnLayout {
        spacing: 16
        anchors.centerIn: parent

        Image {
          opacity: 0.4
          sourceSize: Qt.size(64, 64)
          Layout.alignment: Qt.AlignHCenter
          source: "qrc:/icons/panes/dashboard.svg"
        }

        Label {
          opacity: 0.8
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("Empty Workspace")
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
          font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
        }

        Label {
          opacity: 0.5
          wrapMode: Text.WordWrap
          Layout.maximumWidth: 300
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
          text: qsTr("Use the search bar to find and add widgets, "
                    + "or right-click a widget in another workspace "
                    + "to add it here.")
        }

        Item {
          implicitHeight: 4
        }

        Button {
          topPadding: 8
          leftPadding: 16
          bottomPadding: 8
          rightPadding: 20
          text: qsTr("Search Widgets")
          Layout.alignment: Qt.AlignHCenter
          icon.source: "qrc:/icons/buttons/search.svg"
          onClicked: {
            if (root.taskbarView)
              root.taskbarView.focusSearch()
          }
        }
      }
    }
  }
}
