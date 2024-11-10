/*
 * Copyright (c) 2021-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
  onActionButtonClicked: windowLoader.active = true

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

    sourceComponent: Window {
      id: window
      width: 640
      height: 480
      visible: false
      minimumWidth: 640 / 2
      minimumHeight: 480 / 2
      title: widget.widgetTitle

      //
      // Make window stay on top
      //
      Component.onCompleted: {
        window.flags = Qt.Dialog |
            Qt.WindowTitleHint |
            Qt.WindowCloseButtonHint
      }

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
