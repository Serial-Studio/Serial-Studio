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

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Item {
  id: root

  //
  // Custom input properties
  //
  required property SS_Ui.TaskBar taskBar
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
      text: qsTr("Set Wallpaper...")
      onTriggered: _wm.selectBackgroundImage()
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
    z: 2
    id: commercialNotification

    activationFlag: Cpp_UI_Dashboard.containsCommercialFeatures
    titleText: qsTr("Pro features detected in this project.")
    subtitleText: qsTr("Fallback widgets are active. Purchase a license for full functionality.")

    anchors {
      margins: -1
      top: parent.top
      left: parent.left
      right: parent.right
    }
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
          root.taskBar.unregisterWindow(this)
          if (root.taskBar.activeWindow === this)
            root.taskBar.activeWindow = null
        }

        Component.onCompleted: {
          root.taskBar.registerWindow(widgetIndex, this)
          if (widgetIndex === 0)
            root.taskBar.activeWindow = this
        }
      }
    }
  }
}
