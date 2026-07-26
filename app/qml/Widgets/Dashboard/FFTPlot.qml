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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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
  readonly property bool hasToolbar: toolbar.shown

  //
  // Custom properties
  //
  property bool showAreaUnderPlot: true
  property bool showFrequencyMarkers: true
  property int interpolationMode: SerialStudio.InterpolationLinear

  //
  // Transient marker spotlight: click a marker chip to emphasize it and dim the rest
  //
  property int selectedMarker: -1
  onShowFrequencyMarkersChanged: root.selectedMarker = -1

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

    if (s["showFrequencyMarkers"] !== undefined)
      root.showFrequencyMarkers = s["showFrequencyMarkers"]
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
  WidgetToolbar {
    id: toolbar

    windowRoot: root.windowRoot

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
                     ? Cpp_Misc_IconRegistry.icon("commands", "interpolate-off", 16)
                     : Cpp_Misc_IconRegistry.icon("commands", "interpolate-on", 16)
    }

    DashboardToolButton {
      opacity: enabled ? 1 : 0.5
      checked: root.showAreaUnderPlot
      ToolTip.text: qsTr("Show Area Under Plot")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "area", 16)

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
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "x", 16)

      onClicked: {
        root.userShowXLabel = !root.userShowXLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowXLabel", root.userShowXLabel)
      }
    }

    DashboardToolButton {
      checked: root.userShowYLabel
      ToolTip.text: qsTr("Show Y Axis Label")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "y", 16)

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
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "crosshair", 16)
    }

    DashboardToolButton {
      checked: root.showFrequencyMarkers
      ToolTip.text: qsTr("Show Frequency Markers")
      visible: root.model && root.model.markers.length > 0
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "labels", 16)
      onClicked: {
        root.showFrequencyMarkers = !root.showFrequencyMarkers
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId,
                                                "showFrequencyMarkers",
                                                root.showFrequencyMarkers)
      }
    }

    Rectangle {
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
      enabled: plot.xAxis.zoom !== 1 || plot.yAxis.zoom !== 1
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "return", 16)
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Axis Range Settings")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "settings", 32)
      onClicked: axisRangeDialog.openDialog(plot, root.model)
    }

    Rectangle {
      visible: Cpp_CommercialBuild
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    //
    // Record the widget's time-domain input to WAV (Pro)
    //
    DashboardToolButton {
      visible: Cpp_CommercialBuild
      ToolTip.text: qsTr("Record Audio")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "audio-file", 32)
      checked: Cpp_CommercialBuild && root.model && root.model.audioRecordingEnabled
      onClicked: {
        if (root.model)
          root.model.audioRecordingEnabled = !root.model.audioRecordingEnabled
      }
    }

    //
    // Reveal the folder holding this widget's recorded WAV files (Pro)
    //
    DashboardToolButton {
      visible: Cpp_CommercialBuild
      ToolTip.text: qsTr("Open Recordings Folder")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "sound-folder", 32)
      onClicked: {
        if (root.model)
          Cpp_Misc_Utilities.revealFile(root.model.recordingsFolder())
      }
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      checked: !model.running
      ToolTip.text: model.running ? qsTr("Pause") : qsTr("Resume")
      icon.source: model.running?
                     Cpp_Misc_IconRegistry.icon("commands", "pause", 16) :
                     Cpp_Misc_IconRegistry.icon("commands", "resume", 16)
      onClicked: model.running = !model.running
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
    logX: root.model.logX
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

    //
    // Frequency marker bands and lines, drawn under the spectrum curve (spec 0019).
    // Hz map to pixels through the same visible-window transform as PlotCurve.
    //
    Item {
      id: markerBandLayer

      z: -1
      anchors.fill: parent
      parent: plot.curveLayer
      visible: root.showFrequencyMarkers

      function xAt(freq) {
        const w = plot.logX ? Math.log10(Math.max(freq, 1e-12)) : freq
        return (w - plot.xVisibleMin) / plot.xVisibleRange * width
      }

      Repeater {
        model: root.model ? root.model.markers : []

        delegate: Item {
          id: markerShape

          required property int index
          required property var modelData

          property int markerState: 0

          readonly property bool isBand: modelData.endFreq > modelData.freq
          readonly property real xLo: markerBandLayer.xAt(modelData.freq)
          readonly property real xHi: isBand ? markerBandLayer.xAt(modelData.endFreq) : xLo
          readonly property bool spotlit: root.selectedMarker === index
          readonly property bool dimmed: root.selectedMarker >= 0 && !spotlit
          readonly property color baseColor: modelData.color !== "" ? modelData.color : root.color
          readonly property color liveColor: markerState === 2
                                             ? Cpp_ThemeManager.alarmColorForSeverity(3)
                                             : (markerState === 1
                                                ? Cpp_ThemeManager.alarmColorForSeverity(2)
                                                : baseColor)

          anchors.fill: parent
          opacity: dimmed ? 0.18 : 1
          visible: xHi >= 0 && xLo <= markerBandLayer.width

          Behavior on opacity {
            NumberAnimation {
              duration: 150
              easing.type: Easing.InOutQuad
            }
          }

          Connections {
            target: root.model
            enabled: markerShape.modelData.hasThresholds === true
            function onMarkerValuesChanged() {
              markerShape.markerState = root.model.markerState(markerShape.index)
            }
          }

          //
          // Band region: soft DAW-EQ style horizontal gradient bloom
          //
          Rectangle {
            y: 0
            x: markerShape.xLo
            height: parent.height
            visible: markerShape.isBand
            opacity: markerShape.markerState > 0 ? 1 : 0.85
            width: Math.max(1, markerShape.xHi - markerShape.xLo)

            gradient: Gradient {
              orientation: Gradient.Horizontal

              GradientStop {
                position: 0.0
                color: Qt.alpha(markerShape.liveColor, markerShape.spotlit ? 0.09 : 0.04)
              }

              GradientStop {
                position: 0.5
                color: Qt.alpha(markerShape.liveColor, markerShape.spotlit ? 0.34 : 0.20)
              }

              GradientStop {
                position: 1.0
                color: Qt.alpha(markerShape.liveColor, markerShape.spotlit ? 0.09 : 0.04)
              }
            }
          }

          //
          // Band edge strokes
          //
          Rectangle {
            y: 0
            x: markerShape.xLo
            height: parent.height
            visible: markerShape.isBand
            width: markerShape.spotlit ? 2 : 1
            color: Qt.alpha(markerShape.liveColor, markerShape.spotlit ? 0.8 : 0.45)
          }

          Rectangle {
            y: 0
            height: parent.height
            visible: markerShape.isBand
            width: markerShape.spotlit ? 2 : 1
            x: markerShape.xHi - (markerShape.spotlit ? 2 : 1)
            color: Qt.alpha(markerShape.liveColor, markerShape.spotlit ? 0.8 : 0.45)
          }

          //
          // Point marker: soft glow behind a 2 px line
          //
          Rectangle {
            y: 0
            width: 9
            height: parent.height
            x: markerShape.xLo - 4
            visible: !markerShape.isBand

            gradient: Gradient {
              orientation: Gradient.Horizontal

              GradientStop {
                position: 0.0
                color: "transparent"
              }

              GradientStop {
                position: 0.5
                color: Qt.alpha(markerShape.liveColor, markerShape.spotlit ? 0.32 : 0.18)
              }

              GradientStop {
                position: 1.0
                color: "transparent"
              }
            }
          }

          Rectangle {
            y: 0
            height: parent.height
            visible: !markerShape.isBand
            width: markerShape.spotlit ? 3 : 2
            x: markerShape.xLo - (markerShape.spotlit ? 1.5 : 1)
            color: Qt.alpha(markerShape.liveColor,
                            markerShape.spotlit
                            ? 1.0
                            : (markerShape.markerState > 0 ? 0.95 : 0.65))
          }
        }
      }
    }

    //
    // Clickable marker chips over the plot area: a click spotlights the marker, and chips
    // greedily drop one row when they would overlap horizontally
    //
    Item {
      id: markerChipLayer

      clip: true
      width: plot.plotArea.width
      height: plot.plotArea.height
      x: plot.graph.x + plot.plotArea.x
      y: plot.graph.y + plot.plotArea.y
      visible: root.showFrequencyMarkers

      function relayout() {
        const rows = []
        const items = []
        for (let i = 0; i < chipRepeater.count; ++i) {
          const it = chipRepeater.itemAt(i)
          if (it && it.visible)
            items.push(it)
        }

        items.sort((a, b) => a.x - b.x)
        for (let i = 0; i < items.length; ++i) {
          const it = items[i]
          let row = 0
          while (row < rows.length && rows[row] > it.x - 4)
            ++row

          rows[row] = it.x + it.width
          it.y = 4 + row * (it.height + 2)
        }
      }

      function scheduleRelayout() {
        Qt.callLater(markerChipLayer.relayout)
      }

      Repeater {
        id: chipRepeater

        model: root.model ? root.model.markers : []
        onItemAdded: markerChipLayer.scheduleRelayout()
        onItemRemoved: markerChipLayer.scheduleRelayout()

        delegate: Label {
          id: chip

          required property int index
          required property var modelData

          property int markerState: 0
          property real blinkFactor: 1
          property real peakDb: Number.NaN

          readonly property bool isBand: modelData.endFreq > modelData.freq
          readonly property real xCenter: isBand
                                          ? (markerBandLayer.xAt(modelData.freq)
                                             + markerBandLayer.xAt(modelData.endFreq)) / 2
                                          : markerBandLayer.xAt(modelData.freq)
          readonly property bool spotlit: root.selectedMarker === index
          readonly property bool dimmed: root.selectedMarker >= 0 && !spotlit
          readonly property color baseColor: modelData.color !== "" ? modelData.color : root.color
          readonly property color liveColor: markerState === 2
                                             ? Cpp_ThemeManager.alarmColorForSeverity(3)
                                             : (markerState === 1
                                                ? Cpp_ThemeManager.alarmColorForSeverity(2)
                                                : baseColor)

          padding: 4
          scale: spotlit ? 1.08 : 1
          opacity: (dimmed ? 0.35 : 1) * blinkFactor
          color: Cpp_ThemeManager.colors["widget_base"]
          font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))
          visible: xCenter >= 0 && xCenter <= markerChipLayer.width
          x: Math.max(2, Math.min(markerChipLayer.width - width - 2, xCenter - width / 2))
          text: {
            const name = modelData.label !== ""
                         ? modelData.label
                         : qsTr("%1 Hz").arg(plot.engineeringFormat(modelData.freq, 0))
            if (isNaN(chip.peakDb))
              return name

            return qsTr("%1  %2 dB").arg(name).arg(chip.peakDb.toFixed(1))
          }

          onXChanged: markerChipLayer.scheduleRelayout()
          onWidthChanged: markerChipLayer.scheduleRelayout()
          onVisibleChanged: markerChipLayer.scheduleRelayout()
          onMarkerStateChanged: {
            if (markerState !== 2)
              blinkFactor = 1
          }

          Behavior on scale {
            NumberAnimation {
              duration: 120
              easing.type: Easing.OutQuad
            }
          }

          background: Rectangle {
            radius: 3
            color: chip.liveColor
            opacity: chip.spotlit ? 1 : 0.9
            border.width: chip.spotlit ? 1.5 : 0
            border.color: Cpp_ThemeManager.colors["widget_text"]
          }

          MouseArea {
            id: chipMouse

            hoverEnabled: true
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            cursorShape: Qt.PointingHandCursor

            ToolTip.delay: 700
            ToolTip.visible: containsMouse
            ToolTip.text: chip.spotlit
                          ? qsTr("Click to clear the spotlight.")
                          : qsTr("Click to spotlight this marker.")

            onClicked: root.selectedMarker = chip.spotlit ? -1 : chip.index
            onWheel: (wheel) => wheel.accepted = false
          }

          Connections {
            target: root.model
            function onMarkerValuesChanged() {
              chip.peakDb = root.model.markerPeakDb(chip.index)
              if (chip.modelData.hasThresholds === true)
                chip.markerState = root.model.markerState(chip.index)
            }
          }

          SequentialAnimation on blinkFactor {
            loops: Animation.Infinite
            running: chip.markerState === 2

            NumberAnimation {
              to: 0.35
              duration: 350
              easing.type: Easing.InOutQuad
            }

            NumberAnimation {
              to: 1.0
              duration: 350
              easing.type: Easing.InOutQuad
            }
          }
        }
      }
    }
  }

  //
  // Unavailable over a remote attach: an empty spectrum reads as a broken one
  //
  Rectangle {
    z: 1000
    anchors.fill: parent
    visible: Cpp_API_Mirror.attached
    color: Cpp_ThemeManager.colors["widget_base"]

    Label {
      anchors.centerIn: parent
      wrapMode: Text.WordWrap
      horizontalAlignment: Text.AlignHCenter
      width: Math.min(parent.width - 32, 320)
      color: Cpp_ThemeManager.colors["widget_text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9)
      text: qsTr("Not available over a remote attach. This widget needs the remote's raw sample "
               + "stream, which the dashboard mirror does not carry.")
    }
  }
}
