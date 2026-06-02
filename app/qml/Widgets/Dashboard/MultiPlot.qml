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
  required property MultiPlotModel model

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Custom properties
  //
  property bool showLegends: true
  property int interpolationMode: SerialStudio.InterpolationLinear

  //
  // User-controlled visibility preferences (persisted, ANDed with size thresholds)
  //
  property bool userShowXLabel: true
  property bool userShowYLabel: true
  property bool userShowLegends: true

  //
  // Sweep/trigger mode is a time-axis-only Pro feature
  //
  readonly property bool sweepAllowed: root.model && root.model.timeAxis && Cpp_CommercialBuild
                                       && (Cpp_Licensing_LemonSqueezy.isActivated
                                           || Cpp_Licensing_Trial.trialEnabled)

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

    if (s["userShowLegends"] !== undefined)
      root.userShowLegends = s["userShowLegends"]

    if (s["userShowXLabel"] !== undefined)
      root.userShowXLabel = s["userShowXLabel"]

    if (s["userShowYLabel"] !== undefined)
      root.userShowYLabel = s["userShowYLabel"]

    if (s["visibleCurves"] !== undefined) {
      const curves = s["visibleCurves"]
      if (curves !== undefined && curves !== null) {
        for (let i = 0; i < curves.length && i < root.model.count; ++i)
          root.model.modifyCurveVisibility(i, curves[i])
      }
    }

    if (s["sweepMode"] !== undefined)
      root.model.sweepMode = s["sweepMode"]

    if (s["triggerEdge"] !== undefined)
      root.model.triggerEdge = s["triggerEdge"]

    if (s["triggerLevel"] !== undefined)
      root.model.triggerLevel = s["triggerLevel"]

    if (s["triggerSource"] !== undefined)
      root.model.triggerSource = s["triggerSource"]

    if (s["holdoff"] !== undefined)
      root.model.holdoff = s["holdoff"]

    if (s["sweepTimebase"] !== undefined)
      root.model.sweepTimebase = s["sweepTimebase"]

    if (root.sweepAllowed && s["sweepEnabled"] !== undefined)
      root.model.sweepEnabled = s["sweepEnabled"]
  }

  //
  // Persist trigger settings whenever they change
  //
  function saveSweepSettings() {
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "sweepMode", root.model.sweepMode)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "triggerEdge", root.model.triggerEdge)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "triggerLevel", root.model.triggerLevel)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "triggerSource", root.model.triggerSource)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "holdoff", root.model.holdoff)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "sweepTimebase", root.model.sweepTimebase)
    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "sweepEnabled", root.model.sweepEnabled)
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
    const count = plot.graph.seriesList.length
    for (let i = 0; i < count; ++i)
      plot.graph.seriesList[i].clear()
  }
  function updateWidgetOptions() {
    root.showLegends = root.userShowLegends && (root.width >= 320)
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
  // Update widget at 24 Hz
  //
  Connections {
    target: Cpp_Misc_TimerEvents

    function onUiTimeout() {
      if (root.visible) {
        root.model.updateData()

        const count = plot.graph.seriesList.length
        for (let i = 0; i < count; ++i) {
          let ptr = plot.graph.seriesList[i]
          if (ptr.curveVisible)
            root.model.draw(ptr, ptr.curveIndex)
          else
            ptr.clear()
        }
      }
    }
  }

  //
  // Re-draw whole plot when curves changes
  //
  Connections {
    target: root.model

    function onCurvesChanged() {
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

        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId,
                                                "interpolationMode",
                                                root.interpolationMode)
      }
      icon.source: root.interpolationMode === SerialStudio.InterpolationNone
                     ? "qrc:/icons/dashboard-buttons/interpolate-off.svg"
                     : "qrc:/icons/dashboard-buttons/interpolate-on.svg"
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
      onClicked: root.model.sweepEnabled = !root.model.sweepEnabled
    }

    DashboardToolButton {
      visible: root.sweepAllowed
      opacity: enabled ? 1 : 0.5
      enabled: root.model.sweepEnabled
      ToolTip.text: qsTr("Trigger Settings")
      icon.source: "qrc:/icons/dashboard-buttons/trigger.svg"
      onClicked: triggerDialog.openDialog(root.model, true)
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
  // Plot + Legends
  //
  RowLayout {
    spacing: 4

    anchors {
      margins: 8
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
    }

    //
    // Plot widget
    //
    PlotWidget {
      id: plot

      xMin: root.model.minX
      xMax: root.model.maxX
      yMin: root.model.minY
      yMax: root.model.maxY
      Layout.fillWidth: true
      Layout.fillHeight: true
      xLabel: root.model.xLabel
      yLabel: root.model.yLabel
      timeAxis: root.model.timeAxis
      curveColors: root.model.colors
      sweepMode: root.model.sweepEnabled
      mouseAreaEnabled: windowRoot.focused
      triggerLevel: root.model.triggerLevel
      triggerEditing: triggerDialog.visible
      xAxis.tickInterval: plot.xTickInterval
      yAxis.tickInterval: plot.yTickInterval

      onZoomChanged: plotCommon.setDownsampleFactor(plot, model)
      onWidthChanged: plotCommon.setDownsampleFactor(plot, model)
      onHeightChanged: plotCommon.setDownsampleFactor(plot, model)

      //
      // Register line series
      //
      Instantiator {
        model: root.model.count
        delegate: LineSeries {
          property int curveIndex: index
          property bool curveVisible: root.interpolationMode !== SerialStudio.InterpolationNone
                                     && root.model.visibleCurves[index]
          Component.onCompleted: plot.graph.addSeries(this)
        }
      }

      //
      // Register scatter series
      //
      Instantiator {
        model: root.model.count
        delegate: ScatterSeries {
          property int curveIndex: index
          property bool curveVisible: root.interpolationMode === SerialStudio.InterpolationNone
                                     && root.model.visibleCurves[index]
          Component.onCompleted: plot.graph.addSeries(this)
          pointDelegate: Rectangle {
            width: 2
            height: 2
            radius: 1
            color: root.model.colors[index]
          }
        }
      }
    }

    //
    // Legends widget
    //
    Item {
      opacity: plot.opacity
      Layout.fillHeight: true
      enabled: root.model.count > 1
      visible: enabled && root.showLegends
      implicitWidth: _legends.implicitWidth + 8

      Rectangle {
        width: parent.width
        anchors.centerIn: parent
        height: Math.min(_legends.implicitHeight + 8, parent.height)

        border.width: 1
        color: Cpp_ThemeManager.colors["widget_base"]
        border.color: Cpp_ThemeManager.colors["widget_border"]

        Flickable {
          clip: true
          anchors.margins: 4
          contentWidth: width
          anchors.fill: parent
          contentHeight: _legends.implicitHeight

          ScrollBar.vertical: ScrollBar {
            id: scroll
          }

          ColumnLayout {
            id: _legends

            spacing: 4
            width: parent.width

            Repeater {
              model: root.model.count
              delegate: Switch {
                onCheckedChanged: {
                  if (checked !== root.model.visibleCurves[index]) {
                    root.model.modifyCurveVisibility(index, checked)
                    Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "visibleCurves",
                                                            root.model.visibleCurves)
                  }
                }
                Layout.fillWidth: true
                text: root.model.labels[index]
                Layout.alignment: Qt.AlignVCenter
                checked: root.model.visibleCurves[index]
                palette.highlight: root.model.colors[index]
                palette.text: Cpp_ThemeManager.colors["widget_text"]
                font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))
              }
            }
          }
        }
      }
    }
  }
}
