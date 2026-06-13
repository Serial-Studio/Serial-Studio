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

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property FFTPlotModel model

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
  // User-controlled visibility preferences (persisted, ANDed with size thresholds)
  //
  property bool userShowXLabel: true
  property bool userShowYLabel: true

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

    if (s["showAreaUnderPlot"] !== undefined)
      root.showAreaUnderPlot = s["showAreaUnderPlot"]

    if (s["interpolationMode"] !== undefined)
      root.interpolationMode = plotCommon.normalizeInterpolationMode(s["interpolationMode"])

    if (!plotCommon.canShowAreaUnderPlot(root.interpolationMode))
      root.showAreaUnderPlot = false

    if (root.model)
      root.model.interpolationMode = root.interpolationMode

    if (s["userShowXLabel"] !== undefined)
      root.userShowXLabel = s["userShowXLabel"]

    if (s["userShowYLabel"] !== undefined)
      root.userShowYLabel = s["userShowYLabel"]
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
  // Guards the plotCommon call so geometry/zoom signals firing during teardown (anchors
  // detaching) don't dereference the child QtObject after its context is invalid
  //
  function setDownsample() {
    if (root.model && typeof plotCommon.setDownsampleFactor === "function")
      plotCommon.setDownsampleFactor(plot, model)
  }

  //
  // Axis range configuration dialog
  //
  Dialogs.AxisRangeDialog {
    id: axisRangeDialog
  }

  //
  // Update curve at the UI refresh rate (60 Hz default, Settings-configurable)
  //
  Connections {
    enabled: root.alive
    target: Cpp_Misc_TimerEvents

    function onUiTimeout() {
      if (root.visible && root.model) {
        if (root.interpolationMode === SerialStudio.InterpolationNone)
          root.model.draw(scatterSeries)
        else
          root.model.draw(upperSeries)
      }
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
      checked: root.interpolationMode !== SerialStudio.InterpolationNone
      ToolTip.text: qsTr("Interpolation: %1").arg(plotCommon.modeLabel(root.interpolationMode))
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
      icon.source: root.interpolationMode === SerialStudio.InterpolationNone
                     ? "qrc:/icons/dashboard-buttons/interpolate-off.svg"
                     : "qrc:/icons/dashboard-buttons/interpolate-on.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      opacity: enabled ? 1 : 0.5
      checked: root.showAreaUnderPlot
      ToolTip.text: qsTr("Show Area Under Plot")
      icon.source: "qrc:/icons/dashboard-buttons/area.svg"

      onClicked: {
        root.showAreaUnderPlot = !root.showAreaUnderPlot
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showAreaUnderPlot", root.showAreaUnderPlot)
      }
      enabled: plotCommon.canShowAreaUnderPlot(root.interpolationMode)
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      checked: root.userShowXLabel
      ToolTip.text: qsTr("Show X Axis Label")
      icon.source: "qrc:/icons/dashboard-buttons/x.svg"

      onClicked: {
        root.userShowXLabel = !root.userShowXLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowXLabel", root.userShowXLabel)
      }
    }

    DashboardToolButton {
      checked: root.userShowYLabel
      ToolTip.text: qsTr("Show Y Axis Label")
      icon.source: "qrc:/icons/dashboard-buttons/y.svg"

      onClicked: {
        root.userShowYLabel = !root.userShowYLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowYLabel", root.userShowYLabel)
      }
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
    yLabel: qsTr("Magnitude (dB)")
    xLabel: qsTr("Frequency (Hz)")
    mouseAreaEnabled: windowRoot.focused
    xAxis.tickInterval: plot.xTickInterval
    yAxis.tickInterval: plot.yTickInterval

    areaFillColor: root.color
    areaFillBaseline: root.model.minY
    areaFillSource: root.showAreaUnderPlot
         && root.interpolationMode !== SerialStudio.InterpolationNone
         && root.interpolationMode !== SerialStudio.InterpolationStem
                    ? upperSeries : null

    onZoomChanged: root.setDownsample()
    onWidthChanged: root.setDownsample()
    onHeightChanged: root.setDownsample()

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
