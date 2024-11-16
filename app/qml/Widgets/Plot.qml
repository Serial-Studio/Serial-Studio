/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
  property alias yLabel: _yLabel.text
  property alias xLabel: _xLabel.text
  property alias curveColors: _theme.seriesColors

  //
  // Set axis visibility based on user options and/or widget size
  //
  readonly property bool yLabelVisible: root.height >= 120 && (Cpp_UI_Dashboard.axisVisibility & SerialStudio.AxisY)
  readonly property bool xLabelVisible: root.height >= 120 && (Cpp_UI_Dashboard.axisVisibility & SerialStudio.AxisX)

  //
  // Plot widget
  //
  GraphsView {
    id: _graph
    anchors {
      top: parent.top
      right: parent.right
      topMargin: root.yLabelVisible ? 4 : 0
      leftMargin: root.yLabelVisible ? 2 : 0
      rightMargin: root.xLabelVisible ? 4 : 0
      bottomMargin: root.xLabelVisible ? 2 : 0
      left: root.yLabelVisible ? _yLabelContainer.right : parent.left
      bottom: root.xLabelVisible ? _xLabelContainer.top : parent.bottom
    }

    marginTop: root.yLabelVisible ? 8 : 0
    marginRight: root.xLabelVisible ? 8 : 0
    marginLeft: root.yLabelVisible ? -8 : (root.xLabelVisible ? -40 : -60)
    marginBottom: root.xLabelVisible ? -8 : (root.yLabelVisible ? -20 : -40)

    //
    // Set plot colors
    //
    theme: GraphsTheme {
      id: _theme

      // Background
      plotAreaBackgroundVisible: true
      theme: GraphsTheme.Theme.UserDefined
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
  // Plot area geometry calculation for tick intervals
  //
  Item {
    id: _plotAreaGeometry

    anchors {
      top: parent.top
      right: parent.right
      topMargin: root.yLabelVisible ? 16 : 0
      leftMargin: root.yLabelVisible ? 4 : 0
      rightMargin: root.xLabelVisible ? 16 : 0
      bottomMargin: root.xLabelVisible ? 8 : 0
      left: root.yLabelVisible ? _yLabelContainer.right : parent.left
      bottom: root.xLabelVisible ? _xLabelContainer.top : parent.bottom
    }

    //
    // Mouse area to detect cursor position
    //
    /*MouseArea {
      id: crosshairMouseArea
      anchors.fill: parent
      hoverEnabled: true
      onPositionChanged: (mouse) => {
        verticalCrosshair.x = mouse.x
        horizontalCrosshair.y = mouse.y
      }

      onEntered: {
        verticalCrosshair.visible = true
        horizontalCrosshair.visible = true
      }

      onExited: {
        verticalCrosshair.visible = false
        horizontalCrosshair.visible = false
      }
    }

    //
    // Vertical crosshair
    //
    Rectangle {
      id: verticalCrosshair

      width: 1
      visible: false
      color: Cpp_ThemeManager.colors["widget_highlight"]

      anchors {
        top: parent.top
        bottom: parent.bottom
      }
    }

    //
    // Horizontal crosshair
    //
    Rectangle {
      id: horizontalCrosshair

      height: 1
      visible: false
      color: Cpp_ThemeManager.colors["widget_highlight"]

      anchors {
        left: parent.left
        right: parent.right
      }
    }*/
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
      font: Cpp_Misc_CommonFonts.customMonoFont(0.91)
      color: Cpp_ThemeManager.colors["widget_text"]
      anchors.verticalCenterOffset: root.xLabelVisible && _yLabel.implicitWidth <= _graph.height ?
                                      -1 * Math.abs(_graph.marginBottom - _graph.marginTop) : 0
    }
  }

  //
  // X-axis label
  //
  Item {
    id: _xLabelContainer
    height: _xLabel.height
    visible: root.xLabelVisible

    anchors {
      right: parent.right
      bottom: parent.bottom
      left: _yLabelContainer.right
      leftMargin: root.yLabelVisible ? 4 : 0
    }

    Label {
      id: _xLabel
      width: parent.width
      elide: Qt.ElideRight
      anchors.centerIn: parent
      horizontalAlignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.customMonoFont(0.91)
      color: Cpp_ThemeManager.colors["widget_text"]
      anchors.horizontalCenterOffset: root.yLabelVisible ? Math.abs(_graph.marginLeft - _graph.marginRight) : 0
    }
  }
}
