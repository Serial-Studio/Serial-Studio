/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  fixedSize: false
  title: qsTr("Frequency Markers")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Bound dataset identifiers + Nyquist limit. Populated by showDialog().
  //
  property int groupId: -1
  property int datasetId: -1
  property real nyquist: 100

  //
  // Shared table column widths. Single source of truth so the column header row
  // and the marker delegate rows line up regardless of style metrics.
  //
  readonly property int colWDb: 64
  readonly property int colWEnd: 84
  readonly property int colWFreq: 84
  readonly property int colWMove: 24
  readonly property int colWColor: 28
  readonly property int colSpacing: 8
  readonly property int colWDelete: 28
  readonly property int colScrollSlot: 12

  //
  // Editable working copy of the marker list. Optional numeric fields are held as
  // strings so an empty field cleanly means "unset".
  //
  ListModel { id: markersModel }

  //
  // Presets: common fixed-frequency watchlists. Entries above Nyquist are skipped
  // at apply time.
  //
  readonly property var presets: [
    {
      label: qsTr("Mains Hum (50 Hz + harmonics)"),
      markers: [
        { freq: 50, end: 0, label: qsTr("Mains"), color: "#ffb300" },
        { freq: 100, end: 0, label: qsTr("Mains 2x"), color: "#fb8c00" },
        { freq: 150, end: 0, label: qsTr("Mains 3x"), color: "#f4511e" }
      ]
    },
    {
      label: qsTr("Mains Hum (60 Hz + harmonics)"),
      markers: [
        { freq: 60, end: 0, label: qsTr("Mains"), color: "#ffb300" },
        { freq: 120, end: 0, label: qsTr("Mains 2x"), color: "#fb8c00" },
        { freq: 180, end: 0, label: qsTr("Mains 3x"), color: "#f4511e" }
      ]
    },
    {
      label: qsTr("Octave Bands (31.5 Hz - 16 kHz)"),
      markers: [
        { freq: 22.4, end: 44.7, label: qsTr("31.5 Hz"), color: "#e53935" },
        { freq: 44.7, end: 89.1, label: qsTr("63 Hz"), color: "#f4511e" },
        { freq: 89.1, end: 178, label: qsTr("125 Hz"), color: "#fb8c00" },
        { freq: 178, end: 355, label: qsTr("250 Hz"), color: "#fdd835" },
        { freq: 355, end: 708, label: qsTr("500 Hz"), color: "#7cb342" },
        { freq: 708, end: 1410, label: qsTr("1 kHz"), color: "#26a69a" },
        { freq: 1410, end: 2820, label: qsTr("2 kHz"), color: "#29b6f6" },
        { freq: 2820, end: 5620, label: qsTr("4 kHz"), color: "#5c6bc0" },
        { freq: 5620, end: 11200, label: qsTr("8 kHz"), color: "#8e24aa" },
        { freq: 11200, end: 22400, label: qsTr("16 kHz"), color: "#d81b60" }
      ]
    }
  ]

  //
  // Public API
  //
  function showDialog(gId, dId, nyq, initialMarkers) {
    root.groupId = gId
    root.datasetId = dId
    root.nyquist = Math.max(1, nyq)
    root.loadMarkers(initialMarkers)
    presetCombo.currentIndex = -1
    root.show()
    root.raise()
    root.requestActivate()
  }

  function optionalText(value) {
    if (value === undefined || value === null)
      return ""

    const n = Number(value)
    return isNaN(n) ? "" : String(n)
  }

  function loadMarkers(arr) {
    markersModel.clear()
    if (!arr)
      return

    for (let i = 0; i < arr.length; ++i) {
      const m = arr[i]
      const end = Number(m.endFreq ?? 0)
      markersModel.append({
        freqVal: Number(m.freq ?? 0),
        endFreqText: end > Number(m.freq ?? 0) ? String(end) : "",
        markerLabel: String(m.label ?? ""),
        colorHex: String(m.color ?? ""),
        warnDbText: root.optionalText(m.warningDb),
        alarmDbText: root.optionalText(m.alarmDb)
      })
    }
  }

  function collectMarkers() {
    const out = []
    for (let i = 0; i < markersModel.count; ++i) {
      const r = markersModel.get(i)
      const freq = Math.min(Number(r.freqVal), root.nyquist)
      if (!(freq > 0))
        continue

      const entry = {
        freq: freq,
        endFreq: 0,
        label: String(r.markerLabel),
        color: String(r.colorHex)
      }

      const end = Number(r.endFreqText)
      if (r.endFreqText !== "" && !isNaN(end) && end > freq)
        entry.endFreq = Math.min(end, root.nyquist)

      const warn = Number(r.warnDbText)
      if (r.warnDbText !== "" && !isNaN(warn))
        entry.warningDb = warn

      const alarm = Number(r.alarmDbText)
      if (r.alarmDbText !== "" && !isNaN(alarm))
        entry.alarmDb = alarm

      out.push(entry)
    }
    return out
  }

  function applyPreset(presetIdx) {
    if (presetIdx < 0 || presetIdx >= root.presets.length)
      return

    const preset = root.presets[presetIdx]
    markersModel.clear()
    for (let i = 0; i < preset.markers.length; ++i) {
      const m = preset.markers[i]
      if (m.freq > root.nyquist)
        continue

      markersModel.append({
        freqVal: m.freq,
        endFreqText: m.end > m.freq ? String(Math.min(m.end, root.nyquist)) : "",
        markerLabel: m.label,
        colorHex: String(m.color ?? ""),
        warnDbText: "",
        alarmDbText: ""
      })
    }
  }

  function addMarker() {
    const freq = Math.max(1, Math.round(root.nyquist * 0.25))
    markersModel.append({
      freqVal: freq,
      endFreqText: "",
      markerLabel: "",
      colorHex: "",
      warnDbText: "",
      alarmDbText: ""
    })
  }

  //
  // Resolves the on-screen color of a marker (custom override OR theme accent).
  //
  function markerColor(colorHex) {
    if (colorHex && colorHex.length > 0)
      return colorHex

    return Cpp_ThemeManager.colors["highlight"]
  }

  //
  // Converts a Qt color object to a "#rrggbb" hex string (alpha discarded).
  //
  function hexFromColor(c) {
    const r = Math.round(c.r * 255).toString(16).padStart(2, '0')
    const g = Math.round(c.g * 255).toString(16).padStart(2, '0')
    const b = Math.round(c.b * 255).toString(16).padStart(2, '0')
    return "#" + r + g + b
  }

  //
  // Single ColorDialog reused for every row; the target row is tracked by index.
  //
  ColorDialog {
    id: colorDialog

    property int targetRow: -1

    title: qsTr("Choose Marker Color")
    onAccepted: {
      if (colorDialog.targetRow >= 0 && colorDialog.targetRow < markersModel.count)
        markersModel.setProperty(colorDialog.targetRow, "colorHex",
                                 root.hexFromColor(colorDialog.selectedColor))
    }
  }

  //--------------------------------------------------------------------------------------------
  // Layout: section labels live outside their content cards; no internal separators
  //--------------------------------------------------------------------------------------------

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    Layout.minimumWidth: 840
    anchors.centerIn: parent

    //
    // Section 1: Preset picker + Nyquist readout
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true

      Label {
        text: qsTr("Presets")
        font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.minimumWidth: 840
        implicitHeight: presetLayout.implicitHeight + 16
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        RowLayout {
          id: presetLayout

          spacing: 8
          anchors.margins: 8
          anchors.fill: parent

          Label {
            text: qsTr("Preset")
            color: Cpp_ThemeManager.colors["text"]
            Layout.alignment: Qt.AlignVCenter
          }

          ComboBox {
            id: presetCombo

            currentIndex: -1
            textRole: "label"
            model: root.presets
            Layout.minimumWidth: 240
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            displayText: currentIndex < 0
                         ? qsTr("Choose preset…")
                         : root.presets[currentIndex].label
            onActivated: {
              root.applyPreset(presetCombo.currentIndex)
              presetCombo.currentIndex = -1
            }
          }

          Item { Layout.fillWidth: true }

          Label {
            text: qsTr("Frequency range")
            color: Cpp_ThemeManager.colors["placeholder_text"]
            Layout.alignment: Qt.AlignVCenter
          }

          Label {
            text: qsTr("0 - %1 Hz").arg(root.nyquist)
            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["text"]
            Layout.alignment: Qt.AlignVCenter
          }
        }
      }
    }

    //
    // Section 2: Markers table
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          text: qsTr("Markers")
          font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
          color: Cpp_ThemeManager.colors["pane_section_label"]
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        Item { Layout.fillWidth: true }

        Widgets.IconButton {
          iconSize: 14
          ToolTip.delay: 700
          text: qsTr("Add Marker")
          ToolTip.visible: hovered
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/icons/buttons/plus.svg"
          ToolTip.text: qsTr("Add a new point marker; set an end frequency to turn it into a band.")
          onClicked: root.addMarker()
        }
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 840
        Layout.minimumHeight: 220
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ColumnLayout {
          id: markersLayout

          spacing: 0
          anchors.margins: 8
          anchors.fill: parent

          //
          // Column headers: shared widths with the row delegate
          //
          RowLayout {
            Layout.fillWidth: true
            spacing: root.colSpacing
            Layout.rightMargin: root.colScrollSlot

            Label {
              text: qsTr("Start (Hz)")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.preferredWidth: root.colWFreq
            }
            Label {
              text: qsTr("End (Hz)")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.preferredWidth: root.colWEnd
            }
            Label {
              text: qsTr("Warn (dB)")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.preferredWidth: root.colWDb
            }
            Label {
              text: qsTr("Alarm (dB)")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.preferredWidth: root.colWDb
            }
            Label {
              text: qsTr("Color")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              horizontalAlignment: Text.AlignHCenter
              Layout.preferredWidth: root.colWColor
            }
            Label {
              text: qsTr("Label")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.fillWidth: true
            }
            Item { Layout.preferredWidth: root.colWMove }
            Item { Layout.preferredWidth: root.colWMove }
            Item { Layout.preferredWidth: root.colWDelete }
          }

          Rectangle {
            implicitHeight: 1
            Layout.topMargin: 4
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          //
          // Marker rows
          //
          ListView {
            id: markerList

            spacing: 1
            clip: true
            model: markersModel
            Layout.topMargin: 4
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 160
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
              policy: ScrollBar.AsNeeded
            }

            delegate: Item {
              id: markerRow

              required property int index
              required property double freqVal
              required property string endFreqText
              required property string markerLabel
              required property string colorHex
              required property string warnDbText
              required property string alarmDbText

              implicitHeight: 36
              width: ListView.view.width

              RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 0
                spacing: root.colSpacing
                anchors.rightMargin: root.colScrollSlot

                Widgets.LineField {
                  text: markerRow.freqVal
                  font: Cpp_Misc_CommonFonts.monoFont
                  Layout.preferredWidth: root.colWFreq
                  Layout.alignment: Qt.AlignVCenter
                  validator: DoubleValidator { bottom: 0; top: 1e9 }
                  onEditingFinished: {
                    const v = Number(text)
                    if (!isNaN(v))
                      markersModel.setProperty(markerRow.index, "freqVal",
                                               Math.min(v, root.nyquist))
                  }
                }

                Widgets.LineField {
                  text: markerRow.endFreqText
                  placeholderText: qsTr("(point)")
                  font: Cpp_Misc_CommonFonts.monoFont
                  Layout.preferredWidth: root.colWEnd
                  Layout.alignment: Qt.AlignVCenter
                  validator: DoubleValidator { bottom: 0; top: 1e9 }
                  onEditingFinished: {
                    const v = Number(text)
                    if (text === "" || isNaN(v))
                      markersModel.setProperty(markerRow.index, "endFreqText", "")
                    else
                      markersModel.setProperty(markerRow.index, "endFreqText",
                                               String(Math.min(v, root.nyquist)))
                  }
                }

                Widgets.LineField {
                  text: markerRow.warnDbText
                  placeholderText: qsTr("(off)")
                  font: Cpp_Misc_CommonFonts.monoFont
                  Layout.preferredWidth: root.colWDb
                  Layout.alignment: Qt.AlignVCenter
                  validator: DoubleValidator { bottom: -200; top: 100 }
                  onEditingFinished: {
                    const v = Number(text)
                    markersModel.setProperty(markerRow.index, "warnDbText",
                                             text === "" || isNaN(v) ? "" : String(v))
                  }
                }

                Widgets.LineField {
                  text: markerRow.alarmDbText
                  placeholderText: qsTr("(off)")
                  font: Cpp_Misc_CommonFonts.monoFont
                  Layout.preferredWidth: root.colWDb
                  Layout.alignment: Qt.AlignVCenter
                  validator: DoubleValidator { bottom: -200; top: 100 }
                  onEditingFinished: {
                    const v = Number(text)
                    markersModel.setProperty(markerRow.index, "alarmDbText",
                                             text === "" || isNaN(v) ? "" : String(v))
                  }
                }

                //
                // Color column: click to pick, right-click to reset to theme accent
                //
                Item {
                  Layout.preferredHeight: 24
                  Layout.alignment: Qt.AlignVCenter
                  Layout.preferredWidth: root.colWColor

                  Rectangle {
                    width: 24
                    height: 24
                    radius: 3
                    border.width: 1
                    anchors.centerIn: parent
                    border.color: Cpp_ThemeManager.colors["widget_border"]
                    color: root.markerColor(markerRow.colorHex)

                    Menu {
                      id: colorMenu

                      MenuItem {
                        enabled: markerRow.colorHex.length > 0
                        text: qsTr("Reset to automatic color")
                        onTriggered: markersModel.setProperty(markerRow.index, "colorHex", "")
                      }
                    }

                    MouseArea {
                      hoverEnabled: true
                      anchors.fill: parent
                      cursorShape: Qt.PointingHandCursor
                      acceptedButtons: Qt.LeftButton | Qt.RightButton

                      ToolTip.delay: 700
                      ToolTip.visible: containsMouse
                      ToolTip.text: markerRow.colorHex.length > 0
                                    ? qsTr("Click to choose a color. Right-click to reset to automatic.")
                                    : qsTr("Click to choose a custom color.")

                      onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                          colorMenu.popup()
                          return
                        }

                        colorDialog.targetRow = markerRow.index
                        colorDialog.selectedColor = root.markerColor(markerRow.colorHex)
                        colorDialog.open()
                      }
                    }
                  }
                }

                Widgets.LineField {
                  Layout.fillWidth: true
                  text: markerRow.markerLabel
                  font: Cpp_Misc_CommonFonts.uiFont
                  Layout.alignment: Qt.AlignVCenter
                  placeholderText: qsTr("(optional)")
                  onEditingFinished: markersModel.setProperty(markerRow.index, "markerLabel", text)
                }

                Widgets.IconButton {
                  iconSize: 14
                  padding: 2
                  rotation: 90
                  mirrorIconInRtl: false
                  icon.source: "qrc:/icons/buttons/media-prev.svg"
                  icon.color: enabled
                              ? Cpp_ThemeManager.colors["text"]
                              : Cpp_ThemeManager.colors["mid"]
                  Layout.preferredWidth: root.colWMove
                  Layout.preferredHeight: 24
                  Layout.alignment: Qt.AlignVCenter
                  enabled: markerRow.index > 0
                  ToolTip.text: qsTr("Move up.")
                  ToolTip.visible: hovered
                  ToolTip.delay: 700
                  onClicked: markersModel.move(markerRow.index, markerRow.index - 1, 1)
                }

                Widgets.IconButton {
                  iconSize: 14
                  padding: 2
                  rotation: 90
                  mirrorIconInRtl: false
                  icon.source: "qrc:/icons/buttons/media-next.svg"
                  icon.color: enabled
                              ? Cpp_ThemeManager.colors["text"]
                              : Cpp_ThemeManager.colors["mid"]
                  Layout.preferredWidth: root.colWMove
                  Layout.preferredHeight: 24
                  Layout.alignment: Qt.AlignVCenter
                  enabled: markerRow.index < markersModel.count - 1
                  ToolTip.text: qsTr("Move down.")
                  ToolTip.visible: hovered
                  ToolTip.delay: 700
                  onClicked: markersModel.move(markerRow.index, markerRow.index + 1, 1)
                }

                Widgets.IconButton {
                  iconSize: 14
                  padding: 2
                  icon.source: "qrc:/icons/buttons/trash.svg"
                  icon.color: Cpp_ThemeManager.colors["text"]
                  Layout.preferredWidth: root.colWDelete
                  Layout.alignment: Qt.AlignVCenter
                  ToolTip.text: qsTr("Remove this marker.")
                  ToolTip.visible: hovered
                  ToolTip.delay: 700
                  onClicked: markersModel.remove(markerRow.index)
                }
              }
            }

            //
            // Empty-state hint: visible only when no markers defined
            //
            Label {
              anchors.centerIn: parent
              visible: markersModel.count === 0
              text: qsTr("No markers defined. Pick a preset above or add a marker to get started.")
              color: Cpp_ThemeManager.colors["placeholder_text"]
              font: Cpp_Misc_CommonFonts.uiFont
            }
          }
        }
      }
    }

    //
    // Section 3: Live preview strip (0 Hz to Nyquist)
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true

      Label {
        text: qsTr("Preview")
        font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.minimumWidth: 840
        implicitHeight: previewLayout.implicitHeight + 16
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ColumnLayout {
          id: previewLayout

          spacing: 6
          anchors.margins: 8
          anchors.fill: parent

          Rectangle {
            radius: 3
            border.width: 1
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: Cpp_ThemeManager.colors["window"]
            border.color: Cpp_ThemeManager.colors["widget_border"]

            Item {
              anchors.margins: 3
              anchors.fill: parent

              Repeater {
                model: markersModel
                delegate: Rectangle {
                  id: previewMarker

                  required property int index
                  required property double freqVal
                  required property string endFreqText
                  required property string colorHex

                  readonly property real endFreq: Number(previewMarker.endFreqText)
                  readonly property bool isBand: previewMarker.endFreqText !== ""
                                                 && !isNaN(previewMarker.endFreq)
                                                 && previewMarker.endFreq > previewMarker.freqVal
                  readonly property real fracLo:
                    Math.max(0, Math.min(1, previewMarker.freqVal / root.nyquist))
                  readonly property real fracHi:
                    Math.max(0, Math.min(1, (previewMarker.isBand ? previewMarker.endFreq
                                                                  : previewMarker.freqVal)
                                            / root.nyquist))

                  y: 0
                  height: parent.height
                  color: root.markerColor(previewMarker.colorHex)
                  opacity: previewMarker.isBand ? 0.55 : 0.9
                  x: previewMarker.fracLo * parent.width
                  width: previewMarker.isBand
                         ? Math.max(2, (previewMarker.fracHi - previewMarker.fracLo) * parent.width)
                         : 2
                }
              }
            }
          }

          //
          // Tick labels for 0 Hz / Nyquist
          //
          RowLayout {
            spacing: 0
            Layout.fillWidth: true

            Label {
              text: qsTr("0 Hz")
              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }
            Item { Layout.fillWidth: true }
            Label {
              text: qsTr("%1 Hz").arg(root.nyquist)
              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }
          }
        }
      }
    }

    //
    // Footer: Cancel / Apply
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 4
      Layout.fillWidth: true

      Item { Layout.fillWidth: true }

      Widgets.IconButton {
        iconSize: 16
        ToolTip.delay: 700
        text: qsTr("Cancel")
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Discard changes.")
        icon.source: "qrc:/icons/buttons/close.svg"
        onClicked: root.close()
      }

      Widgets.IconButton {
        iconSize: 16
        highlighted: true
        ToolTip.delay: 700
        text: qsTr("Apply")
        ToolTip.visible: hovered
        icon.source: "qrc:/icons/buttons/apply.svg"
        ToolTip.text: qsTr("Apply changes to the dataset.")
        onClicked: {
          Cpp_JSON_ProjectEditor.commitFrequencyMarkers(root.collectMarkers())
          root.close()
        }
      }
    }
  }
}
