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
  onActionButtonClicked: window.showNormal()
  buttonIcon: "qrc:/rcc/icons/buttons/expand.svg"

  property bool active: true
  property int widgetIndex: -1

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

  Window {
    id: window
    width: 640
    height: 480
    visible: false
    minimumWidth: 640 / 2
    minimumHeight: 480 / 2
    title: widget.widgetTitle
    Component.onCompleted: {
      window.flags = Qt.Dialog |
          Qt.WindowTitleHint |
          Qt.WindowStaysOnTopHint |
          Qt.WindowCloseButtonHint
    }

    Rectangle {
      anchors.fill: parent
      color: Cpp_ThemeManager.colors["widget_base"]
    }

    DashboardWidget {
      id: windowWidget
      anchors.margins: 4
      anchors.fill: parent
      visible: window.visible
      widgetIndex: root.widgetIndex

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
