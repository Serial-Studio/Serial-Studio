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
import QtGraphs
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  //
  // Plot properties
  //
  property alias graph: _graph
  property alias xAxis: _axisX
  property alias yAxis: _axisY
  property alias xMin: _axisX.min
  property alias xMax: _axisX.max
  property alias yMin: _axisY.min
  property alias yMax: _axisY.max
  property alias yLabel: _yLabel.text
  property alias xLabel: _xLabel.text
  property alias curveColors: _theme.seriesColors

  //
  // Set axis visibility based on user options and/or widget size
  //
  readonly property bool yLabelVisible: root.height >= 120 && (Cpp_UI_Dashboard.axisVisibility & SerialStudio.AxisY)
  readonly property bool xLabelVisible: root.height >= 120 && (Cpp_UI_Dashboard.axisVisibility & SerialStudio.AxisX)

  //
  // Updates the X and Y value labels to reflect the world coordinates under
  // the mouse cursor.
  //
  function updateCrosshairLabels(mouseX, mouseY) {
    // Only update if mouse is inside the tracking area
    if (!_overlayMouse.containsMouse)
      return

    // X axis calculation
    const xFullRange = _axisX.max - _axisX.min
    const xWorldCenter = _axisX.min + xFullRange / 2
    const xVisibleRange = xFullRange / _axisX.zoom
    const xViewStart = xWorldCenter + _axisX.pan - xVisibleRange / 2
    const x = xViewStart + xVisibleRange * (mouseX / _graph.plotArea.width)

    // Y axis calculation
    const yFullRange = _axisY.max - _axisY.min
    const yWorldCenter = _axisY.min + yFullRange / 2
    const yVisibleRange = yFullRange / _axisY.zoom
    const yViewStart = yWorldCenter + _axisY.pan - yVisibleRange / 2
    const y = yViewStart + yVisibleRange * (1 - (mouseY / _graph.plotArea.height))

    // Update labels
    _xPosLabel.text = "x = " + x.toFixed(2)
    _yPosLabel.text = "y = " + y.toFixed(2)
  }

  //
  // Applies zoom centered on the cursor position and adjusts pan to keep the
  // same world point under the cursor, this results in a behavior similar
  // to zooming in CAD programs...
  //
  function applyCursorZoom(axis, oldZoom, newZoom, cursorPos, axisLength, inverted) {
    // Ensure that zoom level stays limited
    const minZoom = 1
    const maxZoom = 100
    const clampedZoom = Math.max(minZoom, Math.min(maxZoom, newZoom))

    // Reset to default view when zoom reaches minimum
    if (clampedZoom === 1) {
      axis.pan = 0
      axis.zoom = 1
      return
    }

    // Skip if there's no effective zoom change
    if (oldZoom === clampedZoom)
      return

    // // Calculate the center of the full axis range in world coordinates
    const fullRange = axis.max - axis.min
    const worldCenter = axis.min + fullRange / 2

    // Convert cursor position (in pixels) to ratio along axis (0..1)
    const cursorRatio = inverted
                      ? (1 - cursorPos / axisLength)
                      : (cursorPos / axisLength)

    // Get world coordinate under cursor before zoom
    const visibleRangeBefore = fullRange / oldZoom
    const visibleStartBefore = worldCenter + axis.pan - visibleRangeBefore / 2
    const worldUnderCursor = visibleStartBefore + cursorRatio * visibleRangeBefore

    // Apply new zoom level
    axis.zoom = clampedZoom

    // Calculate view window so that worldUnderCursor stays in the same position
    const visibleRangeAfter = fullRange / clampedZoom
    const newVisibleStart = worldUnderCursor - cursorRatio * visibleRangeAfter
    const newCenter = newVisibleStart + visibleRangeAfter / 2
    const newPan = newCenter - worldCenter

    // Update pan to match new center
    axis.pan = newPan
  }

  //
  // Plot widget
  //
  GraphsView {
    id: _graph
    anchors {
      top: parent.top
      right: parent.right
      topMargin: root.yLabelVisible ? 4 : 0
      leftMargin: root.yLabelVisible ? 2 : 0
      rightMargin: root.xLabelVisible ? 4 : 0
      bottomMargin: root.xLabelVisible ? 2 : 0
      left: root.yLabelVisible ? _yLabelContainer.right : parent.left
      bottom: root.xLabelVisible ? _xLabelContainer.top : parent.bottom
    }

    //
    // Modify the margins depending on axis visibilty
    //
    marginTop: root.yLabelVisible ? 8 : 0
    marginRight: root.xLabelVisible ? 8 : 0
    marginLeft: root.yLabelVisible ? -8 : (root.xLabelVisible ? -40 : -60)
    marginBottom: root.xLabelVisible ? -8 : (root.yLabelVisible ? -20 : -40)

    //
    // Set plot colors
    //
    theme: GraphsTheme {
      id: _theme

      // Background
      plotAreaBackgroundVisible: true
      theme: GraphsTheme.Theme.UserDefined
      borderColors: [Cpp_ThemeManager.colors["widget_border"]]
      backgroundColor: Cpp_ThemeManager.colors["widget_window"]
      plotAreaBackgroundColor: Cpp_ThemeManager.colors["widget_base"]

      // Axis and grid colors
      axisX.subWidth: 1
      axisY.subWidth: 1
      axisX.mainWidth: 1
      axisY.mainWidth: 1
      axisX.subColor: Cpp_ThemeManager.colors["widget_border"]
      axisY.subColor: Cpp_ThemeManager.colors["widget_border"]
      axisX.mainColor: Cpp_ThemeManager.colors["widget_border"]
      axisY.mainColor: Cpp_ThemeManager.colors["widget_border"]
      axisX.labelTextColor: Cpp_ThemeManager.colors["widget_text"]
      axisY.labelTextColor: Cpp_ThemeManager.colors["widget_text"]

      // Axis label fonts and colors
      labelTextColor: Cpp_ThemeManager.colors["widget_text"]
      axisXLabelFont: Cpp_Misc_CommonFonts.customMonoFont(0.83)
      axisYLabelFont: Cpp_Misc_CommonFonts.customMonoFont(0.83)

      // Grid settings
      grid.subWidth: 1
      grid.mainWidth: 1
      gridVisible: true
      grid.subColor: Cpp_ThemeManager.colors["widget_border"]
      grid.mainColor: Cpp_ThemeManager.colors["widget_border"]

      // Highlight colors for better contrast
      multiHighlightColor: Cpp_ThemeManager.colors["widget_highlight"]
      singleHighlightColor: Cpp_ThemeManager.colors["widget_highlighted_text"]
    }

    //
    // Block pan and zoom, we do it manually
    //
    panStyle: GraphsView.PanStyle.None
    zoomStyle: GraphsView.ZoomStyle.None

    //
    // Customize Y axis
    //
    axisY: ValueAxis {
      id: _axisY
      subTickCount: 1
      tickAnchor: root.yMin
      visible: root.yLabelVisible
    }

    //
    // Customize X axis
    //
    axisX: ValueAxis {
      id: _axisX
      subTickCount: 1
      tickAnchor: root.xMin
      visible: root.xLabelVisible
    }
  }

  //
  // Interactive Overlay: handles crosshairs and CAD-like zooming
  //
  Item {
    width: _graph.plotArea.width
    height: _graph.plotArea.height
    x: _graph.x + _graph.plotArea.x
    y: _graph.y + _graph.plotArea.y
    opacity: _overlayMouse.containsMouse ? 1 : 0

    Behavior on opacity { NumberAnimation {} }

    //
    // MouseArea for interactive graph navigation and inspection
    //
    // Responsibilities:
    // - Tracks mouse position to update crosshair lines and value labels
    // - Flips label anchors to prevent overlap with cursor
    // - Responds to scroll wheel to perform zooming centered on cursor position
    // - Uses `applyCursorZoom` to adjust zoom and pan on both X and Y axes
    // - Uses `updateCrosshairLabels` to reflect current world under cursor
    // - Shows a crosshair cursor when hovering the graph
    //
    MouseArea {
      id: _overlayMouse

      hoverEnabled: true
      anchors.fill: parent
      cursorShape: Qt.CrossCursor

      //
      // Handle mouse wheel zoom interaction:
      // - Compute mouse position relative to plot area
      // - Determine zoom direction (in/out) based on wheel delta
      // - Apply exponential zoom factor to both X and Y axes
      // - Use cursor position to focus zoom toward the mouse location
      // - Update crosshair value labels to reflect new view
      // - Mark the wheel event as handled to prevent propagation
      //
      onWheel: (wheel) => {
        // Abort if not mouse is not in plot
        if (!containsMouse)
          return

        // Obtain X/Y position relative to graph
        const localX = mouseX - _graph.plotArea.x
        const localY = mouseY - _graph.plotArea.y

        // Calculate new zoom factor
        const zoomFactor = 1.15
        const direction = wheel.angleDelta.y > 0 ? -1 : 1
        const factor = Math.pow(zoomFactor, direction)

        // Calculate new zoom values for both axes
        const newZoomX = _axisX.zoom * factor
        const newZoomY = _axisY.zoom * factor

        // Zoom & navigate through the graph
        root.applyCursorZoom(_axisX, _axisX.zoom, newZoomX, localX, _graph.plotArea.width, false)
        root.applyCursorZoom(_axisY, _axisY.zoom, newZoomY, localY, _graph.plotArea.height, true)

        // Update crosshair labels to reflect new view window
        root.updateCrosshairLabels(localX, localY)
        wheel.accepted = true
      }

      //
      // Handle mouse movement over the plot area:
      // - Ignore updates if mouse is not within bounds
      // - Update vertical and horizontal crosshair positions
      // - Repaint crosshair canvases to reflect new position
      // - Dynamically flip label anchors to avoid overlapping the cursor:
      //     - Y label flips left if near the right edge
      //     - X label flips to bottom if near the top edge
      // - Update value labels to reflect current cursor world coordinates
      //
      onPositionChanged: (mouse) => {
        // Abort if not mouse is not in plot
        if (!containsMouse)
          return

        // Obtain graph size
        const graphW = _graph.plotArea.width
        const graphH = _graph.plotArea.height

        // Crosshair position
        _verCrosshair.x = mouse.x
        _horCrosshair.y = mouse.y
        _verCrosshair.requestPaint()
        _horCrosshair.requestPaint()

        // Flip Y label left/right
        if (mouse.x > graphW - _yPosLabel.width) {
          _yPosLabel.anchors.right = undefined
          _yPosLabel.anchors.left = parent.left
        } else {
          _yPosLabel.anchors.left = undefined
          _yPosLabel.anchors.right = parent.right
        }

        // Flip X label top/bottom
        if (mouse.y < _xPosLabel.height) {
          _xPosLabel.anchors.top = undefined
          _xPosLabel.anchors.bottom = parent.bottom
        } else {
          _xPosLabel.anchors.bottom = undefined
          _xPosLabel.anchors.top = parent.top
        }

        // Update values
        updateCrosshairLabels(mouse.x, mouse.y)
      }
    }

    //
    // Vertical crosshair line
    //
    Canvas {
      id: _verCrosshair
      width: 2

      onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = Cpp_ThemeManager.colors["polar_indicator"]
        ctx.setLineDash([4, 4])

        const labelTop = _xPosLabel.y
        const labelBottom = labelTop + _xPosLabel.height

        ctx.beginPath()
        ctx.moveTo(0.5, 0)
        ctx.lineTo(0.5, labelTop)
        ctx.moveTo(1.5, 0)
        ctx.lineTo(1.5, labelTop)
        ctx.moveTo(0.5, labelBottom)
        ctx.lineTo(0.5, height)
        ctx.moveTo(1.5, labelBottom)
        ctx.lineTo(1.5, height)
        ctx.stroke()
      }

      anchors {
        top: parent.top
        bottom: parent.bottom
      }
    }

    //
    // Horizontal crosshair line
    //
    Canvas {
      id: _horCrosshair

      height: 2
      onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = Cpp_ThemeManager.colors["polar_indicator"]
        ctx.setLineDash([4, 4])

        const labelLeft = _yPosLabel.x
        const labelRight = labelLeft + _yPosLabel.width

        ctx.beginPath()
        ctx.moveTo(0, 0.5)
        ctx.lineTo(labelLeft, 0.5)
        ctx.moveTo(0, 1.5)
        ctx.lineTo(labelLeft, 1.5)
        ctx.moveTo(labelRight, 0.5)
        ctx.lineTo(width, 0.5)
        ctx.moveTo(labelRight, 1.5)
        ctx.lineTo(width, 1.5)
        ctx.stroke()
      }

      anchors {
        left: parent.left
        right: parent.right
      }
    }

    //
    // X position label
    //
    Label {
      id: _xPosLabel

      padding: 4
      visible: _overlayMouse.containsMouse
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: Cpp_ThemeManager.colors["polar_indicator"]
      background: Rectangle {
        radius: 2
        opacity: 0.8
        color: Cpp_ThemeManager.colors["widget_base"]
      }

      anchors {
        top: parent.top
        horizontalCenter: _verCrosshair.horizontalCenter
      }
    }

    //
    // Y position label
    //
    Label {
      id: _yPosLabel

      padding: 4
      visible: _overlayMouse.containsMouse
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: Cpp_ThemeManager.colors["polar_indicator"]
      background: Rectangle {
        radius: 2
        opacity: 0.8
        color: Cpp_ThemeManager.colors["widget_base"]
      }

      anchors {
        right: parent.right
        verticalCenter: _horCrosshair.verticalCenter
      }
    }
  }

  //
  // Y-axis label
  //
  Item {
    id: _yLabelContainer
    width: _yLabel.height
    visible: root.yLabelVisible

    anchors {
      top: parent.top
      left: parent.left
      bottom: parent.bottom
    }

    Label {
      id: _yLabel
      rotation: 270
      elide: Qt.ElideRight
      width: parent.height
      anchors.centerIn: parent
      horizontalAlignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.customMonoFont(0.91)
      color: Cpp_ThemeManager.colors["widget_text"]
      anchors.verticalCenterOffset: root.xLabelVisible && _yLabel.implicitWidth <= _graph.height ?
                                      -1 * Math.abs(_graph.marginBottom - _graph.marginTop) : 0
    }
  }

  //
  // X-axis label
  //
  Item {
    id: _xLabelContainer
    height: _xLabel.height
    visible: root.xLabelVisible

    anchors {
      right: parent.right
      bottom: parent.bottom
      left: _yLabelContainer.right
      leftMargin: root.yLabelVisible ? 4 : 0
    }

    Label {
      id: _xLabel
      width: parent.width
      elide: Qt.ElideRight
      anchors.centerIn: parent
      horizontalAlignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.customMonoFont(0.91)
      color: Cpp_ThemeManager.colors["widget_text"]
      anchors.horizontalCenterOffset: root.yLabelVisible ? Math.abs(_graph.marginLeft - _graph.marginRight) : 0
    }
  }

  //
  // Ensure a border is always visible for the plot area
  //
  Rectangle {
    border.width: 1
    color: "transparent"
    width: _graph.plotArea.width
    height: _graph.plotArea.height
    x: _graph.x + _graph.plotArea.x
    y: _graph.y + _graph.plotArea.y
    border.color: Cpp_ThemeManager.colors["widget_border"]
  }
}
