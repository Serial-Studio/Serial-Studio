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

import "../"
import "../../Dialogs" as Dialogs

Item {
  id: root

  clip: true

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property PlotModel model
  required property string widgetId

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Custom properties
  //
  property bool showAreaUnderPlot: true
  property int interpolationMode: SerialStudio.InterpolationLinear

  //
  // Bipolarity keys off the real samples, not the axis range: a strictly-positive
  // signal stays a solid mountain even when the configured range spans zero.
  //
  readonly property bool bipolarRange: root.model && root.model.dataBipolar
  readonly property bool areaFillVisible: root.showAreaUnderPlot
       && !(root.model && root.model.xyPlot)
       && root.interpolationMode !== SerialStudio.InterpolationNone
       && root.interpolationMode !== SerialStudio.InterpolationStem
  readonly property real areaFillBaseline: {
    if (!root.model)
      return 0

    if (root.bipolarRange)
      return 0

    return root.model.dataMaxY <= 0 ? root.model.maxY : root.model.minY
  }

  //
  // User-controlled visibility preferences (persisted, ANDed with size thresholds)
  //
  property bool userShowXLabel: true
  property bool userShowYLabel: true

  //
  // Sweep/trigger mode is a time-axis-only Pro feature
  //
  readonly property bool sweepAllowed: root.model && root.model.timeAxis && Cpp_CommercialBuild
                                       && (Cpp_Licensing_LemonSqueezy.isActivated
                                           || Cpp_Licensing_Trial.trialEnabled)

  //
  // Suppresses the sweepChanged auto-save while restore assigns persisted values
  //
  property bool restoringSweep: false

  //
  // Cleared first during teardown so the UI-timer Connection detaches before the
  // dynamically-created widget's context is invalidated (singleton keeps firing)
  //
  property bool alive: true
  Component.onDestruction: root.alive = false

  PlotCommon {
    id: plotCommon
  }

  //
  // Sync model width/height with widget, then restore persisted settings
  //
  Component.onCompleted: {
    plotCommon.setDownsampleFactor(plot, model)

    const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

    if (s["interpolationMode"] !== undefined)
      root.interpolationMode = plotCommon.normalizeInterpolationMode(s["interpolationMode"])
    else if (s["interpolate"] !== undefined)
      root.interpolationMode = s["interpolate"]
        ? SerialStudio.InterpolationLinear
        : SerialStudio.InterpolationNone

    if (root.model)
      root.model.interpolationMode = root.interpolationMode

    if (s["showAreaUnderPlot"] !== undefined)
      root.showAreaUnderPlot = s["showAreaUnderPlot"]

    if (!plotCommon.canShowAreaUnderPlot(root.interpolationMode))
      root.showAreaUnderPlot = false

    if (s["userShowXLabel"] !== undefined)
      root.userShowXLabel = s["userShowXLabel"]

    if (s["userShowYLabel"] !== undefined)
      root.userShowYLabel = s["userShowYLabel"]

    root.restoringSweep = true

    if (s["sweepMode"] !== undefined)
      root.model.sweepMode = s["sweepMode"]

    if (s["triggerEdge"] !== undefined)
      root.model.triggerEdge = s["triggerEdge"]

    if (s["triggerLevel"] !== undefined)
      root.model.triggerLevel = s["triggerLevel"]

    if (s["holdoff"] !== undefined)
      root.model.holdoff = s["holdoff"]

    if (s["sweepTimebase"] !== undefined)
      root.model.sweepTimebase = s["sweepTimebase"]

    if (root.sweepAllowed && s["sweepEnabled"] !== undefined)
      root.model.sweepEnabled = s["sweepEnabled"]

    root.restoringSweep = false
  }

  //
  // Persist trigger settings whenever they change
  //
  function saveSweepSettings() {
    if (root.restoringSweep)
      return

    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "sweepMode", root.model.sweepMode)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "triggerEdge", root.model.triggerEdge)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "triggerLevel", root.model.triggerLevel)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "holdoff", root.model.holdoff)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "sweepTimebase", root.model.sweepTimebase)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "sweepEnabled", root.model.sweepEnabled)
  }

  //
  // Reset trigger/sweep settings to defaults when sweep mode is toggled
  //
  function resetSweepSettings() {
    if (!root.model)
      return

    root.model.sweepMode = SerialStudio.SweepAuto
    root.model.triggerEdge = SerialStudio.TriggerRising
    root.model.triggerLevel = 0
    root.model.holdoff = 0
    root.model.sweepTimebase = 0
  }

  Connections {
    target: root.model

    function onSweepChanged() {
      root.saveSweepSettings()
    }
  }

  //
  // Enable/disable features depending on window size, ANDed with user preferences
  //
  onWidthChanged: updateWidgetOptions()
  onHeightChanged: updateWidgetOptions()
  onInterpolationModeChanged: {
    scatterSeries.clear()
    upperSeries.clear()
  }
  function updateWidgetOptions() {
    plot.yLabelVisible = root.userShowYLabel && (root.width >= 196)
    plot.xLabelVisible = root.userShowXLabel && (root.height >= (196 * 2/3))
    root.hasToolbar = (root.width >= toolbar.implicitWidth) && (root.height >= 220)
  }

  //
  // Axis range configuration dialog
  //
  Dialogs.AxisRangeDialog {
    id: axisRangeDialog
  }

  //
  // Trigger configuration dialog
  //
  Dialogs.TriggerDialog {
    id: triggerDialog
  }

  //
  // Re-downsamples the visible window and redraws the active series; zoom/pan call it
  // via Qt.callLater because draw() emits rangeChanged (binding loop if synchronous)
  //
  function redrawCurves() {
    if (!root.visible || !root.model
        || typeof plotCommon.setDownsampleFactor !== "function")
      return

    plotCommon.setDownsampleFactor(plot, model)
    if (root.interpolationMode === SerialStudio.InterpolationNone)
      root.model.draw(scatterSeries)
    else
      root.model.draw(upperSeries)
  }

  //
  // Guards the plotCommon call so geometry signals firing during teardown (anchors
  // detaching) don't dereference the child QtObject after its context is invalid
  //
  function setDownsample() {
    if (root.model && typeof plotCommon.setDownsampleFactor === "function")
      plotCommon.setDownsampleFactor(plot, model)
  }

  //
  // Update curve at the UI refresh rate (60 Hz default, Settings-configurable)
  //
  Connections {
    enabled: root.alive
    target: Cpp_Misc_TimerEvents

    function onUiTimeout() {
      root.redrawCurves()
    }
  }

  //
  // Add toolbar
  //
  RowLayout {
    id: toolbar

    spacing: 4
    visible: root.hasToolbar
    height: root.hasToolbar ? 48 : 0
    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Cpp_Misc_Translator.rtl

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    DashboardToolButton {
      onClicked: {
        root.interpolationMode = plotCommon.nextInterpolationMode(root.interpolationMode)
        if (root.model)
          root.model.interpolationMode = root.interpolationMode

        if (!plotCommon.canShowAreaUnderPlot(root.interpolationMode))
          root.showAreaUnderPlot = false

        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId,
                                                "interpolationMode",
                                                root.interpolationMode)
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showAreaUnderPlot", root.showAreaUnderPlot)
      }
      checked: root.interpolationMode !== SerialStudio.InterpolationNone
      ToolTip.text: qsTr("Interpolation: %1").arg(plotCommon.modeLabel(root.interpolationMode))
      icon.source: root.interpolationMode === SerialStudio.InterpolationNone
             ? "qrc:/icons/dashboard-buttons/interpolate-off.svg"
             : "qrc:/icons/dashboard-buttons/interpolate-on.svg"
    }

    DashboardToolButton {
      onClicked: {
        root.showAreaUnderPlot = !root.showAreaUnderPlot
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showAreaUnderPlot", root.showAreaUnderPlot)
      }
      opacity: enabled ? 1 : 0.5
      checked: root.showAreaUnderPlot
      ToolTip.text: qsTr("Show Area Under Plot")
      visible: !(root.model && root.model.xyPlot)
      icon.source: "qrc:/icons/dashboard-buttons/area.svg"
      enabled: plotCommon.canShowAreaUnderPlot(root.interpolationMode)
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      onClicked: {
        root.userShowXLabel = !root.userShowXLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowXLabel", root.userShowXLabel)
      }
      checked: root.userShowXLabel
      ToolTip.text: qsTr("Show X Axis Label")
      icon.source: "qrc:/icons/dashboard-buttons/x.svg"
    }

    DashboardToolButton {
      onClicked: {
        root.userShowYLabel = !root.userShowYLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowYLabel", root.userShowYLabel)
      }
      checked: root.userShowYLabel
      ToolTip.text: qsTr("Show Y Axis Label")
      icon.source: "qrc:/icons/dashboard-buttons/y.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      checked: plot.showCrosshairs
      ToolTip.text: qsTr("Show Crosshair")
      onClicked: plot.showCrosshairs = !plot.showCrosshairs
      icon.source: "qrc:/icons/dashboard-buttons/crosshair.svg"
    }

    DashboardToolButton {
      checked: !model.running
      ToolTip.text: model.running ? qsTr("Pause") : qsTr("Resume")
      icon.source: model.running?
                     "qrc:/icons/dashboard-buttons/pause.svg" :
                     "qrc:/icons/dashboard-buttons/resume.svg"
      onClicked: model.running = !model.running
    }

    Rectangle {
      visible: root.sweepAllowed
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      visible: root.sweepAllowed
      checked: root.model.sweepEnabled
      ToolTip.text: qsTr("Sweep / Trigger Mode")
      icon.source: "qrc:/icons/dashboard-buttons/sweep.svg"
      onClicked: {
        root.model.sweepEnabled = !root.model.sweepEnabled
        root.resetSweepSettings()
      }
    }

    DashboardToolButton {
      visible: root.sweepAllowed
      opacity: enabled ? 1 : 0.5
      enabled: root.model.sweepEnabled
      ToolTip.text: qsTr("Trigger Settings")
      icon.source: "qrc:/icons/dashboard-buttons/trigger.svg"
      onClicked: triggerDialog.openDialog(root.model, false)
    }

    Rectangle {
      visible: root.sweepAllowed
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      onClicked: {
        plot.xAxis.pan = 0
        plot.yAxis.pan = 0
        plot.xAxis.zoom = 1
        plot.yAxis.zoom = 1
        plot.xMin = Qt.binding(function() { return root.model.minX })
        plot.xMax = Qt.binding(function() { return root.model.maxX })
        plot.yMin = Qt.binding(function() { return root.model.minY })
        plot.yMax = Qt.binding(function() { return root.model.maxY })
      }
      opacity: enabled ? 1 : 0.5
      ToolTip.text: qsTr("Reset View")
      icon.source: "qrc:/icons/dashboard-buttons/return.svg"
      enabled: plot.xAxis.zoom !== 1 || plot.yAxis.zoom !== 1
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Axis Range Settings")
      icon.source: "qrc:/icons/toolbar/settings.svg"
      onClicked: axisRangeDialog.openDialog(plot, root.model)
    }

    Item {
      Layout.fillWidth: true
    }
  }

  //
  // Plot widget
  //
  PlotWidget {
    id: plot

    anchors {
      margins: 8
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
    }

    xMin: root.model.minX
    xMax: root.model.maxX
    yMin: root.model.minY
    yMax: root.model.maxY
    curveColors: [root.color]
    xLabel: root.model.xLabel
    yLabel: root.model.yLabel
    timeAxis: root.model.timeAxis
    sweepMode: root.model.sweepEnabled
    triggerLevel: root.model.triggerLevel
    triggerEditing: triggerDialog.visible
    mouseAreaEnabled: windowRoot.focused
    xAxis.tickInterval: plot.xTickInterval
    yAxis.tickInterval: plot.yTickInterval

    areaFillColor: root.color
    areaFillBaseline: root.areaFillBaseline
    areaFillSource: root.areaFillVisible ? upperSeries : null

    onZoomChanged: Qt.callLater(root.redrawCurves)
    onXVisibleMinChanged: Qt.callLater(root.redrawCurves)
    onWidthChanged: root.setDownsample()
    onHeightChanged: root.setDownsample()
    onTriggerLevelChangeRequested: (level) => {
      if (root.model)
        root.model.triggerLevel = level
    }

    Connections {
      target: root.windowRoot
      function onFocusedChanged() {
        plot.mouseAreaEnabled = root.windowRoot.focused
      }
    }

    Component.onCompleted: graph.addSeries(scatterSeries)

    ScatterSeries {
      id: scatterSeries

      visible: root.interpolationMode === SerialStudio.InterpolationNone
      pointDelegate: Rectangle {
        width: 2
        height: 2
        radius: 1
        color: root.color
      }
    }

    //
    // Data carrier only (never added to the graph): the model draws into it and
    // the GPU PlotCurve below renders it
    //
    LineSeries {
      id: upperSeries
    }

    PlotCurve {
      lineWidth: 2
      color: root.color
      source: upperSeries
      anchors.fill: parent
      xMin: plot.xVisibleMin
      xMax: plot.xVisibleMax
      yMin: plot.yVisibleMin
      yMax: plot.yVisibleMax
      parent: plot.curveLayer
      visible: root.interpolationMode !== SerialStudio.InterpolationNone
    }
  }
}
