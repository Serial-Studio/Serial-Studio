/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

//
// Pointer handling for PlotWidget: CAD-like pan/zoom, cursor placement and dragging, the
// trigger-level grab and the ruler right-click. What it cannot own leaves as a signal.
//
MouseArea {
  id: root

  //
  // The PlotWidget driven by this area; axes, plot area and cursor state are read from it
  //
  required property Item plot

  //
  // Ruler context menu requested by a plain right-click outside cursor mode
  //
  signal rulerMenuRequested(real worldX, int markerIndex)

  //
  // The pointer came to rest over the trigger line, which should flash into view
  //
  signal triggerFlashRequested()

  //
  // The user dragged the trigger line to @p level (world Y, already clamped to the view)
  //
  signal triggerLevelRequested(real level)

  preventStealing: true
  propagateComposedEvents: true
  acceptedButtons: Qt.LeftButton | Qt.RightButton

  enabled: root.plot.mouseAreaEnabled
  visible: root.plot.mouseAreaEnabled
  hoverEnabled: root.plot.mouseAreaEnabled

  cursorShape: dragging ? Qt.ClosedHandCursor :
               (draggingTrigger || overTrigger ? Qt.SizeVerCursor :
               (draggedCursor !== null ? Qt.SizeAllCursor :
               (root.plot.cursorMode ? Qt.CrossCursor : Qt.ArrowCursor)))

  //
  // Custom properties for drag handling
  //
  property real lastX: 0
  property real lastY: 0
  property real dragStartX: 0
  property real dragStartY: 0
  property bool didDrag: false
  property bool overTrigger: false
  property var draggedCursor: null
  property bool draggingTrigger: false
  property int pressedButton: Qt.NoButton
  readonly property bool dragging: containsPress && root.plot.xAxis.zoom > 1
                                   && draggedCursor === null && !draggingTrigger

  //
  // Flash the trigger line when the pointer moves over it, so a faded line
  // becomes visible before the user grabs it.
  //
  onOverTriggerChanged: {
    if (overTrigger)
      root.triggerFlashRequested()
  }

  function getNearestCursor(mouseX, mouseY) {
    const threshold = 10

    if (root.plot.cursorAVisible) {
      const aPixelX = root.plot.worldToPixelX(root.plot.cursorAX)
      const aPixelY = root.plot.worldToPixelY(root.plot.cursorAY)
      const distA = Math.sqrt(Math.pow(mouseX - aPixelX, 2) + Math.pow(mouseY - aPixelY, 2))
      if (distA < threshold)
        return "A"
    }

    if (root.plot.cursorBVisible) {
      const bPixelX = root.plot.worldToPixelX(root.plot.cursorBX)
      const bPixelY = root.plot.worldToPixelY(root.plot.cursorBY)
      const distB = Math.sqrt(Math.pow(mouseX - bPixelX, 2) + Math.pow(mouseY - bPixelY, 2))
      if (distB < threshold)
        return "B"
    }

    return null
  }

  //
  // Vertical hit-test for the trigger-level line (sweep mode + editing only)
  //
  function nearTriggerLine(mouseY) {
    if (!root.plot.triggerDraggable)
      return false

    const threshold = 8
    const linePixelY = root.plot.worldToPixelY(root.plot.triggerLevel)
    return Math.abs(mouseY - linePixelY) <= threshold
  }

  //
  // Pan an axis by @p dPx pixels, clamped so the view never leaves the axis bounds
  //
  function adjustAxisPan(axis, axisLength, cursorPos, dPx, inverted) {
    const fullRange = axis.max - axis.min
    const visibleRange = fullRange / axis.zoom
    const unitPerPixel = fullRange / axisLength

    const pxDiff = (inverted ? -dPx : dPx)
    const zoomDampeningFactor = 1 / axis.zoom
    let newPan = axis.pan + pxDiff * unitPerPixel * zoomDampeningFactor

    const maxPan = (axis.max - (axis.min + visibleRange)) / 2
    const minPan = (axis.min - (axis.max - visibleRange)) / 2
    newPan = Math.min(Math.max(newPan, minPan), maxPan)

    axis.pan = newPan
  }

  //
  // Cursor-centered zoom: keeps the world point under the cursor fixed.
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
  // Drag state handling
  //
  onPressed: (mouse) => {
    lastX = mouse.x
    lastY = mouse.y
    dragStartX = mouse.x
    dragStartY = mouse.y
    didDrag = false
    pressedButton = mouse.button

    // Grab the trigger line if the press lands on it (vertical drag only)
    if (mouse.button === Qt.LeftButton && nearTriggerLine(mouse.y)) {
      draggingTrigger = true
      mouse.accepted = true
      return
    }

    // Handle cursor interactions when in cursor mode
    if (root.plot.cursorMode) {
      draggedCursor = getNearestCursor(mouse.x, mouse.y)

      //
      // Right click to clear cursors (immediate action)
      //
      if (mouse.button === Qt.RightButton) {
        if (draggedCursor === "A") {
          root.plot.clearCursorA()
        } else if (draggedCursor === "B") {
          root.plot.clearCursorB()
        } else {
          //
          // Clear both if not clicking on a specific cursor
          //
          root.plot.clearAllCursors()
        }
      }
      //
      // Left-click placement deferred to onReleased so drag still pans.
      //
    }

    mouse.accepted = true
  }

  onReleased: (mouse) => {
    // Trigger drag consumes the gesture; no cursor placement on release
    if (draggingTrigger) {
      draggingTrigger = false
      pressedButton = Qt.NoButton
      didDrag = false
      overTrigger = nearTriggerLine(mouse.y)
      return
    }

    // Ruler menu on a plain right-click outside cursor mode (cursor mode keeps clearing)
    if (!root.plot.cursorMode && pressedButton === Qt.RightButton && !didDrag) {
      root.rulerMenuRequested(root.plot.pixelToWorldX(mouse.x), root.plot.markerNear(mouse.x))
      pressedButton = Qt.NoButton
      return
    }

    // Handle cursor placement on release (only if no drag occurred)
    if (root.plot.cursorMode && pressedButton === Qt.LeftButton && !didDrag
        && draggedCursor === null) {
      const worldX = root.plot.pixelToWorldX(mouse.x)
      const worldY = root.plot.pixelToWorldY(mouse.y)

      //
      // Place cursor A if not visible
      //
      if (!root.plot.cursorAVisible)
        root.plot.setCursorA(worldX, worldY)

      //
      // Cursor A visible, place cursor B
      //
      else if (!root.plot.cursorBVisible)
        root.plot.setCursorB(worldX, worldY)

      //
      // Both cursors exist, replace cursor A
      //
      else
        root.plot.setCursorA(worldX, worldY)
    }

    // Reset state
    draggedCursor = null
    pressedButton = Qt.NoButton
    didDrag = false
  }

  //
  // Handle mouse wheel zoom interaction
  //
  onWheel: (wheel) => {
    // Abort if not mouse is not in plot
    if (!containsMouse || !root.plot.mouseAreaEnabled) {
      wheel.accepted = false
      return
    }

    // Obtain X/Y position relative to graph
    const localX = mouseX - root.plot.plotArea.x
    const localY = mouseY - root.plot.plotArea.y

    // Calculate new zoom factor
    const zoomFactor = 1.15
    const delta = -wheel.angleDelta.y / 120
    const factor = Math.pow(zoomFactor, -delta)

    // Calculate new zoom values for both axes
    const xAxis = root.plot.xAxis
    const yAxis = root.plot.yAxis
    const newZoomX = xAxis.zoom * factor
    const newZoomY = yAxis.zoom * factor

    // Zoom & navigate through the graph
    root.applyCursorZoom(xAxis, xAxis.zoom, newZoomX, localX, root.plot.plotArea.width, false)
    root.applyCursorZoom(yAxis, yAxis.zoom, newZoomY, localY, root.plot.plotArea.height, true)

    // Update crosshair labels to reflect new view window
    root.plot.updateCrosshairLabels(localX, localY)
    wheel.accepted = true
  }

  //
  // Handle mouse movement
  //
  onPositionChanged: (mouse) => {
    // Abort if not mouse is not in plot
    if (!containsMouse) {
      mouse.accepted = false
      overTrigger = false
      return
    }

    // Track hover over the trigger line so the cursor can hint a vertical drag
    if (!containsPress)
      overTrigger = nearTriggerLine(mouse.y)

    // Drag the trigger level vertically and push it back to the model
    if (draggingTrigger) {
      const worldY = root.plot.pixelToWorldY(mouse.y)
      const clampedY = Math.min(Math.max(worldY, root.plot.yVisibleMin), root.plot.yVisibleMax)
      root.triggerLevelRequested(clampedY)
      mouse.accepted = true
      return
    }

    // Calculate drag distance from start position
    const dragDistSq = Math.pow(mouse.x - dragStartX, 2) + Math.pow(mouse.y - dragStartY, 2)
    const dragThreshold = 5

    // Past the threshold, treat as a drag.
    if (containsPress && dragDistSq > dragThreshold * dragThreshold) {
      didDrag = true
    }

    // Handle cursor dragging
    if (draggedCursor !== null && containsPress && root.plot.cursorMode) {
      const worldX = root.plot.pixelToWorldX(mouse.x)
      const worldY = root.plot.pixelToWorldY(mouse.y)

      if (draggedCursor === "A") {
        root.plot.cursorAX = worldX
        root.plot.cursorAY = worldY
      } else if (draggedCursor === "B") {
        root.plot.cursorBX = worldX
        root.plot.cursorBY = worldY
      }
    }
    //
    // Micro-pan when dragging the plot (when not dragging a cursor)
    //
    else if (root.dragging) {
      //
      // Obtain drag distance
      //
      const dx = mouse.x - lastX
      const dy = mouse.y - lastY

      //
      // Update pan
      //
      root.adjustAxisPan(root.plot.xAxis, root.plot.plotArea.width, mouse.x, dx, true)
      root.adjustAxisPan(root.plot.yAxis, root.plot.plotArea.height, mouse.y, dy, false)

      //
      // Update drag start point
      //
      lastX = mouse.x
      lastY = mouse.y
    }
  }
}
