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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import SerialStudio
import ".." as Widgets

//
// Frame annotation layer (spec 0059): track strip, table + CSV, payload view, decoder editor
//
Item {
  id: root

  readonly property int maxTrackSpans: 4096
  readonly property int maxPayloadBytes: 65536
  readonly property var model: Cpp_Console_Handler.annotations
  readonly property var filter: Cpp_Console_Handler.annotationFilter
  readonly property var decoder: Cpp_Console_Handler.annotationDecoder
  readonly property int annotationCount: root.model ? root.model.count : 0
  readonly property int trackRows: root.model ? root.model.rowNames.length : 0

  //
  // A decoder that never declared rows leaves every inspector tab empty: the panel then shows
  // one explanation instead of four blank views
  //
  readonly property bool decoded: root.trackRows > 0
  readonly property bool running: root.decoder ? root.decoder.enabled : false

  //
  // Decoder state as a single traffic light, shared by the status dot and its caption
  //
  readonly property color statusColor: {
    if (!root.decoder)
      return Cpp_ThemeManager.colors["placeholder_text"]

    if (root.decoder.failed)
      return Cpp_ThemeManager.colors["alarm"]

    if (root.decoder.enabled)
      return Cpp_ThemeManager.colors["alarm_ok"]

    return root.decoder.compiled ? Cpp_ThemeManager.colors["alarm_warning"]
                                 : Cpp_ThemeManager.colors["placeholder_text"]
  }

  readonly property string statusText: {
    if (!root.decoder)
      return ""

    if (root.decoder.failed)
      return qsTr("Decoder error: %1").arg(root.decoder.lastError)

    if (root.decoder.enabled)
      return qsTr("Decoding, %1 annotations").arg(root.annotationCount)

    return root.decoder.compiled ? qsTr("Paused, %1 annotations kept").arg(root.annotationCount)
                                 : qsTr("No decoder applied")
  }

  //
  // The bundled protocol decoders, and the one the editor starts from: a ready-to-run script
  // beats an empty editor, and the same file feeds the picker
  //
  readonly property var decoderTemplates: root.decoder ? root.decoder.templates() : []

  function starterDecoder() {
    for (let i = 0; i < root.decoderTemplates.length; ++i)
      if (root.decoderTemplates[i]["default"] === true)
        return root.decoder.templateCode(root.decoderTemplates[i].file)

    return root.decoderTemplates.length > 0
        ? root.decoder.templateCode(root.decoderTemplates[0].file)
        : ""
  }

  //
  // Byte offsets run into the millions; the default float rendering turns them into 1.07e+07
  //
  function formatNumber(value) {
    return Number(value).toLocaleString(Qt.locale(), 'f', 0)
  }

  //
  // Panel state that is not project data: it follows the user, not the .ssproj, and has to
  // survive the modes where widgetSettings is discarded (anything but Project File)
  //
  property string lastDecoderCode: ""
  property bool lastDecoderEnabled: false

  Settings {
    category: "ConsoleAnnotations"

    property alias currentTab: _tabs.currentIndex
    property alias windowBytes: _windowSpin.value
    property alias payloadHex: _payloadHex.checked
    property alias decoderCode: root.lastDecoderCode
    property alias decoderEnabled: root.lastDecoderEnabled
  }

  //
  // The decoder is stored twice on purpose: the project copy travels with the .ssproj, the
  // app copy is the fallback for Quick Plot and Console Only, where the project store is a no-op
  //
  function saveDecoderSettings() {
    Cpp_JSON_ProjectModel.saveWidgetSetting("console", "annotationDecoder", root.decoder.code)
    Cpp_JSON_ProjectModel.saveWidgetSetting("console", "annotationDecoderEnabled",
                                            root.decoder.enabled)

    root.lastDecoderCode = root.decoder.code
    root.lastDecoderEnabled = root.decoder.enabled
  }

  function restoreDecoderSettings() {
    const s = Cpp_JSON_ProjectModel.widgetSettings("console")
    const project_code = typeof s["annotationDecoder"] === "string" ? s["annotationDecoder"] : ""
    const code = project_code.length > 0 ? project_code : root.lastDecoderCode
    const enabled = project_code.length > 0 ? s["annotationDecoderEnabled"] === true
                                            : root.lastDecoderEnabled

    if (code.length === 0) {
      _decoderEditor.setText(root.starterDecoder())
      return
    }

    _decoderEditor.setText(code)
    root.decoder.setCode(code)
    if (enabled)
      root.decoder.setEnabled(true)
  }

  function applyDecoder() {
    root.decoder.setCode(_decoderEditor.text)
    root.decoder.setEnabled(true)
    root.saveDecoderSettings()
  }

  //
  // The decoder only runs while this panel is on screen: closed, it would keep copying the whole
  // console stream into the retained window and calling decode() per chunk for nobody
  //
  function publishViewerState(on) {
    if (root.decoder)
      root.decoder.setViewerActive(root, on)
  }

  onVisibleChanged: root.publishViewerState(root.visible)
  Component.onDestruction: root.publishViewerState(false)

  Component.onCompleted: {
    root.publishViewerState(root.visible)
    Qt.callLater(root.restoreDecoderSettings)
  }

  //
  // Refresh the track strip at UI cadence, only while it is on screen
  //
  Connections {
    target: Cpp_Misc_TimerEvents
    enabled: root.visible && _tabs.currentIndex === 0

    function onUiTimeout() {
      _track.refresh()
    }
  }

  FileDialog {
    id: _csvDialog

    fileMode: FileDialog.SaveFile
    title: qsTr("Export annotations")
    nameFilters: [qsTr("CSV files (*.csv)")]
    onAccepted: {
      let path = selectedFile.toString()
      if (path.startsWith("file://"))
        path = decodeURIComponent(path.substring(7))

      if (!root.model.exportCsv(path))
        console.warn("[ConsoleAnnotations] CSV export failed:", path)
    }
  }

  ColumnLayout {
    spacing: 0
    anchors.fill: parent

    //
    // Tab bar, sized like every other tab bar in the application
    //
    TabBar {
      id: _tabs

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        text: qsTr("Track")
        height: _tabs.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Table")
        height: _tabs.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Payload")
        height: _tabs.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Decoder")
        height: _tabs.height + 3
        width: implicitWidth + 2 * 8
      }
    }

    //
    // Framed tab contents, tucked under the tab bar
    //
    Item {
      Layout.topMargin: -1
      Layout.fillWidth: true
      Layout.fillHeight: true

      Rectangle {
        radius: 2
        border.width: 1
        anchors.fill: parent
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      StackLayout {
        clip: true
        anchors.margins: 8
        anchors.fill: parent
        currentIndex: _tabs.currentIndex

        //
        // Track strip: one lane per declared row, spans proportional to byte offset inside the
        // retained window; the longest text that fits the span is drawn, then shorter, then none
        //
        Item {
          id: _track

          property var lanes: []
          property real windowEnd: 1
          property real windowStart: 0
          property var classColors: []
          property string laneStamp: ""

          //
          // Right edge follows the newest committed label, never the raw byte counter, and lanes
          // are reassigned only when the window, the geometry or the class set moved
          //
          function refresh() {
            if (!root.model)
              return

            const end = root.annotationCount > 0 ? root.model.labelledEnd + 1
                                                 : root.model.retainedEnd
            const start = Math.max(root.model.retainedStart, end - _windowSpin.value)
            const pixels = Math.max(1, _track.width - 96)
            const span = Math.max(start + 1, end)
            const perPixel = Math.max(1, Math.floor((span - start) / pixels))

            const colors = []
            const classes = root.model.classes
            for (let c = 0; c < classes.length; ++c)
              colors.push(classes[c].color)

            const stamp = start + "/" + span + "/" + pixels + "/" + root.trackRows + "/"
                        + colors.join(",")
            if (stamp === laneStamp)
              return

            windowStart = start
            windowEnd = span
            laneStamp = stamp

            const next = []
            for (let r = 0; r < root.trackRows; ++r)
              next.push(root.model.trackStrip(start, end, r, root.maxTrackSpans, perPixel, pixels))

            classColors = colors
            lanes = next
          }

          ColumnLayout {
            spacing: 6
            anchors.fill: parent

            RowLayout {
              spacing: 6
              Layout.fillWidth: true

              Label {
                text: qsTr("Window") + ":"
                color: Cpp_ThemeManager.colors["text"]
              }

              SpinBox {
                id: _windowSpin

                from: 256
                to: 1048576
                value: 4096
                stepSize: 256
                editable: true
                implicitHeight: 24
                implicitWidth: 128
                onValueModified: _track.refresh()
                ToolTip.delay: 500
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Bytes of history drawn across the lanes")
              }

              Label {
                text: qsTr("bytes")
                opacity: 0.7
                color: Cpp_ThemeManager.colors["text"]
              }

              Item {
                Layout.fillWidth: true
              }

              //
              // A paused decoder keeps its captured labels on screen; the badge says they are a
              // frozen capture rather than a live feed, on the tab where the button is not visible
              //
              Rectangle {
                radius: 2
                border.width: 1
                implicitHeight: 16
                color: "transparent"
                implicitWidth: _pausedLabel.implicitWidth + 12
                border.color: Cpp_ThemeManager.colors["alarm_warning"]
                visible: !root.running && root.decoded && root.annotationCount > 0

                Label {
                  id: _pausedLabel

                  text: qsTr("paused")
                  anchors.centerIn: parent
                  color: Cpp_ThemeManager.colors["alarm_warning"]
                  font: Cpp_Misc_CommonFonts.customUiFont(0.75, false)
                }
              }

              Label {
                elide: Text.ElideRight
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
                text: qsTr("%1 annotations kept").arg(root.formatNumber(root.annotationCount))
              }
            }

            //
            // Where the decoder is actually labelling, against the window drawn below: the two
            // ranges drifting apart is the signature of a decoder whose offsets left the stream
            //
            Label {
              opacity: 0.7
              wrapMode: Text.WordWrap
              Layout.fillWidth: true
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
              text: {
                if (root.annotationCount <= 0)
                  return qsTr("Each bar is a byte range the decoder labelled, one lane per "
                              + "decoder row: oldest on the left, newest on the right.")

                const range = qsTr("Labelled bytes %1 to %2, oldest on the left.")
                .arg(root.formatNumber(root.model.labelledStart))
                .arg(root.formatNumber(root.model.labelledEnd))

                if (root.annotationCount < root.model.capacity)
                  return range

                return range + " " + qsTr("The store is full at %1 labels, so anything older "
                                          + "was dropped: shrink the window to see individual "
                                          + "labels.").arg(root.formatNumber(root.model.capacity))
              }
            }

            Rectangle {
              implicitHeight: 1
              Layout.fillWidth: true
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            //
            // Legend: the color chips that make the lanes readable without hovering them
            //
            Flow {
              spacing: 12
              visible: root.decoded
              Layout.fillWidth: true

              Repeater {
                model: root.model ? root.model.classes : []

                delegate: RowLayout {
                  id: _chip

                  spacing: 4
                  required property var modelData

                  Rectangle {
                    radius: 2
                    implicitWidth: 8
                    implicitHeight: 8
                    color: _chip.modelData.color
                    Layout.alignment: Qt.AlignVCenter
                  }

                  Label {
                    text: _chip.modelData.name
                    Layout.alignment: Qt.AlignVCenter
                    color: Cpp_ThemeManager.colors["text"]
                    font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
                  }
                }
              }
            }

            ScrollView {
              id: _laneScroll

              clip: true
              Layout.fillWidth: true
              Layout.fillHeight: true
              contentWidth: availableWidth

              Column {
                spacing: 2
                width: _laneScroll.availableWidth

                Repeater {
                  model: root.trackRows

                  delegate: Item {
                    id: _lane

                    required property int index

                    height: 22
                    width: parent.width

                    Label {
                      width: 88
                      elide: Text.ElideRight
                      horizontalAlignment: Text.AlignRight
                      anchors.verticalCenter: parent.verticalCenter
                      color: Cpp_ThemeManager.colors["pane_section_label"]
                      font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
                      text: root.model ? root.model.rowNames[_lane.index] : ""
                    }

                    Rectangle {
                      id: _laneArea

                      x: 96
                      radius: 2
                      border.width: 1
                      height: parent.height
                      width: parent.width - 96
                      color: Cpp_ThemeManager.colors["base"]
                      border.color: Cpp_ThemeManager.colors["groupbox_border"]

                      readonly property var strip: _track.lanes.length > _lane.index
                                                   ? _track.lanes[_lane.index]
                                                   : null
                      readonly property int spanCount: _laneArea.strip
                                                       ? _laneArea.strip.count
                                                       : 0

                      onStripChanged: _laneCanvas.requestPaint()

                      Label {
                        anchors.centerIn: parent
                        visible: _laneArea.spanCount === 0
                        text: qsTr("nothing labelled in this window")
                        color: Cpp_ThemeManager.colors["placeholder_text"]
                        font: Cpp_Misc_CommonFonts.customUiFont(0.75, false)
                      }

                      //
                      // Marks are painted, never instantiated: one scene-graph item per mark,
                      // rebuilt every tick, is what made a decimated lane crawl
                      //
                      Canvas {
                        id: _laneCanvas

                        anchors.margins: 1
                        antialiasing: false
                        anchors.fill: parent
                        renderStrategy: Canvas.Cooperative


                        onPaint: {
                          const ctx = getContext("2d")
                          ctx.clearRect(0, 0, width, height)

                          const strip = _laneArea.strip
                          if (!strip || strip.count <= 0)
                            return

                          const g = strip.geometry
                          const colors = _track.classColors
                          const labels = strip.labels
                          const shorts = strip.shortLabels
                          const top = 1
                          const barHeight = Math.max(1, height - 2)

                          ctx.globalAlpha = 0.9
                          ctx.textAlign = "center"
                          ctx.textBaseline = "middle"
                          ctx.font = _laneCanvas.labelFont

                          for (let i = 0; i < g.length; i += 6) {
                            const cls = g[i + 4]
                            ctx.fillStyle = cls < colors.length ? colors[cls] : "#888888"
                            ctx.fillRect(g[i], top, g[i + 1], barHeight)
                          }

                          if (labels.length === 0)
                            return

                          ctx.globalAlpha = 1
                          ctx.fillStyle = Cpp_ThemeManager.colors["highlighted_text"]
                          for (let j = 0; j < labels.length; ++j) {
                            const x = g[j * 6]
                            const w = g[j * 6 + 1]
                            const merged = g[j * 6 + 5] > 1
                            const long_text = merged ? qsTr("%1 labels")
                                                       .arg(root.formatNumber(g[j * 6 + 5]))
                                                     : labels[j]
                            let label = ""
                            if (ctx.measureText(long_text).width + 6 <= w)
                              label = long_text

                            else {
                              const short_text = merged ? String(g[j * 6 + 5]) : shorts[j]
                              if (ctx.measureText(short_text).width + 6 <= w)
                                label = short_text
                            }

                            if (label.length > 0)
                              ctx.fillText(label, x + w / 2, height / 2)
                          }
                        }

                        readonly property string labelFont: {
                          const f = (Cpp_Misc_CommonFonts.widgetFontRevision,
                                     Cpp_Misc_CommonFonts.widgetFont(0.75))
                          const px = f.pixelSize > 0 ? f.pixelSize
                                                     : Math.round(f.pointSize * 96 / 72)
                          return Math.max(8, px) + "px \"" + f.family + "\""
                        }

                        Connections {
                          target: Cpp_ThemeManager

                          function onThemeChanged() {
                            _laneCanvas.requestPaint()
                          }
                        }
                      }

                      //
                      // One hit test against the packed geometry replaces a hover handler per mark
                      //
                      MouseArea {
                        id: _laneHover

                        hoverEnabled: true
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton

                        property string hoverText: ""

                        onExited: hoverText = ""
                        onPositionChanged: (mouse) => {
                                             const strip = _laneArea.strip
                                             if (!strip || strip.count <= 0) {
                                               hoverText = ""
                                               return
                                             }

                                             const g = strip.geometry
                                             for (let i = 0; i < g.length; i += 6) {
                                               if (mouse.x < g[i] || mouse.x > g[i] + g[i + 1])
                                               continue

                                               const merged = g[i + 5] > 1
                                               hoverText = merged
                                               ? qsTr("%1 labels merged (bytes %2 to %3). Shrink the "
                                                      + "window to separate them.")
                                                 .arg(root.formatNumber(g[i + 5]))
                                                 .arg(root.formatNumber(g[i + 2]))
                                                 .arg(root.formatNumber(g[i + 3]))
                                               : qsTr("%1 (bytes %2 to %3)")
                                               .arg(strip.labels.length > i / 6
                                                    ? strip.labels[i / 6] : "")
                                               .arg(root.formatNumber(g[i + 2]))
                                               .arg(root.formatNumber(g[i + 3]))
                                               return
                                             }

                                             hoverText = ""
                                           }

                        ToolTip.delay: 300
                        ToolTip.text: _laneHover.hoverText
                        ToolTip.visible: _laneHover.containsMouse && _laneHover.hoverText.length > 0
                      }
                    }
                  }
                }
              }
            }

            //
            // Byte ruler: the strip is a window over the stream, so both ends carry their
            // absolute offset
            //
            RowLayout {
              spacing: 6
              Layout.leftMargin: 96
              Layout.fillWidth: true

              Label {
                opacity: 0.7
                color: Cpp_ThemeManager.colors["text"]
                text: root.formatNumber(_track.windowStart)
                font: Cpp_Misc_CommonFonts.customUiFont(0.75, false)
              }

              Item {
                Layout.fillWidth: true
              }

              Label {
                opacity: 0.7
                color: Cpp_ThemeManager.colors["text"]
                text: root.formatNumber(_track.windowEnd)
                font: Cpp_Misc_CommonFonts.customUiFont(0.75, false)
              }
            }
          }

        }

        //
        // Table: filter by row/class, sort by any column, export as CSV
        //
        ColumnLayout {
          spacing: 6

          RowLayout {
            spacing: 6
            Layout.fillWidth: true

            Label {
              text: qsTr("Row") + ":"
              color: Cpp_ThemeManager.colors["text"]
            }

            Widgets.Combo {
              id: _rowFilter

              Layout.preferredWidth: 130
              onActivated: root.filter.rowFilter = currentIndex - 1
              model: [qsTr("All rows")].concat(root.model ? root.model.rowNames : [])
            }

            Label {
              text: qsTr("Class") + ":"
              Layout.leftMargin: 6
              color: Cpp_ThemeManager.colors["text"]
            }

            Widgets.Combo {
              id: _classFilter

              textRole: "name"
              Layout.preferredWidth: 150
              onActivated: root.filter.classFilter = currentIndex - 1
              model: [{ "name": qsTr("All classes") }].concat(root.model ? root.model.classes : [])
            }

            Item {
              Layout.fillWidth: true
            }

            Widgets.IconButton {
              leftPadding: 8
              rightPadding: 8
              text: qsTr("Export CSV")
              onClicked: _csvDialog.open()
              opacity: enabled ? 1 : 0.5
              enabled: root.annotationCount > 0
              icon.source: "qrc:/icons/buttons/export-csv.svg"
              ToolTip.text: qsTr("Save every annotation to a spreadsheet")
            }
          }

          Rectangle {
            id: _tableFrame

            border.width: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Cpp_ThemeManager.colors["base"]
            border.color: Cpp_ThemeManager.colors["groupbox_border"]

            readonly property var fixedWidths: [80, 80, 72, 104, 116]

            ColumnLayout {
              spacing: 0
              anchors.margins: 1
              anchors.fill: parent

              HorizontalHeaderView {
                id: _header

                clip: true
                syncView: _table
                Layout.fillWidth: true
                Layout.preferredHeight: 24

                delegate: Rectangle {
                  id: _headerCell

                  required property var display

                  implicitHeight: 24

                  gradient: Gradient {
                    GradientStop {
                      position: 0
                      color: Cpp_ThemeManager.colors["table_bg_header_top"]
                    }

                    GradientStop {
                      position: 1
                      color: Cpp_ThemeManager.colors["table_bg_header_bottom"]
                    }
                  }

                  Rectangle {
                    height: 1
                    width: parent.width
                    anchors.bottom: parent.bottom
                    color: Cpp_ThemeManager.colors["table_border_header"]
                  }

                  Label {
                    anchors.fill: parent
                    anchors.margins: 6
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
                onWidthChanged: Qt.callLater(forceLayout)
                boundsBehavior: Flickable.StopAtBounds
                columnWidthProvider: function(column) {
                  const fixed = _tableFrame.fixedWidths
                  if (column < fixed.length)
                    return fixed[column]

                  return Math.max(180, _table.width - 452)
                }

                delegate: Rectangle {
                  id: _cell

                  required property int column
                  required property var display
                  required property color classColor

                  implicitWidth: 80
                  implicitHeight: 22
                  color: Cpp_ThemeManager.colors["table_cell_bg"]

                  Rectangle {
                    height: 1
                    width: parent.width
                    anchors.bottom: parent.bottom
                    color: Cpp_ThemeManager.colors["table_separator"]
                  }

                  RowLayout {
                    spacing: 6
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6

                    Rectangle {
                      radius: 2
                      implicitWidth: 8
                      implicitHeight: 8
                      color: _cell.classColor
                      visible: _cell.column === 4
                      Layout.alignment: Qt.AlignVCenter
                    }

                    Label {
                      Layout.fillWidth: true
                      text: _cell.display
                      elide: Text.ElideRight
                      verticalAlignment: Text.AlignVCenter
                      color: Cpp_ThemeManager.colors["table_text"]
                      font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
                    }
                  }
                }
              }
            }

            Label {
              anchors.centerIn: parent
              visible: root.annotationCount === 0
              text: qsTr("No annotations decoded yet")
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }
          }
        }

        //
        // Payload: every byte annotated with one class, concatenated in stream order
        //
        ColumnLayout {
          spacing: 6

          RowLayout {
            spacing: 6
            Layout.fillWidth: true

            Label {
              text: qsTr("Class") + ":"
              color: Cpp_ThemeManager.colors["text"]
            }

            Widgets.Combo {
              id: _payloadClass

              textRole: "name"
              Layout.preferredWidth: 150
              onActivated: _payload.refresh()
              model: root.model ? root.model.classes : []
            }

            CheckBox {
              id: _payloadHex

              checked: true
              Layout.leftMargin: 6
              text: qsTr("Hexadecimal")
              onToggled: _payload.refresh()
            }

            Widgets.IconButton {
              leftPadding: 8
              rightPadding: 8
              text: qsTr("Refresh")
              onClicked: _payload.refresh()
              icon.source: "qrc:/icons/buttons/refresh.svg"
              ToolTip.text: qsTr("Re-read the bytes of the selected class")
            }

            Item {
              Layout.fillWidth: true
            }

            Label {
              elide: Text.ElideRight
              opacity: _payload.length > 0 ? 1 : 0
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
              text: qsTr("%1 characters").arg(_payload.length)
            }
          }

          Rectangle {
            border.width: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Cpp_ThemeManager.colors["console_base"]
            border.color: Cpp_ThemeManager.colors["console_border"]

            ScrollView {
              clip: true
              anchors.margins: 1
              anchors.fill: parent

              TextArea {
                id: _payload

                readOnly: true
                wrapMode: TextEdit.WrapAnywhere
                color: Cpp_ThemeManager.colors["console_text"]
                placeholderTextColor: Cpp_ThemeManager.colors["placeholder_text"]
                font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.monoFont)
                placeholderText: qsTr("Pick a class and press Refresh to extract its bytes")

                background: Rectangle {
                  color: Cpp_ThemeManager.colors["console_base"]
                }

                function refresh() {
                  if (!root.model || _payloadClass.currentIndex < 0)
                    return

                  text = _payloadHex.checked
                      ? root.model.payloadHex(_payloadClass.currentIndex, root.maxPayloadBytes)
                      : root.model.payloadText(_payloadClass.currentIndex, root.maxPayloadBytes)
                }
              }
            }
          }
        }

        //
        // Decoder editor: a JavaScript object with rows, classes and decode(bytes, offset, ctx)
        //
        ColumnLayout {
          spacing: 6

          RowLayout {
            spacing: 6
            Layout.fillWidth: true

            Widgets.IconButton {
              id: _applyBt

              leftPadding: 8
              rightPadding: 8
              text: qsTr("Apply")
              onClicked: root.applyDecoder()
              icon.source: "qrc:/icons/buttons/apply.svg"
              ToolTip.text: qsTr("Compile the script and start decoding")
            }

            //
            // Protocol starting points; picking one replaces the editor, Apply still arms it
            //
            Widgets.Combo {
              id: _decoderTemplates

              textRole: "name"
              Layout.preferredWidth: 190
              model: root.decoderTemplates
              implicitHeight: _applyBt.implicitHeight
              ToolTip.text: qsTr("Load a decoder for a known protocol")
              onActivated: (index) => {
                             const code = root.decoder.templateCode(
                               root.decoderTemplates[index].file)
                             if (code.length > 0)
                             _decoderEditor.setText(code)
                           }
            }

            Widgets.IconButton {
              leftPadding: 8
              rightPadding: 8
              text: qsTr("Clear")
              opacity: enabled ? 1 : 0.5
              enabled: root.annotationCount > 0
              onClicked: root.decoder.reset()
              icon.source: "qrc:/icons/buttons/clear.svg"
              ToolTip.text: qsTr("Discard the annotations decoded so far")
            }

            Widgets.IconButton {
              leftPadding: 8
              rightPadding: 8
              opacity: enabled ? 1 : 0.5
              enabled: root.decoder ? root.decoder.compiled : false
              text: root.running ? qsTr("Pause") : qsTr("Resume")
              icon.source: root.running
                           ? Cpp_Misc_IconRegistry.icon("commands", "pause", 16)
                           : Cpp_Misc_IconRegistry.icon("commands", "resume", 16)
              ToolTip.text: root.running
                            ? qsTr("Stop decoding, keep the labels already captured")
                            : qsTr("Resume decoding the incoming bytes")
              onClicked: {
                root.decoder.setEnabled(!root.running)
                root.saveDecoderSettings()
              }
            }

            Item {
              Layout.fillWidth: true
            }

            Rectangle {
              radius: 4
              implicitWidth: 8
              implicitHeight: 8
              color: root.statusColor
              Layout.alignment: Qt.AlignVCenter
            }

            Label {
              elide: Text.ElideRight
              text: root.statusText
              Layout.maximumWidth: 280
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
              color: root.decoder && root.decoder.failed
                     ? Cpp_ThemeManager.colors["alarm"]
                     : Cpp_ThemeManager.colors["text"]
            }
          }

          MacroEditor {
            id: _decoderEditor

            language: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
          }
        }
      }

      //
      // Nothing to inspect until a decoder declares its rows: one explanation beats four blank
      // tabs, and the button lands the user where the work happens
      //
      Rectangle {
        radius: 2
        anchors.fill: parent
        anchors.margins: 1
        visible: !root.decoded && _tabs.currentIndex !== 3
        color: Cpp_ThemeManager.colors["groupbox_background"]

        ColumnLayout {
          spacing: 8
          anchors.centerIn: parent
          width: Math.min(parent.width - 32, 460)

          Image {
            opacity: 0.6
            sourceSize: Qt.size(48, 48)
            Layout.alignment: Qt.AlignHCenter
            fillMode: Image.PreserveAspectFit
            source: Cpp_Misc_IconRegistry.icon("console", "annotations", 48)
          }

          Label {
            text: qsTr("No annotations yet")
            Layout.alignment: Qt.AlignHCenter
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1, true)
          }

          Label {
            opacity: 0.8
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            text: qsTr("Annotations label ranges of the incoming byte stream: a small script "
                       + "names each range, and this panel draws them as lanes, lists them, "
                       + "and extracts their bytes.")
          }

          Widgets.IconButton {
            leftPadding: 8
            rightPadding: 8
            text: qsTr("Open Decoder")
            Layout.alignment: Qt.AlignHCenter
            onClicked: _tabs.currentIndex = 3
            icon.source: "qrc:/icons/buttons/code.svg"
          }
        }
      }
    }
  }
}
