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
  property int appliedColumns: Cpp_UI_Dashboard.totalWidgetCount === 1 ? 1 : Math.min(columns, Math.max(1, Math.floor((page.width - 16) / 140)))
  readonly property int cellSpacerWidths: Math.max(0, (appliedColumns - 1)) * 4
  readonly property int cellWidth: Math.max(140, (page.width - 16 - cellSpacerWidths) / appliedColumns)
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

    //
    // Background
    //
    Rectangle {
      anchors.fill: parent
      color: Cpp_ThemeManager.colors["dashboard_background"]
    }

    //
    // Actions
    //
    Rectangle {
      z: 2
      id: header
      height: visible ? 64 : 0
      visible: Cpp_UI_Dashboard.actionCount > 0
      color: Cpp_ThemeManager.colors["groupbox_background"]
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      RowLayout {
        spacing: 4

        anchors {
          margins: 8
          left: parent.left
          right: parent.right
          verticalCenter: parent.verticalCenter
        }

        Repeater {
          model: Cpp_UI_Dashboard.actionCount
          delegate: Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            Layout.alignment: Qt.AlignVCenter
            text: Cpp_UI_Dashboard.actionTitles[index]
            icon.source: Cpp_UI_Dashboard.actionIcons[index]
            onClicked: Cpp_UI_Dashboard.activateAction(index)
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
          }
        }

        Item {
          Layout.fillWidth: true
        }
      }

      Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }
    }

    //
    // Widgets
    //
    Flickable {
      id: flickable
      anchors.fill: parent
      contentWidth: width
      anchors.leftMargin: 8
      anchors.bottomMargin: 8
      anchors.topMargin: header.height + 8
      contentHeight: grid.height

      ScrollBar.vertical: ScrollBar {
        id: scroll
      }

      //
      // Viewport size and position tracking
      //
      onWidthChanged: updateVisibleWidgets()
      onHeightChanged: updateVisibleWidgets()
      onContentXChanged: updateVisibleWidgets()
      onContentYChanged: updateVisibleWidgets()

      //
      // Only render pixmaps for widgets that are inside the viewport
      //
      function updateVisibleWidgets() {
        const viewportTop = contentY
        const viewportLeft = contentX
        const viewportRight = contentX + width
        const viewportBottom = contentY + height

        // Notify the model to update the visibility of each widget
        for (let i = 0; i < grid.children.length; i++) {
          let item = grid.children[i]
          let itemTop = item.y
          let itemLeft = item.x
          let itemRight = item.x + cellWidth
          let itemBottom = item.y + cellHeight

          // Check if the item is within the viewport bounds
          let isVisibleVertically = (itemBottom > viewportTop) && (itemTop < viewportBottom)
          let isVisibleHorizontally = (itemRight > viewportLeft) && (itemLeft < viewportRight)

          // Update visibility based on position in the viewport
          item.opacity = isVisibleVertically && isVisibleHorizontally ? 1 : 0
        }
      }

      //
      // Obtain which widgets to render when user changes widget sizes
      //
      Connections {
        target: root
        function onColumnsChanged() {
          const contentX = flickable.contentX
          flickable.contentX = -1
          flickable.contentX = contentX
        }
      }

      Grid {
        id: grid
        rowSpacing: 4
        columnSpacing: 4
        width: parent.width
        height: childrenRect.height
        columns: root.appliedColumns

        Timer {
          id: timer
          interval: 200
          onTriggered: {
            transition.enabled = false
            flickable.updateVisibleWidgets()
          }
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
