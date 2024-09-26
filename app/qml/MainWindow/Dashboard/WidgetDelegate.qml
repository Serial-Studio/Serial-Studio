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
    visible: root.active
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -8
    anchors.rightMargin: -8
    anchors.bottomMargin: -7
    widgetIndex: root.widgetIndex

    Loader {
      anchors.fill: parent
      asynchronous: true
      active: widget.isGpsMap
      visible: widget.isGpsMap && status == Loader.Ready
      sourceComponent: Widgets.GpsMap {
        altitude: widget.gpsAltitude
        latitude: widget.gpsLatitude
        longitude: widget.gpsLongitude
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
      Component.onCompleted: {
        root.flags = Qt.Dialog |
            Qt.WindowTitleHint |
            Qt.WindowStaysOnTopHint |
            Qt.WindowCloseButtonHint

        Cpp_NativeWindow.addWindow(window)
      }

      onClosing: {
        Cpp_NativeWindow.removeWindow(window)
        windowLoader.active = false
      }

      //
      // Background + window title on macOS
      //
      Rectangle {
        anchors.fill: parent
        color: Cpp_ThemeManager.colors["widget_base"]

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
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(13, true)
          visible: Cpp_NativeWindow.titlebarHeight(window) > 0

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
        anchors.topMargin: Cpp_NativeWindow.titlebarHeight(window)

        Loader {
          anchors.fill: parent
          asynchronous: true
          active: windowWidget.isGpsMap
          visible: windowWidget.isGpsMap && status == Loader.Ready
          sourceComponent: Widgets.GpsMap {
            altitude: windowWidget.gpsAltitude
            latitude: windowWidget.gpsLatitude
            longitude: windowWidget.gpsLongitude
          }
        }
      }
    }
  }
}
