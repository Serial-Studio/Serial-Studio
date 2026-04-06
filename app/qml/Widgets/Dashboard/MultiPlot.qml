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
  property bool interpolate: true
  property bool showLegends: true

  //
  // User-controlled visibility preferences (persisted, ANDed with size thresholds)
  //
  property bool userShowLegends: true
  property bool userShowXLabel: true
  property bool userShowYLabel: true

  //
  // Set downsample size based on widget size & zoom factor
  //
  function setDownsampleFactor()
  {
    const z = plot.zoom
    model.dataW = plot.plotArea.width * z
    model.dataH = plot.plotArea.height * z
  }

  //
  // Sync model width/height with widget, then restore persisted settings
  //
  Component.onCompleted: {
    root.setDownsampleFactor()

    const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

    if (s["interpolate"] !== undefined)
      root.interpolate = s["interpolate"]

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
  }

  //
  // Enable/disable features depending on window size, ANDed with user preferences
  //
  onWidthChanged: updateWidgetOptions()
  onHeightChanged: updateWidgetOptions()
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

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    DashboardToolButton {
      checked: root.interpolate
      ToolTip.text: qsTr("Interpolate")
      onClicked: {
        root.interpolate = !root.interpolate
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "interpolate", root.interpolate)
      }
      icon.source: root.interpolate?
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-on.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-off.svg"
    }

    DashboardToolButton {
      onClicked: {
        root.userShowLegends = !root.userShowLegends
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowLegends", root.userShowLegends)
      }
      checked: root.userShowLegends
      ToolTip.text: qsTr("Show Legends")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/labels.svg"
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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/x.svg"
    }

    DashboardToolButton {
      onClicked: {
        root.userShowYLabel = !root.userShowYLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowYLabel", root.userShowYLabel)
      }
      checked: root.userShowYLabel
      ToolTip.text: qsTr("Show Y Axis Label")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/y.svg"
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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/crosshair.svg"
    }

    DashboardToolButton {
      checked: !model.running
      ToolTip.text: model.running ? qsTr("Pause") : qsTr("Resume")
      icon.source: model.running?
                     "qrc:/rcc/icons/dashboard-buttons/pause.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/resume.svg"
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
      enabled: plot.xAxis.zoom !== 1 || plot.yAxis.zoom !== 1
      icon.source: "qrc:/rcc/icons/dashboard-buttons/return.svg"
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Axis Range Settings")
      icon.source: "qrc:/rcc/icons/toolbar/settings.svg"
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
      xLabel: qsTr("Samples")
      Layout.fillWidth: true
      Layout.fillHeight: true
      yLabel: root.model.yLabel
      curveColors: root.model.colors
      mouseAreaEnabled: windowRoot.focused
      xAxis.tickInterval: plot.xTickInterval
      yAxis.tickInterval: plot.yTickInterval

      onZoomChanged: root.setDownsampleFactor()
      onWidthChanged: root.setDownsampleFactor()
      onHeightChanged: root.setDownsampleFactor()

      //
      // Register line series
      //
      Instantiator {
        model: root.model.count
        delegate: LineSeries {
          property int curveIndex: index
          property bool curveVisible: root.interpolate
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
          property bool curveVisible: !root.interpolate
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
      visible: root.showLegends
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
