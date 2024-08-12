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

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  title: qsTr("Dashboard")
  icon: "qrc:/rcc/icons/panes/dashboard.svg"

  //
  // Hacks for calculating cell width
  //
  property int columns: 4
  readonly property int cellSpacerWidths: Math.max(0, (columns - 1)) * 4
  readonly property int cellWidth: (page.width - 16 - cellSpacerWidths) / columns
  readonly property int cellHeight: cellWidth * (2 / 3)

  //
  // Put everything into a flickable to enable scrolling
  //
  Page {
    id: page
    clip: true
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

    Rectangle {
      anchors.fill: parent
      color: Cpp_ThemeManager.colors["dashboard_background"]
    }

    Flickable {
      anchors.fill: parent
      contentWidth: width
      anchors.topMargin: 8
      anchors.leftMargin: 8
      anchors.bottomMargin: 8
      contentHeight: grid.height

      ScrollBar.vertical: ScrollBar {
        id: scroll
      }

      Grid {
        id: grid
        rowSpacing: 4
        columnSpacing: 4
        width: parent.width
        columns: root.columns
        height: childrenRect.height

        Timer {
          id: timer
          interval: 200
          onTriggered: transition.enabled = false
        }

        Connections {
          target: Cpp_UI_Dashboard
          function onWidgetVisibilityChanged() {
            transition.enabled = true
            timer.start()
          }
        }

        move: Transition {
          id: transition
          enabled: false

          NumberAnimation {
            duration: 200
            properties: "x,y"
            easing.type: Easing.OutSine
          }
        }

        WidgetModel {
          id: model
          cellWidth: root.cellWidth
          cellHeight: root.cellHeight
          model: Cpp_UI_Dashboard.totalWidgetCount
        }
      }
    }
  }
}
