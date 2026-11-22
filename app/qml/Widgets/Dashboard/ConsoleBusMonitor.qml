/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import ".." as Widgets

//
// Bus monitor (spec 0127 R10-R17, Pro): the messages on the link as a table and as a timeline.
// The monitor ingests only while this panel is on screen.
//
Item {
  id: root

  objectName: "console.busMonitorPanel"

  readonly property var monitor: Cpp_Console_Handler.busMonitor
  readonly property var filter: Cpp_Console_Handler.busMonitorFilter
  readonly property bool modbus: root.monitor && root.monitor.mode === 1

  //
  // Timeline window in milliseconds, from 200 ms to 10 minutes
  //
  property real windowMs: 5000
  readonly property real minWindowMs: 200
  readonly property real maxWindowMs: 600000

  //
  // Register formats of the data summary, as the monitor's bit flags
  //
  readonly property var formatNames: ["U16", "S16", "U32", "S32", "F32", "BIN"]

  function zoom(factor) {
    root.windowMs = Math.max(root.minWindowMs, Math.min(root.maxWindowMs, root.windowMs * factor))
  }

  function intervalText(ms) {
    if (ms <= 0)
      return "--"

    return ms < 1000 ? qsTr("%1 ms").arg(ms.toFixed(1)) : qsTr("%1 s").arg((ms / 1000).toFixed(2))
  }

  function severityColor(severity) {
    if (severity === 3)
      return Cpp_ThemeManager.colors["alarm"]

    if (severity === 2)
      return Cpp_ThemeManager.colors["alarm_warning"]

    return Cpp_ThemeManager.colors["highlight"]
  }

  //
  // Viewer gate: registered while visible, so a closed panel costs the console stream nothing
  //
  function syncViewer() {
    if (root.monitor)
      root.monitor.setViewerActive(root, root.visible)
  }

  onVisibleChanged: root.syncViewer()
  Component.onCompleted: root.syncViewer()
  Component.onDestruction: {
    if (root.monitor)
      root.monitor.setViewerActive(root, false)
  }

  ColumnLayout {
    spacing: 4
    anchors.fill: parent

    //
    // Controls
    //
    RowLayout {
      spacing: 6
      Layout.fillWidth: true

      Widgets.Combo {
        Layout.preferredWidth: 130
        objectName: "console.busMonitor.mode"
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
        currentIndex: root.monitor.mode
        model: [qsTr("Messages"), qsTr("Modbus RTU")]
        onActivated: root.monitor.mode = currentIndex
      }

      Widgets.PushButton {
        id: pauseButton

        objectName: "console.busMonitor.pause"

        checkable: true
        checked: root.monitor.paused
        Layout.alignment: Qt.AlignVCenter
        onClicked: root.monitor.paused = !root.monitor.paused
        text: root.monitor.paused ? qsTr("Resume") : qsTr("Pause")

        Connections {
          target: root.monitor
          function onSettingsChanged() { pauseButton.checked = root.monitor.paused }
        }
      }

      Widgets.PushButton {
        text: qsTr("Clear")
        Layout.alignment: Qt.AlignVCenter
        onClicked: root.monitor.clear()
        objectName: "console.busMonitor.clear"
      }

      Label {
        visible: root.modbus
        text: qsTr("Response timeout:")
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
      }

      Widgets.SpinBox {
        from: 10
        to: 10000
        stepSize: 50
        editable: true
        visible: root.modbus
        Layout.preferredWidth: 96
        value: root.monitor.timeoutMs
        Layout.alignment: Qt.AlignVCenter
        objectName: "console.busMonitor.timeout"
        onValueModified: root.monitor.timeoutMs = value
      }

      Label {
        visible: !root.modbus
        text: qsTr("Quiet time:")
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
      }

      Widgets.SpinBox {
        from: 1
        to: 1000
        editable: true
        visible: !root.modbus
        Layout.preferredWidth: 96
        value: root.monitor.gapMs
        Layout.alignment: Qt.AlignVCenter
        objectName: "console.busMonitor.gap"
        onValueModified: root.monitor.gapMs = value
      }

      Label {
        text: qsTr("ms")
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
      }

      CheckBox {
        visible: root.modbus
        text: qsTr("Issues only")
        checked: root.monitor.issuesOnly
        Layout.alignment: Qt.AlignVCenter
        objectName: "console.busMonitor.issuesOnly"
        onClicked: root.monitor.issuesOnly = checked
      }

      Repeater {
        model: root.formatNames

        delegate: CheckBox {
          required property int index
          required property string modelData

          text: modelData
          visible: root.modbus
          Layout.alignment: Qt.AlignVCenter
          objectName: "console.busMonitor.format." + modelData
          checked: (root.monitor.formats & (1 << index)) !== 0
          onClicked: root.monitor.formats = checked
                     ? (root.monitor.formats | (1 << index))
                     : (root.monitor.formats & ~(1 << index))
        }
      }

      Item {
        Layout.fillWidth: true
      }

      Label {
        font: Cpp_Misc_CommonFonts.monoFont
        Layout.alignment: Qt.AlignVCenter
        color: Cpp_ThemeManager.colors["placeholder_text"]
        text: qsTr("%1 messages, %2 errors, %3 warnings, every %4")
              .arg(root.monitor.frameTotal)
              .arg(root.monitor.errorTotal)
              .arg(root.monitor.warningTotal)
              .arg(root.intervalText(root.monitor.averageIntervalMs))
      }
    }

    //
    // Tab bar, sized like every other tab bar in the application
    //
    TabBar {
      id: _tabs

      implicitHeight: 24
      Layout.fillWidth: true

      Widgets.PanelTab {
        text: qsTr("Table")
        height: _tabs.height + 3
        width: implicitWidth + 2 * 8
      }

      Widgets.PanelTab {
        text: qsTr("Timeline")
        height: _tabs.height + 3
        width: implicitWidth + 2 * 8
      }
    }

    StackLayout {
      Layout.topMargin: -1
      Layout.fillWidth: true
      Layout.fillHeight: true
      currentIndex: _tabs.currentIndex

      //
      // Table: one row per message
      //
      Rectangle {
        id: _tableFrame

        border.width: 1
        color: Cpp_ThemeManager.colors["base"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        //
        // Time, role, slave, function, length, summary (fills), CRC, diagnosis, raw frame.
        // The Modbus-only columns collapse in Messages mode.
        //
        readonly property var fixedWidths: [96, 76, 52, 190, 60, -1, 56, 240, 280]
        readonly property var modbusOnly: [false, false, true, true, false, true, true, true, false]

        function columnWidth(column, viewWidth) {
          if (!root.modbus && _tableFrame.modbusOnly[column])
            return 0

          const fixed = _tableFrame.fixedWidths[column]
          if (fixed > 0)
            return fixed

          let used = 0
          for (let i = 0; i < _tableFrame.fixedWidths.length; ++i)
            if (_tableFrame.fixedWidths[i] > 0)
              used += _tableFrame.fixedWidths[i]

          return Math.max(220, viewWidth - used)
        }

        ColumnLayout {
          spacing: 0
          anchors.margins: 1
          anchors.fill: parent

          HorizontalHeaderView {
            clip: true
            syncView: _table
            Layout.fillWidth: true
            Layout.preferredHeight: 24

            delegate: Rectangle {
              id: _headerCell

              required property var display

              implicitHeight: 24
              color: Cpp_ThemeManager.colors["table_bg_header_bottom"]

              Rectangle {
                height: 1
                width: parent.width
                anchors.bottom: parent.bottom
                color: Cpp_ThemeManager.colors["table_border_header"]
              }

              Label {
                anchors.margins: 6
                anchors.fill: parent
                elide: Text.ElideRight
                text: _headerCell.display
                verticalAlignment: Text.AlignVCenter
                color: Cpp_ThemeManager.colors["table_fg_header"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
              }
            }
          }

          TableView {
            id: _table

            clip: true
            model: root.filter
            Layout.fillWidth: true
            Layout.fillHeight: true
            boundsBehavior: Flickable.StopAtBounds
            onWidthChanged: Qt.callLater(forceLayout)
            columnWidthProvider: function(column) {
              return _tableFrame.columnWidth(column, _table.width)
            }

            //
            // Follow the newest message unless the user scrolled up
            //
            property bool follow: true
            onRowsChanged: {
              if (_table.follow && _table.rows > 0)
                Qt.callLater(_table.positionViewAtRow, _table.rows - 1, TableView.AlignBottom)
            }
            onMovementEnded: _table.follow = _table.atYEnd

            ScrollBar.vertical: ScrollBar {
              policy: ScrollBar.AsNeeded
            }

            ScrollBar.horizontal: ScrollBar {
              policy: ScrollBar.AsNeeded
            }

            delegate: Rectangle {
              id: _cell

              required property var display
              required property int severity

              implicitWidth: 80
              implicitHeight: 22
              color: _cell.severity >= 2
                     ? Qt.alpha(root.severityColor(_cell.severity), 0.18)
                     : Cpp_ThemeManager.colors["table_cell_bg"]

              Rectangle {
                height: 1
                width: parent.width
                anchors.bottom: parent.bottom
                color: Cpp_ThemeManager.colors["table_separator"]
              }

              Label {
                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: 6
                text: _cell.display
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                color: Cpp_ThemeManager.colors["table_text"]
                font: Cpp_Misc_CommonFonts.customMonoFont(0.8, false)
              }
            }
          }
        }

        Connections {
          target: root.monitor
          function onSettingsChanged() { Qt.callLater(_table.forceLayout) }
        }

        Label {
          anchors.centerIn: parent
          visible: root.monitor.count === 0
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["placeholder_text"]
          text: root.modbus
                ? qsTr("Waiting for Modbus RTU traffic on the connected line")
                : qsTr("Waiting for messages on the connected device")
        }
      }

      //
      // Timeline: the same messages as bars on a time axis
      //
      Rectangle {
        border.width: 1
        color: Cpp_ThemeManager.colors["base"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ColumnLayout {
          spacing: 2
          anchors.margins: 6
          anchors.fill: parent

          RowLayout {
            spacing: 6
            Layout.fillWidth: true

            Label {
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              text: qsTr("Window: %1").arg(root.intervalText(root.windowMs))
            }

            Item {
              Layout.fillWidth: true
            }

            Widgets.PushButton {
              text: qsTr("Zoom In")
              onClicked: root.zoom(0.5)
              objectName: "console.busMonitor.zoomIn"
              enabled: root.windowMs > root.minWindowMs
            }

            Widgets.PushButton {
              text: qsTr("Zoom Out")
              onClicked: root.zoom(2)
              objectName: "console.busMonitor.zoomOut"
              enabled: root.windowMs < root.maxWindowMs
            }
          }

          Item {
            id: _timeline

            Layout.fillWidth: true
            Layout.fillHeight: true

            readonly property real endMs: root.monitor.lastTimeMs
            readonly property real startMs: _timeline.endMs - root.windowMs
            readonly property real laneHeight: Math.max(12, (_timeline.height - 22) / 2)

            property var bars: []
            property int hoverRow: -1

            function refresh() {
              _timeline.bars = root.monitor.timelineBars(_timeline.startMs, _timeline.endMs,
                                                         _timeline.width)
              _timeline.hoverRow = -1
              _canvas.requestPaint()
            }

            //
            // Row of the bar nearest to the pointer within a few pixels and in its lane, or -1
            //
            function rowAt(x, y) {
              const lane = y < _timeline.laneHeight ? 0 : 1
              let best = -1
              let bestDistance = 5
              for (let i = 0; i + 3 < _timeline.bars.length; i += 4) {
                const distance = Math.abs(_timeline.bars[i] - x)
                if (_timeline.bars[i + 1] === lane && distance < bestDistance) {
                  best = _timeline.bars[i + 3]
                  bestDistance = distance
                }
              }

              return best
            }

            onWidthChanged: _timeline.refresh()
            onVisibleChanged: _timeline.refresh()

            Connections {
              target: root.monitor
              function onCountChanged() {
                if (_timeline.visible)
                  _timeline.refresh()
              }
            }

            Connections {
              target: root
              function onWindowMsChanged() { _timeline.refresh() }
            }

            Connections {
              target: Cpp_ThemeManager
              function onThemeChanged() { _canvas.requestPaint() }
            }

            Label {
              x: 2
              y: 0
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              text: root.modbus ? qsTr("Master (TX)") : qsTr("TX")
            }

            Label {
              x: 2
              y: _timeline.laneHeight
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              text: root.modbus ? qsTr("Slave (RX)") : qsTr("RX")
            }

            Canvas {
              id: _canvas

              anchors.fill: parent

              onPaint: {
                const ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                const lane = _timeline.laneHeight
                const barWidth = Math.max(1, Math.min(40, 30 * width / root.windowMs))
                ctx.fillStyle = Cpp_ThemeManager.colors["groupbox_border"]
                ctx.fillRect(0, lane, width, 1)
                ctx.fillRect(0, 2 * lane, width, 1)

                const bars = _timeline.bars
                for (let i = 0; i + 3 < bars.length; i += 4) {
                  ctx.fillStyle = root.severityColor(bars[i + 2])
                  ctx.fillRect(bars[i], bars[i + 1] * lane + 16, barWidth, lane - 20)
                }
              }
            }

            Label {
              anchors.bottom: parent.bottom
              anchors.horizontalCenter: parent.horizontalCenter
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              text: qsTr("Scroll to zoom. Hover a bar for its contents.")
            }

            MouseArea {
              hoverEnabled: true
              anchors.fill: parent
              acceptedButtons: Qt.NoButton
              onExited: _timeline.hoverRow = -1
              onPositionChanged: (mouse) => _timeline.hoverRow = _timeline.rowAt(mouse.x, mouse.y)
              onWheel: (wheel) => root.zoom(wheel.angleDelta.y > 0 ? 0.5 : 2)

              ToolTip.delay: 200
              ToolTip.visible: _timeline.hoverRow >= 0
              ToolTip.text: {
                if (_timeline.hoverRow < 0)
                  return ""

                const info = root.monitor.messageInfo(_timeline.hoverRow)
                const lines = [qsTr("%1  %2  %3 bytes").arg(info.time).arg(info.role).arg(info.length),
                               info.raw]
                if (info.diagnosis && info.diagnosis.length > 0)
                  lines.push(info.diagnosis)

                return lines.join("\n")
              }
            }
          }
        }
      }
    }
  }
}
