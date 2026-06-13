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
  property real xMin: 0
  property real xMax: 1
  property real yMin: 0
  property real yMax: 1
  property string xLabel: ""
  property alias graph: _graph
  property alias xAxis: _axisX
  property alias yAxis: _axisY
  property real triggerLevel: 0
  property bool sweepMode: false
  property alias zoom: _axisX.zoom
  property alias yLabel: _yLabel.text
  property bool triggerEditing: false
  property alias plotArea: _graph.plotArea
  property alias curveColors: _theme.seriesColors

  //
  // Area-under-curve fill: assign a curve series to areaFillSource and the fill
  // renders as a GPU triangle strip under the crosshair overlay (PlotAreaFill)
  //
  property var areaFillSource: null
  property real areaFillBaseline: 0
  property color areaFillColor: "transparent"

  //
  // GPU curve layer: dashboard widgets parent PlotCurve items here (anchors.fill the
  // layer); it sits above the area fill and below the crosshair overlay
  //
  property alias curveLayer: _curveLayer

  //
  // Emitted when the user drags the trigger-level line; the parent writes the
  // new level back into the data model (triggerLevel is a one-way binding).
  //
  signal triggerLevelChangeRequested(real level)

  //
  // TextMetrics for estimating label sizes
  //
  TextMetrics {
    id: _xLabelMetrics

    text: "-8888.88"
    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.83))
  }

  TextMetrics {
    id: _yLabelMetrics

    text: "-8888.88"
    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.83))
  }

  //
  // Calculate tick intervals for X axis based on available width
  //
  function smartIntervalX(min, max) {
    const range = max - min
    if (range <= 0)
      return 1.0

    const availableWidth = _graph.plotArea.width
    const labelWidth = _xLabelMetrics.width + 20
    const maxLabels = Math.max(2, Math.floor(availableWidth / labelWidth))

    const roughInterval = range / maxLabels
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
  // Calculate tick intervals for Y axis based on available height
  //
  function smartIntervalY(min, max) {
    const range = max - min
    if (range <= 0)
      return 1.0

    const availableHeight = _graph.plotArea.height
    const labelHeight = _yLabelMetrics.height + 8
    const maxLabels = Math.max(2, Math.floor(availableHeight / labelHeight))

    const roughInterval = range / maxLabels
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

    const magnitude = Math.floor(Math.log10(visibleRange))

    // ~3-4 significant digits across zoom levels
    const precision = Math.max(0, 2 - magnitude)
    return Math.min(precision, 6)
  }

  //
  // Engineering-unit formatter for tick labels (K/M/G, u/n/p)
  //
  function engineeringFormat(value, tickInterval) {
    if (!isFinite(value))
      return ""

    const abs    = Math.abs(value)
    const refMag = (tickInterval > 0) ? Math.max(abs, tickInterval) : abs

    // Pick the engineering scale from refMag, ignore millis range
    // code-verify off
    let scaleFactor = 1
    let suffix      = ""
    if      (refMag >= 1e9)  { scaleFactor = 1e9;   suffix = "G" }
    else if (refMag >= 1e6)  { scaleFactor = 1e6;   suffix = "M" }
    else if (refMag >= 1e3)  { scaleFactor = 1e3;   suffix = "K" }
    else if (refMag >= 1e-3) { scaleFactor = 1;     suffix = ""  }
    else if (refMag >= 1e-6) { scaleFactor = 1e-6;  suffix = "µ" }
    else if (refMag >= 1e-9) { scaleFactor = 1e-9;  suffix = "n" }
    else if (refMag > 0)     { scaleFactor = 1e-12; suffix = "p" }
    else                     { return "0" }
    // code-verify on

    // Decimals derive from the scaled tick interval
    let decimals
    if (tickInterval > 0) {
      const scaledInterval = tickInterval / scaleFactor
      decimals = scaledInterval >= 1
                  ? 0
                  : Math.min(6, Math.ceil(-Math.log10(scaledInterval) + 1e-9))
    } else if (abs === 0) {
      decimals = 0
    } else {
      const scaled = abs / scaleFactor
      decimals     = scaled >= 1
                      ? 0
                      : Math.min(6, Math.max(0,
                          2 - Math.floor(Math.log10(scaled))))
    }

    return (value / scaleFactor).toFixed(decimals) + suffix
  }

  //
  // Relative-time X axis: pick a friendly unit (s / ms / us) from the visible span so the
  // ticks and axis title read in whole, human numbers regardless of the time range.
  //
  readonly property real timeSpanSeconds: Math.abs(xMax - xMin)
  readonly property real timeUnitFactor: timeSpanSeconds >= 1 ? 1
                                       : (timeSpanSeconds >= 1e-3 ? 1e3 : 1e6)
  // code-verify off
  readonly property string timeUnitName: timeSpanSeconds >= 1 ? "s"
                                       : (timeSpanSeconds >= 1e-3 ? "ms" : "µs")
  // code-verify on

  //
  // Tick formatter for the relative-time X axis: shows the magnitude (time ago) in the
  // chosen unit, so the axis reads e.g. 10 8 6 4 2 0 with 0 = now on the right.
  //
  function secondsAgoFormat(value, tickInterval) {
    if (!isFinite(value))
      return ""

    const scaled   = Math.abs(value) * root.timeUnitFactor
    const scaledIv = tickInterval * root.timeUnitFactor
    let decimals   = 0
    if (scaledIv > 0 && scaledIv < 1)
      decimals = Math.min(3, Math.ceil(-Math.log10(scaledIv) + 1e-9))

    return scaled.toFixed(decimals)
  }

  //
  // Cursor/readout formatter: absolute time-ago magnitude in the chosen unit (e.g. "12 ms").
  //
  function timeAgoLabel(worldX) {
    const scaled = Math.abs(worldX) * root.timeUnitFactor
    return scaled.toFixed(scaled >= 100 ? 0 : (scaled >= 1 ? 1 : 3)) + " " + root.timeUnitName
  }

  function isPointVisible(worldX, worldY) {
    return worldX >= xVisibleMin && worldX <= xVisibleMax &&
           worldY >= yVisibleMin && worldY <= yVisibleMax
  }

  //
  // Visible range calculations for dynamic tick intervals
  //
  readonly property real xVisibleMax: xVisibleMin + xVisibleRange
  readonly property real yVisibleMax: yVisibleMin + yVisibleRange
  readonly property real xVisibleRange: (xMax - xMin) / _axisX.zoom
  readonly property real yVisibleRange: (yMax - yMin) / _axisY.zoom
  readonly property real xVisibleMin: xMin + (xMax - xMin) / 2 + _axisX.pan - xVisibleRange / 2
  readonly property real yVisibleMin: yMin + (yMax - yMin) / 2 + _axisY.pan - yVisibleRange / 2

  //
  // Dynamic tick intervals based on visible range and available space
  //
  readonly property real xTickInterval: smartIntervalX(xVisibleMin, xVisibleMax)
  readonly property real yTickInterval: smartIntervalY(yVisibleMin, yVisibleMax)

  //
  // Custom properties
  //
  property bool timeAxis: false
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
  // The trigger-level line is draggable while it is shown (sweep mode + editing)
  //
  readonly property bool triggerDraggable: sweepMode && triggerEditing

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

  function updateCrosshairLabels(mouseX, mouseY) {
    if (!_overlayMouse.containsMouse)
      return

    const x = pixelToWorldX(mouseX)
    const y = pixelToWorldY(mouseY)

    _xPosLabel.text = x.toFixed(2)
    _yPosLabel.text = y.toFixed(2)
  }

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
      id: _maxYMetric

      text: yMax
      font: _yPosLabel.font
    }

    TextMetrics {
      id: _minYMetric

      text: yMin
      font: _yPosLabel.font
    }

    //
    // Set plot colors
    //
    theme: GraphsTheme {
      id: _theme

      //
      // Background
      //
      borderColors: [Cpp_ThemeManager.colors["widget_border"]]
      plotAreaBackgroundVisible: true
      theme: GraphsTheme.Theme.UserDefined
      backgroundColor: Cpp_ThemeManager.colors["widget_window"]
      plotAreaBackgroundColor: Cpp_ThemeManager.colors["widget_base"]

      //
      // Axis and grid colors
      //
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

      //
      // Axis label fonts and colors
      //
      labelTextColor: Cpp_ThemeManager.colors["widget_text"]
      axisXLabelFont: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.83))
      axisYLabelFont: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.83))

      //
      // Grid settings
      //
      grid.subWidth: 1
      gridVisible: true
      grid.mainWidth: 1
      grid.subColor: Cpp_ThemeManager.colors["widget_border"]
      grid.mainColor: Cpp_ThemeManager.colors["widget_border"]

      //
      // Highlight colors for better contrast
      //
      multiHighlightColor: Cpp_ThemeManager.colors["widget_highlight"]
      singleHighlightColor: Cpp_ThemeManager.colors["widget_highlighted_text"]

      //
      // Force refresh all theme colors on theme change
      //
      Connections {
        target: Cpp_ThemeManager
        function onThemeChanged() {
          _theme.borderColors = [Cpp_ThemeManager.colors["widget_border"]]
          _theme.backgroundColor = Cpp_ThemeManager.colors["widget_window"]
          _theme.plotAreaBackgroundColor = Cpp_ThemeManager.colors["widget_base"]
          _theme.axisX.subColor = Cpp_ThemeManager.colors["widget_border"]
          _theme.axisY.subColor = Cpp_ThemeManager.colors["widget_border"]
          _theme.axisX.mainColor = Cpp_ThemeManager.colors["widget_border"]
          _theme.axisY.mainColor = Cpp_ThemeManager.colors["widget_border"]
          _theme.axisX.labelTextColor = Cpp_ThemeManager.colors["widget_text"]
          _theme.axisY.labelTextColor = Cpp_ThemeManager.colors["widget_text"]
          _theme.labelTextColor = Cpp_ThemeManager.colors["widget_text"]
          _theme.grid.subColor = Cpp_ThemeManager.colors["widget_border"]
          _theme.grid.mainColor = Cpp_ThemeManager.colors["widget_border"]
          _theme.multiHighlightColor = Cpp_ThemeManager.colors["widget_highlight"]
          _theme.singleHighlightColor = Cpp_ThemeManager.colors["widget_highlighted_text"]
        }
      }
    }

    //
    // Disable built-in pan/zoom; root drives both manually
    //
    zoomAreaEnabled: false
    enabled: root.mouseAreaEnabled
    panStyle: GraphsView.PanStyle.None
    zoomStyle: GraphsView.ZoomStyle.None

    //
    // Customize Y axis (engineering-unit tick labels)
    //
    axisY: ValueAxis {
      id: _axisY

      min: root.yMin
      max: root.yMax
      subTickCount: 1
      tickAnchor: root.yMin
      visible: root.yLabelVisible
      tickInterval: root.yTickInterval

      labelDelegate: Item {
        id: _yLabelItem

        property string text

        implicitWidth:  _yEngLabel.implicitWidth
        implicitHeight: _yEngLabel.implicitHeight

        Text {
          id: _yEngLabel

          anchors.centerIn: parent
          text: root.engineeringFormat(parseFloat(_yLabelItem.text), root.yTickInterval)
          color: Cpp_ThemeManager.colors["widget_text"]
          font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                 Cpp_Misc_CommonFonts.widgetFont(0.83))
        }
      }
    }

    //
    // Customize X axis (same engineering-unit treatment as Y)
    //
    axisX: ValueAxis {
      id: _axisX

      min: root.xMin
      max: root.xMax
      subTickCount: 1
      tickAnchor: root.xMin
      visible: root.xLabelVisible
      tickInterval: root.xTickInterval

      labelDelegate: Item {
        id: _xLabelItem

        property string text

        implicitWidth:  _xEngLabel.implicitWidth
        implicitHeight: _xEngLabel.implicitHeight

        Text {
          id: _xEngLabel

          anchors.centerIn: parent
          text: root.timeAxis
                ? root.secondsAgoFormat(parseFloat(_xLabelItem.text), root.xTickInterval)
                : root.engineeringFormat(parseFloat(_xLabelItem.text), root.xTickInterval)
          color: Cpp_ThemeManager.colors["widget_text"]
          font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                 Cpp_Misc_CommonFonts.widgetFont(0.83))
        }
      }
    }
  }

  //
  // Area-under-curve fill, tracking the visible window so it follows zoom/pan
  //
  PlotAreaFill {
    clip: true
    yMin: root.yVisibleMin
    xMin: root.xVisibleMin
    color: root.areaFillColor
    source: root.areaFillSource
    width: _graph.plotArea.width
    height: _graph.plotArea.height
    x: _graph.x + _graph.plotArea.x
    y: _graph.y + _graph.plotArea.y
    baselineValue: root.areaFillBaseline
    visible: root.areaFillSource !== null
    yMax: root.yVisibleMin + root.yVisibleRange
    xMax: root.xVisibleMin + root.xVisibleRange
  }

  //
  // GPU curve layer, tracking the plot area so PlotCurve children map world
  // coordinates with the same visible-window transform as the fill and cursors
  //
  Item {
    id: _curveLayer

    clip: true
    width: _graph.plotArea.width
    height: _graph.plotArea.height
    x: _graph.x + _graph.plotArea.x
    y: _graph.y + _graph.plotArea.y
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

      enabled: root.mouseAreaEnabled
      visible: root.mouseAreaEnabled
      hoverEnabled: root.mouseAreaEnabled

      cursorShape: dragging ? Qt.ClosedHandCursor :
                   (draggingTrigger || overTrigger ? Qt.SizeVerCursor :
                   (draggedCursor !== null ? Qt.SizeAllCursor :
                   (root.cursorMode ? Qt.CrossCursor : Qt.ArrowCursor)))

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
      readonly property bool dragging: containsPress && _axisX.zoom > 1
                                       && draggedCursor === null && !draggingTrigger

      //
      // Flash the trigger line when the pointer moves over it, so a faded line
      // becomes visible before the user grabs it.
      //
      onOverTriggerChanged: {
        if (overTrigger)
          _triggerLine.flash()
      }

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
      // Vertical hit-test for the trigger-level line (sweep mode + editing only)
      //
      function nearTriggerLine(mouseY) {
        if (!root.triggerDraggable)
          return false

        const threshold = 8
        const linePixelY = root.worldToPixelY(root.triggerLevel)
        return Math.abs(mouseY - linePixelY) <= threshold
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
        if (root.cursorMode) {
          draggedCursor = getNearestCursor(mouse.x, mouse.y)

          //
          // Right click to clear cursors (immediate action)
          //
          if (mouse.button === Qt.RightButton) {
            if (draggedCursor === "A") {
              root.clearCursorA()
            } else if (draggedCursor === "B") {
              root.clearCursorB()
            } else {
              //
              // Clear both if not clicking on a specific cursor
              //
              root.clearAllCursors()
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

        // Handle cursor placement on release (only if no drag occurred)
        if (root.cursorMode && pressedButton === Qt.LeftButton && !didDrag && draggedCursor === null) {
          const worldX = root.pixelToWorldX(mouse.x)
          const worldY = root.pixelToWorldY(mouse.y)

          //
          // Place cursor A if not visible
          //
          if (!root.cursorAVisible)
            root.setCursorA(worldX, worldY)

          //
          // Cursor A visible, place cursor B
          //
          else if (!root.cursorBVisible)
            root.setCursorB(worldX, worldY)

          //
          // Both cursors exist, replace cursor A
          //
          else
            root.setCursorA(worldX, worldY)
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
          overTrigger = false
          return
        }

        // Track hover over the trigger line so the cursor can hint a vertical drag
        if (!containsPress)
          overTrigger = nearTriggerLine(mouse.y)

        // Drag the trigger level vertically and push it back to the model
        if (draggingTrigger) {
          const worldY = root.pixelToWorldY(mouse.y)
          const clampedY = Math.min(Math.max(worldY, root.yVisibleMin), root.yVisibleMax)
          root.triggerLevelChangeRequested(clampedY)
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
        //
        // Micro-pan when dragging the plot (when not dragging a cursor)
        //
        else if (_overlayMouse.dragging) {
          //
          // Obtain drag distance
          //
          const dx = mouse.x - lastX
          const dy = mouse.y - lastY

          //
          // Update pan
          //
          root.adjustAxisPan(_axisX, _graph.plotArea.width, mouse.x, dx, true)
          root.adjustAxisPan(_axisY, _graph.plotArea.height, mouse.y, dy, false)

          //
          // Update drag start point
          //
          lastX = mouse.x
          lastY = mouse.y
        }
      }
    }

    //
    // Cursor A (visible when in cursor mode and any part is in view)
    //
    Item {
      id: _cursorA

      x: root.worldToPixelX(root.cursorAX)
      y: root.worldToPixelY(root.cursorAY)
      visible: root.cursorMode && root.cursorAVisible && (root.cursorAXInRange || root.cursorAYInRange)

      //
      // Vertical line with dash-dot pattern (full height) - visible when X is in range
      //
      Canvas {
        id: _cursorAVertical

        x: 0
        width: 2
        y: -parent.y
        height: parent.parent.height
        visible: root.cursorAXInRange

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

      //
      // Horizontal line with dash-dot pattern (full width) - visible when Y is in range
      //
      Canvas {
        id: _cursorAHorizontal

        y: 0
        height: 2
        x: -parent.x
        width: parent.parent.width
        visible: root.cursorAYInRange

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

      //
      // Center marker - only visible when fully in view
      //
      Rectangle {
        width: 10
        radius: 5
        height: 10
        border.width: 2
        color: root.cursorAColor
        anchors.centerIn: parent
        visible: root.cursorAInView
        border.color: Cpp_ThemeManager.colors["widget_base"]
      }

      //
      // Cursor A identifier label - only visible when fully in view
      //
      Label {
        text: "A"
        padding: 4
        visible: root.cursorAInView
        color: root.cursorATextColor
        font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.9, true))
        background: Rectangle {
          radius: 3
          opacity: 0.9
          color: root.cursorAColor
        }
        anchors {
          topMargin: 5
          leftMargin: 5
          left: parent.right
          top: parent.bottom
        }
      }

      //
      // Trigger repaint when cursor moves
      //
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

      x: root.worldToPixelX(root.cursorBX)
      y: root.worldToPixelY(root.cursorBY)
      visible: root.cursorMode && root.cursorBVisible && (root.cursorBXInRange || root.cursorBYInRange)

      //
      // Vertical line with dash-dot pattern (full height) - visible when X is in range
      //
      Canvas {
        id: _cursorBVertical

        x: 0
        width: 2
        y: -parent.y
        height: parent.parent.height
        visible: root.cursorBXInRange

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

      //
      // Horizontal line with dash-dot pattern (full width) - visible when Y is in range
      //
      Canvas {
        id: _cursorBHorizontal

        y: 0
        height: 2
        x: -parent.x
        width: parent.parent.width
        visible: root.cursorBYInRange

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

      //
      // Center marker - only visible when fully in view
      //
      Rectangle {
        width: 10
        radius: 5
        height: 10
        border.width: 2
        color: root.cursorBColor
        anchors.centerIn: parent
        visible: root.cursorBInView
        border.color: Cpp_ThemeManager.colors["widget_base"]
      }

      //
      // Cursor B identifier label - only visible when fully in view
      //
      Label {
        text: "B"
        padding: 4
        visible: root.cursorBInView
        color: root.cursorBTextColor
        font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.9, true))
        background: Rectangle {
          radius: 3
          opacity: 0.9
          color: root.cursorBColor
        }
        anchors {
          topMargin: 5
          leftMargin: 5
          left: parent.right
          top: parent.bottom
        }
      }

      //
      // Trigger repaint when cursor moves
      //
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

      padding: 4
      color: root.cursorATextColor
      visible: root.cursorMode && root.cursorAVisible && root.cursorAXInRange
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))
      text: root.timeAxis ? root.timeAgoLabel(root.cursorAX) : root.cursorAX.toFixed(root.xPrecision)

      background: Rectangle {
        radius: 3
        opacity: 0.9
        color: root.cursorAColor
      }

      x: Math.max(0, Math.min(parent.width - width, root.worldToPixelX(root.cursorAX) - width / 2))
      anchors {
        topMargin: 2
        top: parent.bottom
      }
    }

    //
    // Y position label for Cursor A (on Y-axis) - visible when Y is in range
    //
    Label {
      id: _cursorAYPosLabel

      padding: 4
      color: root.cursorATextColor
      text: root.cursorAY.toFixed(root.yPrecision)
      visible: root.cursorMode && root.cursorAVisible && root.cursorAYInRange
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))

      background: Rectangle {
        radius: 3
        opacity: 0.9
        color: root.cursorAColor
      }

      y: Math.max(0, Math.min(parent.height - height, root.worldToPixelY(root.cursorAY) - height / 2))
      anchors {
        rightMargin: 2
        right: parent.left
      }
    }

    //
    // X position label for Cursor B (on X-axis) - visible when X is in range
    //
    Label {
      id: _cursorBXPosLabel

      padding: 4
      color: root.cursorBTextColor
      visible: root.cursorMode && root.cursorBVisible && root.cursorBXInRange
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))
      text: root.timeAxis ? root.timeAgoLabel(root.cursorBX) : root.cursorBX.toFixed(root.xPrecision)

      background: Rectangle {
        radius: 3
        opacity: 0.9
        color: root.cursorBColor
      }

      x: Math.max(0, Math.min(parent.width - width, root.worldToPixelX(root.cursorBX) - width / 2))
      anchors {
        topMargin: 2
        top: parent.bottom
      }
    }

    //
    // Y position label for Cursor B (on Y-axis) - visible when Y is in range
    //
    Label {
      id: _cursorBYPosLabel

      padding: 4
      color: root.cursorBTextColor
      text: root.cursorBY.toFixed(root.yPrecision)
      visible: root.cursorMode && root.cursorBVisible && root.cursorBYInRange
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))

      background: Rectangle {
        radius: 3
        opacity: 0.9
        color: root.cursorBColor
      }

      y: Math.max(0, Math.min(parent.height - height, root.worldToPixelY(root.cursorBY) - height / 2))
      anchors {
        rightMargin: 2
        right: parent.left
      }
    }

    //
    // X position label (removed - not needed in cursor mode)
    //
    Label {
      id: _xPosLabel

      padding: 4
      visible: false
      color: Cpp_ThemeManager.colors["widget_base"]
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))
    }

    //
    // Y position label (removed - not needed in cursor mode)
    //
    Label {
      id: _yPosLabel

      padding: 4
      visible: false
      color: Cpp_ThemeManager.colors["widget_base"]
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))
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
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.91, true))
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
      leftMargin: 4
      right: parent.right
      bottom: parent.bottom
      left: _yLabelContainer.right
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
        text: root.timeAxis ? (qsTr("Time") + " (" + root.timeUnitName + ")") : root.xLabel
        color: Cpp_ThemeManager.colors["widget_text"]
        font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.91, true))
      }

      Item {
        implicitHeight: 2
        visible: _deltaLabel.visible
      }

      Label {
        id: _deltaLabel

        elide: Qt.ElideRight
        visible: root.cursorMode
        Layout.maximumWidth: parent.width
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Qt.AlignHCenter
        color: Cpp_ThemeManager.colors["widget_text"]
        font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.85, false))
        text: {
          if (root.cursorAVisible && root.cursorBVisible) {
            const dx = root.timeAxis ? root.timeAgoLabel(root.deltaX) : root.deltaX.toFixed(root.xPrecision)
            return qsTr("ΔX: %1  ΔY: %2 — Drag to move, right-click to clear").arg(dx).arg(root.deltaY.toFixed(root.yPrecision))
          }
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

  //
  // Sweep-mode trigger indicator: dashed cursor-B-styled level line
  //
  Item {
    id: _triggerLine

    clip: true
    width: _graph.plotArea.width
    height: _graph.plotArea.height
    x: _graph.x + _graph.plotArea.x
    y: _graph.y + _graph.plotArea.y
    visible: root.sweepMode && root.triggerEditing

    readonly property color lineColor: Cpp_ThemeManager.colors["plot_cursor_b"]
    readonly property real lineY: root.worldToPixelY(root.triggerLevel)
    readonly property bool inRange: root.triggerLevel >= root.yVisibleMin
                                    && root.triggerLevel <= root.yVisibleMax

    //
    // Restart the fade; only meaningful while the dialog is editing
    //
    function flash() {
      if (!root.sweepMode || !root.triggerEditing)
        return

      _fade.stop()
      _triggerContent.opacity = 1
      _fade.start()
    }

    onVisibleChanged: {
      if (visible)
        flash()
    }

    Connections {
      target: root
      enabled: root.triggerEditing
      function onTriggerLevelChanged() { _triggerLine.flash() }
    }

    Item {
      id: _triggerContent

      anchors.fill: parent
      visible: _triggerLine.inRange

      SequentialAnimation {
        id: _fade

        PauseAnimation { duration: 2500 }
        NumberAnimation {
          to: 0
          duration: 750
          property: "opacity"
          target: _triggerContent
          easing.type: Easing.InOutQuad
        }
      }

      Canvas {
        id: _triggerCanvas

        x: 0
        height: 2
        width: parent.width
        y: _triggerLine.lineY - 1

        onPaint: {
          var ctx = getContext("2d")
          ctx.clearRect(0, 0, width, height)
          ctx.strokeStyle = _triggerLine.lineColor
          ctx.lineWidth = 2
          ctx.setLineDash([8, 4, 2, 4])
          ctx.lineDashOffset = 0

          ctx.beginPath()
          ctx.moveTo(0, 1)
          ctx.lineTo(width, 1)
          ctx.stroke()
        }

        onWidthChanged: requestPaint()

        Connections {
          target: _triggerLine
          function onLineColorChanged() { _triggerCanvas.requestPaint() }
        }
      }

      Label {
        text: "T"
        padding: 4
        color: Cpp_ThemeManager.colors["widget_base"]
        font: (Cpp_Misc_CommonFonts.widgetFontRevision,
               Cpp_Misc_CommonFonts.widgetFont(0.9, true))
        background: Rectangle {
          radius: 3
          opacity: 0.9
          color: _triggerLine.lineColor
        }
        anchors {
          rightMargin: 5
          right: parent.right
          verticalCenter: parent.top
          verticalCenterOffset: _triggerLine.lineY
        }
      }
    }
  }
}
