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
import QtGraphs
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "PlotWidget" as Parts

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
  // X-axis ruler (spec 0058): named markers, a zero point every X label and cursor readout
  // becomes relative to, and a hover marker; the host persists them on rulerChanged()
  //
  property var xMarkers: []
  property real xZero: 0
  property bool xZeroSet: false
  property bool hoverMarkerEnabled: false
  signal rulerChanged()

  function addMarker(worldX, name) {
    const list = root.xMarkers.slice()
    list.push({ "x": worldX, "name": name })
    root.xMarkers = list
    root.rulerChanged()
  }

  function removeMarker(index) {
    if (index < 0 || index >= root.xMarkers.length)
      return

    const list = root.xMarkers.slice()
    list.splice(index, 1)
    root.xMarkers = list
    root.rulerChanged()
  }

  function clearMarkers() {
    root.xMarkers = []
    root.rulerChanged()
  }

  function setZeroAt(worldX) {
    root.xZero = worldX
    root.xZeroSet = true
    root.rulerChanged()
  }

  function resetZero() {
    root.xZero = 0
    root.xZeroSet = false
    root.rulerChanged()
  }

  function setHoverMarker(enabled) {
    root.hoverMarkerEnabled = enabled
    root.rulerChanged()
  }

  //
  // Restores the ruler from a widgetSettings object (missing keys keep the defaults)
  //
  function restoreRuler(settings) {
    if (Array.isArray(settings["xMarkers"]))
      root.xMarkers = settings["xMarkers"].filter(function(m) {
        return m && isFinite(m.x) && typeof m.name === "string"
      })

    if (settings["xZeroSet"] === true && isFinite(settings["xZero"])) {
      root.xZero = settings["xZero"]
      root.xZeroSet = true
    }

    if (settings["hoverMarker"] !== undefined)
      root.hoverMarkerEnabled = settings["hoverMarker"] === true
  }

  //
  // Nearest marker to a plot-area pixel X, or -1 when none is within the hit threshold
  //
  function markerNear(pixelX) {
    let best = -1
    let bestDist = 8
    for (let i = 0; i < root.xMarkers.length; ++i) {
      const d = Math.abs(root.worldToPixelX(root.xMarkers[i].x) - pixelX)
      if (d <= bestDist) {
        bestDist = d
        best = i
      }
    }

    return best
  }

  //
  // X value relative to the ruler zero (identity on log axes, which have no zero point)
  //
  function relativeX(worldX) {
    return (root.xZeroSet && !root.logX) ? worldX - root.xZero : worldX
  }

  //
  // Minor grid lines sit between the plot background and the border color so the fine
  // subdivisions read as texture instead of competing with the majors
  //
  function fadedGrid(strength) {
    const border = Qt.color(Cpp_ThemeManager.colors["widget_border"])
    return Qt.tint(Cpp_ThemeManager.colors["widget_base"],
                   Qt.rgba(border.r, border.g, border.b, strength))
  }

  //
  // Tick fitting and every label/readout formatter: PlotFormatter owns the math and the
  // font metrics it measures with, the plot keeps the names its callers already use.
  //
  Parts.PlotFormatter {
    id: _format

    plot: root
  }

  function xTickLabel(value, interval) {
    return _format.xTickLabel(value, interval)
  }

  function yTickLabel(value, interval) {
    return _format.yTickLabel(value, interval)
  }

  function safeTickAnchor(desired, min, max) {
    return _format.safeTickAnchor(desired, min, max)
  }

  function subTicksFor(interval, logAxis) {
    return _format.subTicksFor(interval, logAxis)
  }

  function tickDecimals(interval) {
    return _format.tickDecimals(interval)
  }

  function engineeringFormat(value, tickInterval) {
    return _format.engineeringFormat(value, tickInterval)
  }

  function logTickFormat(logValue, logStep) {
    return _format.logTickFormat(logValue, logStep)
  }

  function displayValueX(worldX) {
    return _format.displayValueX(worldX)
  }

  function displayValueY(worldY) {
    return _format.displayValueY(worldY)
  }

  function cursorReadout(widthPx) {
    return _format.cursorReadout(widthPx)
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
  // Settled plot-area extent for the tick math: reading live plotArea (which the interval resizes
  // via the label containers) spins QtGraphs' polish; this debounced copy breaks the loop.
  //
  property real settledPlotW: 0
  property real settledPlotH: 0
  property bool plotSettlePending: false
  function schedulePlotAreaSettle() {
    if (root.plotSettlePending)
      return

    root.plotSettlePending = true
    Qt.callLater(root.applyPlotAreaSettle)
  }
  function applyPlotAreaSettle() {
    root.plotSettlePending = false
    const w = _graph.plotArea.width
    const h = _graph.plotArea.height
    if (Math.abs(w - root.settledPlotW) > 1)
      root.settledPlotW = w

    if (Math.abs(h - root.settledPlotH) > 1)
      root.settledPlotH = h
  }

  //
  // Dynamic tick intervals based on visible range and available space
  //
  readonly property real xTickInterval: _format.smartIntervalX(xVisibleMin, xVisibleMax)
  readonly property real yTickInterval: _format.smartIntervalY(yVisibleMin, yVisibleMax)

  //
  // Custom properties
  //
  property bool logX: false
  property bool logY: false
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
  // Readout precision derived from the tick period the axis settled on
  //
  readonly property int xPrecision: _format.precisionForInterval(xTickInterval)
  readonly property int yPrecision: _format.precisionForInterval(yTickInterval)

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

  //
  // Plot widget
  //
  GraphsView {
    id: _graph

    onPlotAreaChanged: root.schedulePlotAreaSettle()

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
      axisX.subColor: root.fadedGrid(0.4)
      axisY.subColor: root.fadedGrid(0.4)
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
      grid.subColor: root.fadedGrid(0.4)
      grid.mainColor: root.fadedGrid(0.75)

      //
      // Highlight colors for better contrast
      //
      multiHighlightColor: Cpp_ThemeManager.colors["widget_highlight"]
      singleHighlightColor: Cpp_ThemeManager.colors["widget_highlighted_text"]

      //
      // Reapplies every themed color from the active palette.
      //
      function refreshColors() {
        _theme.borderColors = [Cpp_ThemeManager.colors["widget_border"]]
        _theme.backgroundColor = Cpp_ThemeManager.colors["widget_window"]
        _theme.plotAreaBackgroundColor = Cpp_ThemeManager.colors["widget_base"]
        _theme.axisX.subColor = root.fadedGrid(0.4)
        _theme.axisY.subColor = root.fadedGrid(0.4)
        _theme.axisX.mainColor = Cpp_ThemeManager.colors["widget_border"]
        _theme.axisY.mainColor = Cpp_ThemeManager.colors["widget_border"]
        _theme.axisX.labelTextColor = Cpp_ThemeManager.colors["widget_text"]
        _theme.axisY.labelTextColor = Cpp_ThemeManager.colors["widget_text"]
        _theme.labelTextColor = Cpp_ThemeManager.colors["widget_text"]
        _theme.grid.subColor = root.fadedGrid(0.4)
        _theme.grid.mainColor = root.fadedGrid(0.75)
        _theme.multiHighlightColor = Cpp_ThemeManager.colors["widget_highlight"]
        _theme.singleHighlightColor = Cpp_ThemeManager.colors["widget_highlighted_text"]
      }

      //
      // QtGraphs can paint a stale light/dark pass on a theme switch, so reapply the colors
      // immediately and once more after the transition settles to clear residual artifacts.
      //
      Timer {
        id: _themeRefreshTimer

        interval: 500
        repeat: false
        onTriggered: _theme.refreshColors()
      }

      Connections {
        target: Cpp_ThemeManager
        function onThemeChanged() {
          _theme.refreshColors()
          _themeRefreshTimer.restart()
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
      visible: root.yLabelVisible
      tickInterval: root.yTickInterval
      labelDecimals: root.tickDecimals(root.yTickInterval)
      tickAnchor: root.safeTickAnchor(root.logY ? Math.ceil(root.yMin) : root.yMin,
                                      root.yMin, root.yMax)
      subTickCount: root.subTicksFor(root.yTickInterval, root.logY)

      labelDelegate: Item {
        id: _yLabelItem

        property string text

        implicitWidth:  _yEngLabel.implicitWidth
        implicitHeight: _yEngLabel.implicitHeight

        Text {
          id: _yEngLabel

          anchors.centerIn: parent
          text: root.logY
                ? root.logTickFormat(parseFloat(_yLabelItem.text), root.yTickInterval)
                : root.engineeringFormat(parseFloat(_yLabelItem.text), root.yTickInterval)
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
      visible: root.xLabelVisible
      tickInterval: root.xTickInterval
      labelDecimals: root.tickDecimals(root.xTickInterval)
      subTickCount: root.subTicksFor(root.xTickInterval, root.logX)
      tickAnchor: root.safeTickAnchor(root.logX ? Math.ceil(root.xMin)
                                                : (root.xZeroSet ? root.xZero : root.xMin),
                                      root.xMin, root.xMax)

      labelDelegate: Item {
        id: _xLabelItem

        property string text

        implicitWidth:  _xEngLabel.implicitWidth
        implicitHeight: _xEngLabel.implicitHeight

        Text {
          id: _xEngLabel

          anchors.centerIn: parent
          text: root.xTickLabel(parseFloat(_xLabelItem.text), root.xTickInterval)
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
    // Pointer handling: pan/zoom, cursor placement, the trigger grab and the ruler menu
    //
    Parts.PlotInteraction {
      id: _overlayMouse

      plot: root
      anchors.fill: parent

      onTriggerFlashRequested: _triggerLine.flash()
      onTriggerLevelRequested: (level) => root.triggerLevelChangeRequested(level)
      onRulerMenuRequested: (worldX, markerIndex) => {
        _rulerMenu.pressWorldX = worldX
        _rulerMenu.pressMarker = markerIndex
        _rulerMenu.popup()
      }
    }

    //
    // Ruler chrome: the zero line, the named markers and the hover marker
    //
    Parts.PlotRulerOverlay {
      plot: root
      anchors.fill: parent
      pointerX: _overlayMouse.mouseX
      pointerActive: _overlayMouse.containsMouse && !_overlayMouse.dragging
    }

    //
    // Ruler context menu (right-click outside cursor mode)
    //
    Parts.PlotRulerMenu {
      id: _rulerMenu

      plot: root
      onAddMarkerRequested: (worldX) => _markerNamePopup.openAt(worldX)
    }

    //
    // Marker name prompt
    //
    Parts.PlotMarkerPopup {
      id: _markerNamePopup

      plot: root
    }

    //
    // Cursor A: crosshair, chip and axis readouts (in view = cursor mode + in range)
    //
    Parts.PlotCursor {
      name: "A"
      plot: root
      anchors.fill: parent
      worldX: root.cursorAX
      worldY: root.cursorAY
      inView: root.cursorAInView
      lineColor: root.cursorAColor
      xInRange: root.cursorAXInRange
      yInRange: root.cursorAYInRange
      textColor: root.cursorATextColor
      cursorVisible: root.cursorAVisible
    }

    //
    // Cursor B: same chrome, second measurement point of the delta pair
    //
    Parts.PlotCursor {
      name: "B"
      plot: root
      anchors.fill: parent
      worldX: root.cursorBX
      worldY: root.cursorBY
      inView: root.cursorBInView
      lineColor: root.cursorBColor
      xInRange: root.cursorBXInRange
      yInRange: root.cursorBYInRange
      textColor: root.cursorBTextColor
      cursorVisible: root.cursorBVisible
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
        text: {
          const base = root.timeAxis ? qsTr("Time") : root.xLabel
          const title = (root.xZeroSet && !root.logX) ? qsTr("%1 from zero").arg(base) : base
          return root.timeAxis ? (title + " (" + root.timeUnitName + ")") : title
        }
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
          if (root.cursorAVisible && root.cursorBVisible)
            return root.cursorReadout(_layout.width)
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
  Parts.PlotTriggerLine {
    id: _triggerLine

    plot: root
    width: _graph.plotArea.width
    height: _graph.plotArea.height
    x: _graph.x + _graph.plotArea.x
    y: _graph.y + _graph.plotArea.y
  }
}
