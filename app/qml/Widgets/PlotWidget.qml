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
  property alias zoom: _axisX.zoom
  property alias yLabel: _yLabel.text
  property alias xLabel: _xLabel.text
  property alias plotArea: _graph.plotArea
  property alias curveColors: _theme.seriesColors

  //
  // Custom properties
  //
  property bool xLabelVisible: true
  property bool yLabelVisible: true
  property bool showCrosshairs: false
  property bool mouseAreaEnabled: true

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
    _xPosLabel.text = x.toFixed(2)
    _yPosLabel.text = y.toFixed(2)
  }

  //
  // Translates pixel movement into world units and adjusts the axis pan
  // accordingly. It ensures that the visible window stays within the axis
  // bounds, preventing overscroll.
  //
  function adjustAxisPan(axis, axisLength, cursorPos, dPx, inverted) {
    // Convert pixels to plot units (whatever they might be)
    const fullRange = axis.max - axis.min
    const visibleRange = fullRange / axis.zoom
    const unitPerPixel = fullRange / axisLength

    // Calculate new pan
    const pxDiff = (inverted ? -dPx : dPx)
    const zoomDampeningFactor = 1 / axis.zoom
    let newPan = axis.pan + pxDiff * unitPerPixel * zoomDampeningFactor

    // Clamp the pan so the view doesn't go beyond the axis bounds
    const maxPan = (axis.max - (axis.min + visibleRange)) / 2
    const minPan = (axis.min - (axis.max - visibleRange)) / 2
    newPan = Math.min(Math.max(newPan, minPan), maxPan)

    // Update the axis pan
    axis.pan = newPan
  }

  //
  // Applies zoom centered on the cursor position and adjusts pan to keep the
  // same world point under the cursor, this results in a behavior similar
  // to zooming in CAD programs.
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
    let newPan = newCenter - worldCenter

    // Clamp the pan so the view doesn't go beyond the axis bounds
    const maxPan = (axis.max - (axis.min + visibleRangeAfter)) / 2
    const minPan = (axis.min - (axis.max - visibleRangeAfter)) / 2
    newPan = Math.min(Math.max(newPan, minPan), maxPan)

    // Update pan to match new center
    axis.pan = newPan
  }

  //
  // Plot widget
  //
  GraphsView {
    id: _graph
    anchors {
      margins: 2
      leftMargin: 4
      rightMargin: 4
      top: parent.top
      right: parent.right
      left: _yLabelContainer.right
      bottom: _xLabelContainer.top
    }

    //
    // Set margins
    //
    marginTop: 8
    marginRight: 8
    marginBottom: 8
    marginLeft: root.showCrosshairs && !yLabelVisible ? yLabelMargin : -8
    readonly property real yLabelMargin: Math.max(_maxYMetric.width, _minYMetric.width) + 8 + _yPosLabel.padding * 2

    TextMetrics {
      text: yMax
      id: _maxYMetric
      font: _yPosLabel.font
    }

    TextMetrics {
      text: yMin
      id: _minYMetric
      font: _yPosLabel.font
    }

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
    zoomAreaEnabled: false
    enabled: root.mouseAreaEnabled
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

      anchors.fill: parent
      preventStealing: true
      propagateComposedEvents: true
      acceptedButtons: Qt.LeftButton

      visible: root.mouseAreaEnabled
      enabled: root.mouseAreaEnabled
      hoverEnabled: root.mouseAreaEnabled

      cursorShape: dragging ? Qt.ClosedHandCursor : Qt.CrossCursor

      //
      // Custom properties for drag handling
      //
      property real _lastX: 0
      property real _lastY: 0
      readonly property bool dragging: containsPress && _axisX.zoom > 1

      //
      // Drag state handling
      //
      onContainsPressChanged: {
        _lastX = _overlayMouse.mouseX
        _lastY = _overlayMouse.mouseY
      }

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
                 if (!containsMouse || !root.mouseAreaEnabled) {
                   wheel.accepted = false
                   return
                 }

                 // Obtain X/Y position relative to graph
                 const localX = mouseX - _graph.plotArea.x
                 const localY = mouseY - _graph.plotArea.y

                 // Calculate new zoom factor
                 const zoomFactor = 1.15
                 const delta = -wheel.angleDelta.y / 120
                 const factor = Math.pow(zoomFactor, -delta)

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
                           if (!containsMouse) {
                             mouse.accepted = false
                             return
                           }

                           // Obtain graph size
                           const graphW = _graph.plotArea.width
                           const graphH = _graph.plotArea.height

                           // Crosshair position
                           _verCrosshair.x = mouse.x
                           _horCrosshair.y = mouse.y
                           _verCrosshair.requestPaint()
                           _horCrosshair.requestPaint()

                           // Update values
                           updateCrosshairLabels(mouse.x, mouse.y)

                           // Micro-pan when dragging
                           if (_overlayMouse.dragging) {
                             // Obtain drag distance
                             const dx = mouse.x - _lastX
                             const dy = mouse.y - _lastY

                             // Update pan
                             root.adjustAxisPan(_axisX, _graph.plotArea.width, mouse.x, dx, true)
                             root.adjustAxisPan(_axisY, _graph.plotArea.height, mouse.y, dy, false)

                             // Update drag start point
                             _lastX = mouse.x
                             _lastY = mouse.y

                             // Update crosshair values again
                             root.updateCrosshairLabels(mouse.x, mouse.y)
                           }
                         }
    }

    //
    // Vertical crosshair line
    //
    Canvas {
      id: _verCrosshair
      width: 2
      visible: root.showCrosshairs && _overlayMouse.containsMouse

      onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = Cpp_ThemeManager.colors["polar_indicator"]
        ctx.setLineDash([4, 4])
        ctx.lineDashOffset = 0

        ctx.beginPath()
        ctx.moveTo(0.5, 0)
        ctx.lineTo(0.5, height)
        ctx.moveTo(1.5, 0)
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
      visible: root.showCrosshairs && _overlayMouse.containsMouse

      height: 2
      onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = Cpp_ThemeManager.colors["polar_indicator"]
        ctx.setLineDash([4, 4])
        ctx.lineDashOffset = 0

        ctx.beginPath()
        ctx.moveTo(0, 0.5)
        ctx.lineTo(width, 0.5)
        ctx.moveTo(0, 1.5)
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
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: Cpp_ThemeManager.colors["widget_base"]
      visible: root.showCrosshairs && _overlayMouse.containsMouse

      background: Rectangle {
        opacity: 0.8
        color: Cpp_ThemeManager.colors["polar_indicator"]
      }

      anchors {
        top: parent.bottom
        horizontalCenter: _verCrosshair.horizontalCenter
      }
    }

    //
    // Y position label
    //
    Label {
      id: _yPosLabel

      padding: 4
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: Cpp_ThemeManager.colors["widget_base"]
      visible: root.showCrosshairs && _overlayMouse.containsMouse

      background: Rectangle {
        opacity: 0.8
        color: Cpp_ThemeManager.colors["polar_indicator"]
      }

      anchors {
        right: parent.left
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
      color: Cpp_ThemeManager.colors["widget_text"]
      font: Cpp_Misc_CommonFonts.customMonoFont(0.91, true)
      anchors.verticalCenterOffset: root.xLabelVisible && _yLabel.implicitWidth <= _graph.height ?
                                      -1 * Math.abs(_graph.marginBottom - _graph.marginTop) : 0
    }
  }

  //
  // X-axis label + real time mouse position
  //
  Item {
    id: _xLabelContainer
    height: _layout.implicitHeight

    anchors {
      right: parent.right
      bottom: parent.bottom
      left: _yLabelContainer.right
      leftMargin: 4
    }

    ColumnLayout {
      id: _layout
      spacing: 0
      width: parent.width
      anchors.centerIn: parent
      anchors.horizontalCenterOffset: Math.abs(_graph.marginLeft - _graph.marginRight)

      Label {
        id: _xLabel
        elide: Qt.ElideRight
        visible: root.xLabelVisible
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Qt.AlignHCenter
        color: Cpp_ThemeManager.colors["widget_text"]
        font: Cpp_Misc_CommonFonts.customMonoFont(0.91, true)
      }

      Item {
        implicitHeight: 2
        visible: _xLabel.visible && _posLabel.visible
      }

      Label {
        id: _posLabel
        elide: Qt.ElideRight
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Qt.AlignHCenter
        color: Cpp_ThemeManager.colors["widget_text"]
        font: Cpp_Misc_CommonFonts.customMonoFont(0.91, false)
        text: qsTr("%1, %2").arg(_xPosLabel.text).arg(_yPosLabel.text)
        opacity: (root.mouseAreaEnabled && _xPosLabel.text.length > 0 && _yPosLabel.text.length) > 0 ? 1 : 0

        Behavior on opacity {NumberAnimation{}}
      }

      Item {
        implicitHeight: 4
        visible: _posLabel.visible
      }
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
