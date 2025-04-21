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

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  hardBorder: true
  icon: widget.widgetIcon
  title: widget.widgetTitle

  //
  // Create an external window when user clicks on the action button of the pane
  //
  buttonIcon: "qrc:/rcc/icons/buttons/expand.svg"
  onActionButtonClicked: windowLoader.activate()

  //
  // Disable widget rendering when it is not visible on the user's screen
  //
  property bool active: true

  //
  // Used to keep track of which C++ widget to render
  //
  property int widgetIndex: -1

  //
  // Render a widget inside the pane
  //
  DashboardWidget {
    id: widget
    clip: true
    visible: root.active
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -8
    anchors.rightMargin: -8
    anchors.bottomMargin: -7
    widgetIndex: root.widgetIndex

    Rectangle {
      anchors.fill: parent
      color: Cpp_ThemeManager.colors["widget_window"]
    }

    Loader {
      id: loader
      asynchronous: true
      anchors.fill: parent
      source: widget.widgetQmlPath
      onLoaded: {
        if (item && widget.widgetModel) {
          item.color = widget.widgetColor
          item.model = widget.widgetModel
        }
      }

      Connections {
        target: Cpp_ThemeManager

        function onThemeChanged() {
          if (loader.item !== null)
            loader.item.color = widget.widgetColor
        }
      }
    }
  }

  //
  // Loader for the external window
  //
  Loader {
    id: windowLoader
    active: false
    asynchronous: true
    onLoaded: windowLoader.item.showNormal()

    property Window window: null

    function activate() {
      if (!active)
        active = true

      else if (window) {
        window.raise()
        window.requestActivate()
      }

      else {
        active = false
        active = true
      }
    }

    sourceComponent: Component {
      Window {
        id: window
        width: 640
        height: 480
        visible: false
        minimumWidth: 640 / 2
        minimumHeight: 480 / 2
        title: widget.widgetTitle
        Component.onCompleted: windowLoader.window = this

        //
        // Close window instead of app
        //
        Shortcut {
          sequences: [StandardKey.Close]
          onActivated: window.close()
        }

        //
        // Destroy widget on close
        //
        onClosing: {
          Cpp_NativeWindow.removeWindow(window)
          windowLoader.window = null
          windowLoader.active = false
        }

        //
        // Native window registration
        //
        property real titlebarHeight: 0
        onVisibleChanged: {
          if (visible) {
            Cpp_NativeWindow.addWindow(window, Cpp_ThemeManager.colors["widget_window"])
            window.titlebarHeight = Cpp_NativeWindow.titlebarHeight(window)
          }
        }

        //
        // Background + window title on macOS
        //
        Rectangle {
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["widget_window"]

          //
          // Drag the window anywhere
          //
          DragHandler {
            target: null
            onActiveChanged: {
              if (active)
                window.startSystemMove()
            }
          }

          //
          // Titlebar text
          //
          Label {
            text: window.title
            visible: window.titlebarHeight > 0
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

            anchors {
              topMargin: 6
              top: parent.top
              horizontalCenter: parent.horizontalCenter
            }
          }
        }

        //
        // Dashboard widget item
        //
        DashboardWidget {
          id: windowWidget
          anchors.margins: 4
          anchors.fill: parent
          visible: window.visible
          widgetIndex: root.widgetIndex
          anchors.topMargin: window.titlebarHeight

          Loader {
            id: windowWidgetLoader
            asynchronous: true
            anchors.fill: parent
            source: windowWidget.widgetQmlPath
            onLoaded: {
              if (item && windowWidget.widgetModel) {
                item.color = windowWidget.widgetColor
                item.model = windowWidget.widgetModel
              }
            }

            Connections {
              target: Cpp_ThemeManager

              function onThemeChanged() {
                if (windowWidgetLoader.item !== null)
                  windowWidgetLoader.item.color = windowWidget.widgetColor
              }
            }
          }
        }
      }
    }
  }
}
