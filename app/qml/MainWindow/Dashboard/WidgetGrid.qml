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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
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
      color: Cpp_ThemeManager.colors["groupbox_background"]
      height: visible ? actionsLayout.implicitHeight + 12 : 0
      visible: Cpp_UI_Dashboard.actionCount > 0 && !Cpp_CSV_Player.isOpen

      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      RowLayout {
        id: actionsLayout
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
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            text: Cpp_UI_Dashboard.actionTitles[index]
            icon.source: Cpp_UI_Dashboard.actionIcons[index]
            onClicked: Cpp_UI_Dashboard.activateAction(index)
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
      contentHeight: grid.height
      anchors.topMargin: header.height + 8

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
