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
  // Calculate tick intervals based on visible range (adjusts with zoom)
  //
  function smartInterval(min, max) {
    const range = max - min
    if (range <= 0)
      return 1.0

    const roughInterval = range / 10.0
    const magnitude = Math.pow(10, Math.floor(Math.log10(roughInterval)))
    const normalized = roughInterval / magnitude

    let niceInterval
    if (normalized <= 1.0)
      niceInterval = 1.0
    else if (normalized <= 2.0)
      niceInterval = 2.0
    else if (normalized <= 5.0)
      niceInterval = 5.0
    else
      niceInterval = 10.0

    return niceInterval * magnitude
  }

  //
  // Calculate appropriate decimal places based on visible range
  //
  function smartPrecision(visibleRange) {
    if (visibleRange <= 0)
      return 2

    // Calculate magnitude of the visible range
    const magnitude = Math.floor(Math.log10(visibleRange))

    // For small ranges (high zoom), we need more decimal places
    // For large ranges (low zoom), we need fewer decimal places
    // We want roughly 3-4 significant digits in the displayed value
    const precision = Math.max(0, 2 - magnitude)
    return Math.min(precision, 6)  // Cap at 6 decimal places
  }

  //
  // Check if a point (in world coordinates) is within the visible area
  //
  function isPointVisible(worldX, worldY) {
    return worldX >= xVisibleMin && worldX <= xVisibleMax &&
           worldY >= yVisibleMin && worldY <= yVisibleMax
  }

  //
  // Visible range calculations for dynamic tick intervals
  //
  readonly property real xVisibleRange: (xMax - xMin) / _axisX.zoom
  readonly property real yVisibleRange: (yMax - yMin) / _axisY.zoom
  readonly property real xVisibleMin: xMin + (xMax - xMin) / 2 + _axisX.pan - xVisibleRange / 2
  readonly property real xVisibleMax: xVisibleMin + xVisibleRange
  readonly property real yVisibleMin: yMin + (yMax - yMin) / 2 + _axisY.pan - yVisibleRange / 2
  readonly property real yVisibleMax: yVisibleMin + yVisibleRange

  //
  // Dynamic tick intervals based on visible range
  //
  readonly property real xTickInterval: smartInterval(xVisibleMin, xVisibleMax)
  readonly property real yTickInterval: smartInterval(yVisibleMin, yVisibleMax)

  //
  // Custom properties
  //
  property bool xLabelVisible: true
  property bool yLabelVisible: true
  property bool showCrosshairs: false
  property bool mouseAreaEnabled: true

  //
  // Cursor color properties
  //
  property color cursorAColor: Cpp_ThemeManager.colors["plot_cursor_a"]
  property color cursorBColor: Cpp_ThemeManager.colors["plot_cursor_b"]
  property color cursorATextColor: Cpp_ThemeManager.colors["widget_base"]
  property color cursorBTextColor: Cpp_ThemeManager.colors["widget_base"]

  //
  // Cursor properties (internal - managed by showCrosshairs)
  //
  property real cursorAX: 0
  property real cursorAY: 0
  property real cursorBX: 0
  property real cursorBY: 0
  property bool cursorAVisible: false
  property bool cursorBVisible: false

  //
  // Cursor mode replaces crosshairs when enabled
  //
  readonly property bool cursorMode: showCrosshairs

  //
  // Reset cursor states when cursor mode is toggled off
  //
  onCursorModeChanged: {
    if (!cursorMode) {
      clearAllCursors()
    }
  }

  //
  // Cursor delta values
  //
  readonly property real deltaX: cursorBX - cursorAX
  readonly property real deltaY: cursorBY - cursorAY

  //
  // Dynamic precision based on visible range
  //
  readonly property int xPrecision: smartPrecision(xVisibleRange)
  readonly property int yPrecision: smartPrecision(yVisibleRange)

  //
  // Check if cursors are within visible area (full and partial)
  //
  readonly property bool cursorAInView: isPointVisible(cursorAX, cursorAY)
  readonly property bool cursorBInView: isPointVisible(cursorBX, cursorBY)
  readonly property bool cursorAXInRange: cursorAX >= xVisibleMin && cursorAX <= xVisibleMax
  readonly property bool cursorAYInRange: cursorAY >= yVisibleMin && cursorAY <= yVisibleMax
  readonly property bool cursorBXInRange: cursorBX >= xVisibleMin && cursorBX <= xVisibleMax
  readonly property bool cursorBYInRange: cursorBY >= yVisibleMin && cursorBY <= yVisibleMax

  //
  // Functions to manage cursors
  //
  function setCursorA(worldX, worldY) {
    cursorAX = worldX
    cursorAY = worldY
    cursorAVisible = true
  } function setCursorB(worldX, worldY) {
    cursorBX = worldX
    cursorBY = worldY
    cursorBVisible = true
  } function clearCursorA() {
    cursorAVisible = false
  } function clearCursorB() {
    cursorBVisible = false
  } function clearAllCursors() {
    cursorAVisible = false
    cursorBVisible = false
  }

  //
  // Convert world coordinates to pixel coordinates
  //
  function worldToPixelX(worldX) {
    const xFullRange = _axisX.max - _axisX.min
    const xWorldCenter = _axisX.min + xFullRange / 2
    const xVisibleRange = xFullRange / _axisX.zoom
    const xViewStart = xWorldCenter + _axisX.pan - xVisibleRange / 2
    const normalizedX = (worldX - xViewStart) / xVisibleRange
    return normalizedX * _graph.plotArea.width
  }

  function worldToPixelY(worldY) {
    const yFullRange = _axisY.max - _axisY.min
    const yWorldCenter = _axisY.min + yFullRange / 2
    const yVisibleRange = yFullRange / _axisY.zoom
    const yViewStart = yWorldCenter + _axisY.pan - yVisibleRange / 2
    const normalizedY = (worldY - yViewStart) / yVisibleRange
    return (1 - normalizedY) * _graph.plotArea.height
  }

  //
  // Convert pixel coordinates to world coordinates
  //
  function pixelToWorldX(pixelX) {
    const xFullRange = _axisX.max - _axisX.min
    const xWorldCenter = _axisX.min + xFullRange / 2
    const xVisibleRange = xFullRange / _axisX.zoom
    const xViewStart = xWorldCenter + _axisX.pan - xVisibleRange / 2
    return xViewStart + xVisibleRange * (pixelX / _graph.plotArea.width)
  }

  function pixelToWorldY(pixelY) {
    const yFullRange = _axisY.max - _axisY.min
    const yWorldCenter = _axisY.min + yFullRange / 2
    const yVisibleRange = yFullRange / _axisY.zoom
    const yViewStart = yWorldCenter + _axisY.pan - yVisibleRange / 2
    return yViewStart + yVisibleRange * (1 - (pixelY / _graph.plotArea.height))
  }

  //
  // Updates the X and Y value labels to reflect the world coordinates under
  // the mouse cursor.
  //
  function updateCrosshairLabels(mouseX, mouseY) {
    // Only update if mouse is inside the tracking area
    if (!_overlayMouse.containsMouse)
      return

    const x = pixelToWorldX(mouseX)
    const y = pixelToWorldY(mouseY)

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
  // Interactive Overlay: handles crosshairs, cursors, and CAD-like zooming
  //
  Item {
    width: _graph.plotArea.width
    height: _graph.plotArea.height
    x: _graph.x + _graph.plotArea.x
    y: _graph.y + _graph.plotArea.y

    //
    // MouseArea for interactive graph navigation and inspection
    //
    MouseArea {
      id: _overlayMouse

      anchors.fill: parent
      preventStealing: true
      propagateComposedEvents: true
      acceptedButtons: Qt.LeftButton | Qt.RightButton

      visible: root.mouseAreaEnabled
      enabled: root.mouseAreaEnabled
      hoverEnabled: root.mouseAreaEnabled

      cursorShape: dragging ? Qt.ClosedHandCursor :
                   (draggedCursor !== null ? Qt.SizeAllCursor :
                   (root.cursorMode ? Qt.CrossCursor : Qt.ArrowCursor))

      //
      // Custom properties for drag handling
      //
      property real _lastX: 0
      property real _lastY: 0
      property real _startX: 0
      property real _startY: 0
      property bool _didDrag: false
      property int _pressedButton: Qt.NoButton
      property var draggedCursor: null  // null, "A", or "B"
      readonly property bool dragging: containsPress && _axisX.zoom > 1 && draggedCursor === null

      //
      // Check if mouse is near a cursor (within 10 pixels)
      //
      function getNearestCursor(mouseX, mouseY) {
        const threshold = 10

        if (root.cursorAVisible) {
          const aPixelX = root.worldToPixelX(root.cursorAX)
          const aPixelY = root.worldToPixelY(root.cursorAY)
          const distA = Math.sqrt(Math.pow(mouseX - aPixelX, 2) + Math.pow(mouseY - aPixelY, 2))
          if (distA < threshold)
            return "A"
        }

        if (root.cursorBVisible) {
          const bPixelX = root.worldToPixelX(root.cursorBX)
          const bPixelY = root.worldToPixelY(root.cursorBY)
          const distB = Math.sqrt(Math.pow(mouseX - bPixelX, 2) + Math.pow(mouseY - bPixelY, 2))
          if (distB < threshold)
            return "B"
        }

        return null
      }

      //
      // Drag state handling
      //
      onPressed: (mouse) => {
        _lastX = mouse.x
        _lastY = mouse.y
        _startX = mouse.x
        _startY = mouse.y
        _didDrag = false
        _pressedButton = mouse.button

        // Handle cursor interactions when in cursor mode
        if (root.cursorMode) {
          // Check if clicking near a cursor (for dragging)
          draggedCursor = getNearestCursor(mouse.x, mouse.y)

          // Right click to clear cursors (immediate action)
          if (mouse.button === Qt.RightButton) {
            if (draggedCursor === "A") {
              root.clearCursorA()
            } else if (draggedCursor === "B") {
              root.clearCursorB()
            } else {
              // Clear both if not clicking on a specific cursor
              root.clearAllCursors()
            }
          }
          // Left click cursor placement is deferred to onReleased
          // to allow panning when dragging
        }

        mouse.accepted = true
      }

      onReleased: (mouse) => {
        // Handle cursor placement on release (only if no drag occurred)
        if (root.cursorMode && _pressedButton === Qt.LeftButton && !_didDrag && draggedCursor === null) {
          const worldX = root.pixelToWorldX(mouse.x)
          const worldY = root.pixelToWorldY(mouse.y)

          // Place cursor A if not visible, otherwise place cursor B
          if (!root.cursorAVisible) {
            root.setCursorA(worldX, worldY)
          } else if (!root.cursorBVisible) {
            root.setCursorB(worldX, worldY)
          } else {
            // Both cursors exist, replace cursor A
            root.setCursorA(worldX, worldY)
          }
        }

        // Reset state
        draggedCursor = null
        _pressedButton = Qt.NoButton
        _didDrag = false
      }

      //
      // Handle mouse wheel zoom interaction
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
      // Handle mouse movement
      //
      onPositionChanged: (mouse) => {
        // Abort if not mouse is not in plot
        if (!containsMouse) {
          mouse.accepted = false
          return
        }

        // Calculate drag distance from start position
        const dragDistSq = Math.pow(mouse.x - _startX, 2) + Math.pow(mouse.y - _startY, 2)
        const dragThreshold = 5  // Pixels - consider it a drag after moving 5 pixels

        // Mark as drag if we've moved significantly
        if (containsPress && dragDistSq > dragThreshold * dragThreshold) {
          _didDrag = true
        }

        // Handle cursor dragging
        if (draggedCursor !== null && containsPress && root.cursorMode) {
          const worldX = root.pixelToWorldX(mouse.x)
          const worldY = root.pixelToWorldY(mouse.y)

          if (draggedCursor === "A") {
            root.cursorAX = worldX
            root.cursorAY = worldY
          } else if (draggedCursor === "B") {
            root.cursorBX = worldX
            root.cursorBY = worldY
          }
        }
        // Micro-pan when dragging the plot (when not dragging a cursor)
        else if (_overlayMouse.dragging) {
          // Obtain drag distance
          const dx = mouse.x - _lastX
          const dy = mouse.y - _lastY

          // Update pan
          root.adjustAxisPan(_axisX, _graph.plotArea.width, mouse.x, dx, true)
          root.adjustAxisPan(_axisY, _graph.plotArea.height, mouse.y, dy, false)

          // Update drag start point
          _lastX = mouse.x
          _lastY = mouse.y
        }
      }
    }

    //
    // Cursor A (visible when in cursor mode and any part is in view)
    //
    Item {
      id: _cursorA
      visible: root.cursorMode && root.cursorAVisible && (root.cursorAXInRange || root.cursorAYInRange)
      x: root.worldToPixelX(root.cursorAX)
      y: root.worldToPixelY(root.cursorAY)

      // Vertical line with dash-dot pattern (full height) - visible when X is in range
      Canvas {
        id: _cursorAVertical
        visible: root.cursorAXInRange
        width: 2
        height: parent.parent.height
        x: 0
        y: -parent.y

        onPaint: {
          var ctx = getContext("2d")
          ctx.clearRect(0, 0, width, height)
          ctx.strokeStyle = root.cursorAColor
          ctx.lineWidth = 2
          ctx.setLineDash([8, 4, 2, 4])
          ctx.lineDashOffset = 0

          ctx.beginPath()
          ctx.moveTo(1, 0)
          ctx.lineTo(1, height)
          ctx.stroke()
        }
      }

      // Horizontal line with dash-dot pattern (full width) - visible when Y is in range
      Canvas {
        id: _cursorAHorizontal
        visible: root.cursorAYInRange
        width: parent.parent.width
        height: 2
        x: -parent.x
        y: 0

        onPaint: {
          var ctx = getContext("2d")
          ctx.clearRect(0, 0, width, height)
          ctx.strokeStyle = root.cursorAColor
          ctx.lineWidth = 2
          ctx.setLineDash([8, 4, 2, 4])
          ctx.lineDashOffset = 0

          ctx.beginPath()
          ctx.moveTo(0, 1)
          ctx.lineTo(width, 1)
          ctx.stroke()
        }
      }

      // Center marker - only visible when fully in view
      Rectangle {
        visible: root.cursorAInView
        width: 10
        height: 10
        radius: 5
        color: root.cursorAColor
        border.width: 2
        border.color: Cpp_ThemeManager.colors["widget_base"]
        anchors.centerIn: parent
      }

      // Cursor A identifier label - only visible when fully in view
      Label {
        visible: root.cursorAInView
        text: "A"
        color: root.cursorATextColor
        font: Cpp_Misc_CommonFonts.customMonoFont(0.9, true)
        padding: 4
        background: Rectangle {
          color: root.cursorAColor
          opacity: 0.9
          radius: 3
        }
        anchors {
          left: parent.right
          top: parent.bottom
          leftMargin: 5
          topMargin: 5
        }
      }

      // Trigger repaint when cursor moves
      onXChanged: {
        _cursorAVertical.requestPaint()
        _cursorAHorizontal.requestPaint()
      }
      onYChanged: {
        _cursorAVertical.requestPaint()
        _cursorAHorizontal.requestPaint()
      }
    }

    //
    // Cursor B (visible when in cursor mode and any part is in view)
    //
    Item {
      id: _cursorB
      visible: root.cursorMode && root.cursorBVisible && (root.cursorBXInRange || root.cursorBYInRange)
      x: root.worldToPixelX(root.cursorBX)
      y: root.worldToPixelY(root.cursorBY)

      // Vertical line with dash-dot pattern (full height) - visible when X is in range
      Canvas {
        id: _cursorBVertical
        visible: root.cursorBXInRange
        width: 2
        height: parent.parent.height
        x: 0
        y: -parent.y

        onPaint: {
          var ctx = getContext("2d")
          ctx.clearRect(0, 0, width, height)
          ctx.strokeStyle = root.cursorBColor
          ctx.lineWidth = 2
          ctx.setLineDash([8, 4, 2, 4])
          ctx.lineDashOffset = 0

          ctx.beginPath()
          ctx.moveTo(1, 0)
          ctx.lineTo(1, height)
          ctx.stroke()
        }
      }

      // Horizontal line with dash-dot pattern (full width) - visible when Y is in range
      Canvas {
        id: _cursorBHorizontal
        visible: root.cursorBYInRange
        width: parent.parent.width
        height: 2
        x: -parent.x
        y: 0

        onPaint: {
          var ctx = getContext("2d")
          ctx.clearRect(0, 0, width, height)
          ctx.strokeStyle = root.cursorBColor
          ctx.lineWidth = 2
          ctx.setLineDash([8, 4, 2, 4])
          ctx.lineDashOffset = 0

          ctx.beginPath()
          ctx.moveTo(0, 1)
          ctx.lineTo(width, 1)
          ctx.stroke()
        }
      }

      // Center marker - only visible when fully in view
      Rectangle {
        visible: root.cursorBInView
        width: 10
        height: 10
        radius: 5
        color: root.cursorBColor
        border.width: 2
        border.color: Cpp_ThemeManager.colors["widget_base"]
        anchors.centerIn: parent
      }

      // Cursor B identifier label - only visible when fully in view
      Label {
        visible: root.cursorBInView
        text: "B"
        color: root.cursorBTextColor
        font: Cpp_Misc_CommonFonts.customMonoFont(0.9, true)
        padding: 4
        background: Rectangle {
          color: root.cursorBColor
          opacity: 0.9
          radius: 3
        }
        anchors {
          left: parent.right
          top: parent.bottom
          leftMargin: 5
          topMargin: 5
        }
      }

      // Trigger repaint when cursor moves
      onXChanged: {
        _cursorBVertical.requestPaint()
        _cursorBHorizontal.requestPaint()
      }
      onYChanged: {
        _cursorBVertical.requestPaint()
        _cursorBHorizontal.requestPaint()
      }
    }

    //
    // X position label for Cursor A (on X-axis) - visible when X is in range
    //
    Label {
      id: _cursorAXPosLabel
      text: root.cursorAX.toFixed(root.xPrecision)
      padding: 4
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: root.cursorATextColor
      visible: root.cursorMode && root.cursorAVisible && root.cursorAXInRange

      background: Rectangle {
        opacity: 0.9
        color: root.cursorAColor
        radius: 3
      }

      x: Math.max(0, Math.min(parent.width - width, root.worldToPixelX(root.cursorAX) - width / 2))
      anchors {
        top: parent.bottom
        topMargin: 2
      }
    }

    //
    // Y position label for Cursor A (on Y-axis) - visible when Y is in range
    //
    Label {
      id: _cursorAYPosLabel
      text: root.cursorAY.toFixed(root.yPrecision)
      padding: 4
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: root.cursorATextColor
      visible: root.cursorMode && root.cursorAVisible && root.cursorAYInRange

      background: Rectangle {
        opacity: 0.9
        color: root.cursorAColor
        radius: 3
      }

      y: Math.max(0, Math.min(parent.height - height, root.worldToPixelY(root.cursorAY) - height / 2))
      anchors {
        right: parent.left
        rightMargin: 2
      }
    }

    //
    // X position label for Cursor B (on X-axis) - visible when X is in range
    //
    Label {
      id: _cursorBXPosLabel
      text: root.cursorBX.toFixed(root.xPrecision)
      padding: 4
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: root.cursorBTextColor
      visible: root.cursorMode && root.cursorBVisible && root.cursorBXInRange

      background: Rectangle {
        opacity: 0.9
        color: root.cursorBColor
        radius: 3
      }

      x: Math.max(0, Math.min(parent.width - width, root.worldToPixelX(root.cursorBX) - width / 2))
      anchors {
        top: parent.bottom
        topMargin: 2
      }
    }

    //
    // Y position label for Cursor B (on Y-axis) - visible when Y is in range
    //
    Label {
      id: _cursorBYPosLabel
      text: root.cursorBY.toFixed(root.yPrecision)
      padding: 4
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: root.cursorBTextColor
      visible: root.cursorMode && root.cursorBVisible && root.cursorBYInRange

      background: Rectangle {
        opacity: 0.9
        color: root.cursorBColor
        radius: 3
      }

      y: Math.max(0, Math.min(parent.height - height, root.worldToPixelY(root.cursorBY) - height / 2))
      anchors {
        right: parent.left
        rightMargin: 2
      }
    }

    //
    // X position label (removed - not needed in cursor mode)
    //
    Label {
      id: _xPosLabel
      visible: false
      padding: 4
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: Cpp_ThemeManager.colors["widget_base"]
    }

    //
    // Y position label (removed - not needed in cursor mode)
    //
    Label {
      id: _yPosLabel
      visible: false
      padding: 4
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
      color: Cpp_ThemeManager.colors["widget_base"]
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
  // X-axis label + real time mouse position + cursor delta
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
        visible: _deltaLabel.visible
      }

      Label {
        id: _deltaLabel
        elide: Qt.ElideRight
        Layout.maximumWidth: parent.width
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Qt.AlignHCenter
        color: Cpp_ThemeManager.colors["widget_text"]
        font: Cpp_Misc_CommonFonts.customMonoFont(0.85, false)
        visible: root.cursorMode
        text: {
          if (root.cursorAVisible && root.cursorBVisible)
            return qsTr("ΔX: %1  ΔY: %2 — Drag to move, right-click to clear").arg(root.deltaX.toFixed(root.xPrecision)).arg(root.deltaY.toFixed(root.yPrecision))
          else if (!root.cursorAVisible)
            return qsTr("Click to place cursor")
          else
            return qsTr("Click to place second cursor — Drag to move")
        }
      }

      Item {
        implicitHeight: 4
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
