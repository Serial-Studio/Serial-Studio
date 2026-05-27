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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
  title: qsTr("Alarm Bands")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Bound dataset identifiers + scale range. Populated by showDialog().
  //
  property int groupId: -1
  property real rangeMin: 0
  property int datasetId: -1
  property real rangeMax: 100

  //
  // Shared table column widths. Single source of truth so the column header row
  // and the band delegate rows line up regardless of style metrics.
  //
  readonly property int colWMin: 90
  readonly property int colWMax: 90
  readonly property int colWMove: 24
  readonly property int colWColor: 28
  readonly property int colSpacing: 8
  readonly property int colWDelete: 28
  readonly property int colWSeverity: 130
  readonly property int colScrollSlot: 12

  //
  // Editable working copy of the bands list. ListModel rows: bandMin, bandMax, severity,
  // colorHex, bandLabel.
  //
  ListModel { id: bandsModel }

  //
  // Severity labels exposed via index = enum int.
  //
  readonly property var severityLabels: [
    qsTr("Info"),
    qsTr("OK"),
    qsTr("Warning"),
    qsTr("Critical")
  ]

  //
  // Presets: author-friendly band setups for common gauges. Fractional bounds get
  // scaled to the dataset's wgtMin/wgtMax at apply time.
  //
  readonly property var presets: [
    {
      label: qsTr("Tachometer"),
      bands: [
        { fracMin: 0.00, fracMax: 0.55, severity: 0, color: "", label: qsTr("Idle") },
        { fracMin: 0.55, fracMax: 0.85, severity: 1, color: "", label: qsTr("Operating") },
        { fracMin: 0.85, fracMax: 0.95, severity: 2, color: "", label: qsTr("Caution") },
        { fracMin: 0.95, fracMax: 1.00, severity: 3, color: "", label: qsTr("Redline") }
      ]
    },
    {
      label: qsTr("Speedometer"),
      bands: [
        { fracMin: 0.00, fracMax: 0.65, severity: 1, color: "", label: qsTr("Cruise") },
        { fracMin: 0.65, fracMax: 0.90, severity: 2, color: "", label: qsTr("Fast") },
        { fracMin: 0.90, fracMax: 1.00, severity: 3, color: "", label: qsTr("Top Speed") }
      ]
    },
    {
      label: qsTr("Engine Temperature"),
      bands: [
        { fracMin: 0.00, fracMax: 0.25, severity: 0, color: "", label: qsTr("Cold") },
        { fracMin: 0.25, fracMax: 0.70, severity: 1, color: "", label: qsTr("Normal") },
        { fracMin: 0.70, fracMax: 0.90, severity: 2, color: "", label: qsTr("Warm") },
        { fracMin: 0.90, fracMax: 1.00, severity: 3, color: "", label: qsTr("Overheat") }
      ]
    },
    {
      label: qsTr("Pressure"),
      bands: [
        { fracMin: 0.00, fracMax: 0.15, severity: 3, color: "", label: qsTr("Vacuum") },
        { fracMin: 0.15, fracMax: 0.75, severity: 1, color: "", label: qsTr("Normal") },
        { fracMin: 0.75, fracMax: 0.90, severity: 2, color: "", label: qsTr("High") },
        { fracMin: 0.90, fracMax: 1.00, severity: 3, color: "", label: qsTr("Burst") }
      ]
    },
    {
      label: qsTr("Battery Voltage"),
      bands: [
        { fracMin: 0.00, fracMax: 0.20, severity: 3, color: "", label: qsTr("Low") },
        { fracMin: 0.20, fracMax: 0.85, severity: 1, color: "", label: qsTr("Nominal") },
        { fracMin: 0.85, fracMax: 1.00, severity: 2, color: "", label: qsTr("High") }
      ]
    },
    {
      label: qsTr("Fuel Level"),
      bands: [
        { fracMin: 0.00, fracMax: 0.10, severity: 3, color: "", label: qsTr("Empty") },
        { fracMin: 0.10, fracMax: 0.25, severity: 2, color: "", label: qsTr("Reserve") },
        { fracMin: 0.25, fracMax: 1.00, severity: 1, color: "", label: qsTr("OK") }
      ]
    },
    {
      label: qsTr("Signal Strength"),
      bands: [
        { fracMin: 0.00, fracMax: 0.15, severity: 3, color: "", label: qsTr("No Signal") },
        { fracMin: 0.15, fracMax: 0.50, severity: 2, color: "", label: qsTr("Weak") },
        { fracMin: 0.50, fracMax: 1.00, severity: 1, color: "", label: qsTr("Good") }
      ]
    },
    {
      label: qsTr("CPU / System Load"),
      bands: [
        { fracMin: 0.00, fracMax: 0.50, severity: 1, color: "", label: qsTr("Normal") },
        { fracMin: 0.50, fracMax: 0.80, severity: 2, color: "", label: qsTr("Busy") },
        { fracMin: 0.80, fracMax: 1.00, severity: 3, color: "", label: qsTr("Overload") }
      ]
    },
    {
      label: qsTr("OK / Warning / Critical"),
      bands: [
        { fracMin: 0.00, fracMax: 0.33, severity: 1, color: "", label: qsTr("OK") },
        { fracMin: 0.33, fracMax: 0.66, severity: 2, color: "", label: qsTr("Warning") },
        { fracMin: 0.66, fracMax: 1.00, severity: 3, color: "", label: qsTr("Critical") }
      ]
    }
  ]

  //
  // Public API
  //
  function showDialog(gId, dId, rMin, rMax, initialBands) {
    root.groupId   = gId
    root.datasetId = dId
    root.rangeMin  = rMin
    root.rangeMax  = rMax
    root.loadBands(initialBands)
    presetCombo.currentIndex = -1
    root.show()
    root.raise()
    root.requestActivate()
  }

  function loadBands(arr) {
    bandsModel.clear()
    if (!arr)
      return

    for (let i = 0; i < arr.length; ++i) {
      const b = arr[i]
      bandsModel.append({
        bandMin: Number(b.min ?? 0),
        bandMax: Number(b.max ?? 0),
        severity: Number(b.severity ?? 2),
        colorHex: String(b.color ?? ""),
        bandLabel: String(b.label ?? "")
      })
    }
  }

  function collectBands() {
    const out = []
    for (let i = 0; i < bandsModel.count; ++i) {
      const r = bandsModel.get(i)
      if (Number(r.bandMax) <= Number(r.bandMin))
        continue

      out.push({
        min: Number(r.bandMin),
        max: Number(r.bandMax),
        severity: Number(r.severity),
        color: String(r.colorHex),
        label: String(r.bandLabel)
      })
    }
    return out
  }

  function applyPreset(presetIdx) {
    if (presetIdx < 0 || presetIdx >= root.presets.length)
      return

    const preset = root.presets[presetIdx]
    const range  = root.rangeMax - root.rangeMin
    bandsModel.clear()
    for (let i = 0; i < preset.bands.length; ++i) {
      const b = preset.bands[i]
      bandsModel.append({
        bandMin: root.rangeMin + b.fracMin * range,
        bandMax: root.rangeMin + b.fracMax * range,
        severity: b.severity,
        colorHex: b.color,
        bandLabel: b.label
      })
    }
  }

  function addBand() {
    const range = root.rangeMax - root.rangeMin
    const lo    = bandsModel.count > 0
                  ? Number(bandsModel.get(bandsModel.count - 1).bandMax)
                  : root.rangeMin
    const hi    = Math.min(root.rangeMax, lo + range * 0.10)
    bandsModel.append({
      bandMin: lo,
      bandMax: hi,
      severity: 2,
      colorHex: "",
      bandLabel: ""
    })
  }

  //
  // Resolves the on-screen color of a band (custom override OR severity default).
  //
  function bandColor(colorHex, severity) {
    if (colorHex && colorHex.length > 0)
      return colorHex

    return Cpp_ThemeManager.alarmColorForSeverity(severity)
  }

  //
  // Converts a Qt color object to a "#rrggbb" hex string (alpha discarded; bands are opaque).
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

    title: qsTr("Choose Band Color")
    onAccepted: {
      if (colorDialog.targetRow >= 0 && colorDialog.targetRow < bandsModel.count)
        bandsModel.setProperty(colorDialog.targetRow, "colorHex",
                               root.hexFromColor(colorDialog.selectedColor))
    }
  }

  //--------------------------------------------------------------------------------------------
  // Layout: section labels live outside their content cards; no internal separators
  //--------------------------------------------------------------------------------------------

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    Layout.minimumWidth: 760
    anchors.centerIn: parent

    //
    // Section 1: Preset picker + range readout
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
        Layout.minimumWidth: 760
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
            text: qsTr("Range")
            color: Cpp_ThemeManager.colors["placeholder_text"]
            Layout.alignment: Qt.AlignVCenter
          }

          Label {
            text: root.rangeMin.toFixed(2) + " - " + root.rangeMax.toFixed(2)
            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["text"]
            Layout.alignment: Qt.AlignVCenter
          }
        }
      }
    }

    //
    // Section 2: Bands table
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          text: qsTr("Bands")
          font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
          color: Cpp_ThemeManager.colors["pane_section_label"]
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        Item { Layout.fillWidth: true }

        Widgets.IconButton {
          iconSize: 14
          ToolTip.delay: 700
          text: qsTr("Add Band")
          ToolTip.visible: hovered
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/icons/buttons/plus.svg"
          ToolTip.text: qsTr("Add a new band continuing from the last one.")
          onClicked: root.addBand()
        }
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 760
        Layout.minimumHeight: 220
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ColumnLayout {
          id: bandsLayout

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
              text: qsTr("Min")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.preferredWidth: root.colWMin
            }
            Label {
              text: qsTr("Max")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.preferredWidth: root.colWMax
            }
            Label {
              text: qsTr("Severity")
              font: Cpp_Misc_CommonFonts.boldUiFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
              Layout.preferredWidth: root.colWSeverity
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
          // Band rows
          //
          ListView {
            id: bandList

            spacing: 1
            clip: true
            model: bandsModel
            Layout.topMargin: 4
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 160
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
              policy: ScrollBar.AsNeeded
            }

            delegate: Item {
              id: bandRow

              required property int index
              required property int severity
              required property double bandMin
              required property double bandMax
              required property string colorHex
              required property string bandLabel

              implicitHeight: 36
              width: ListView.view.width

              RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 0
                spacing: root.colSpacing
                anchors.rightMargin: root.colScrollSlot

                Widgets.LineField {
                  text: bandRow.bandMin.toFixed(2)
                  font: Cpp_Misc_CommonFonts.monoFont
                  Layout.preferredWidth: root.colWMin
                  Layout.alignment: Qt.AlignVCenter
                  validator: DoubleValidator { bottom: -1e12; top: 1e12 }
                  onEditingFinished: {
                    const v = Number(text)
                    if (!isNaN(v))
                      bandsModel.setProperty(bandRow.index, "bandMin", v)
                  }
                }

                Widgets.LineField {
                  text: bandRow.bandMax.toFixed(2)
                  font: Cpp_Misc_CommonFonts.monoFont
                  Layout.preferredWidth: root.colWMax
                  Layout.alignment: Qt.AlignVCenter
                  validator: DoubleValidator { bottom: -1e12; top: 1e12 }
                  onEditingFinished: {
                    const v = Number(text)
                    if (!isNaN(v))
                      bandsModel.setProperty(bandRow.index, "bandMax", v)
                  }
                }

                Widgets.Combo {
                  model: root.severityLabels
                  currentIndex: bandRow.severity
                  font: Cpp_Misc_CommonFonts.uiFont
                  Layout.alignment: Qt.AlignVCenter
                  Layout.preferredWidth: root.colWSeverity
                  onActivated: bandsModel.setProperty(bandRow.index, "severity", currentIndex)
                }

                //
                // Color column: click to pick, right-click to reset to severity default
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
                    color: root.bandColor(bandRow.colorHex, bandRow.severity)

                    Menu {
                      id: colorMenu

                      MenuItem {
                        enabled: bandRow.colorHex.length > 0
                        text: qsTr("Reset to severity default")
                        onTriggered: bandsModel.setProperty(bandRow.index, "colorHex", "")
                      }
                    }

                    MouseArea {
                      hoverEnabled: true
                      anchors.fill: parent
                      cursorShape: Qt.PointingHandCursor
                      acceptedButtons: Qt.LeftButton | Qt.RightButton

                      ToolTip.delay: 700
                      ToolTip.visible: containsMouse
                      ToolTip.text: bandRow.colorHex.length > 0
                                    ? qsTr("Click to choose a color. Right-click to reset to severity default.")
                                    : qsTr("Click to choose a custom color.")

                      onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                          colorMenu.popup()
                          return
                        }

                        colorDialog.targetRow     = bandRow.index
                        colorDialog.selectedColor = root.bandColor(bandRow.colorHex,
                                                                   bandRow.severity)
                        colorDialog.open()
                      }
                    }
                  }
                }

                Widgets.LineField {
                  Layout.fillWidth: true
                  text: bandRow.bandLabel
                  font: Cpp_Misc_CommonFonts.uiFont
                  Layout.alignment: Qt.AlignVCenter
                  placeholderText: qsTr("(optional)")
                  onEditingFinished: bandsModel.setProperty(bandRow.index, "bandLabel", text)
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
                  enabled: bandRow.index > 0
                  ToolTip.text: qsTr("Move up.")
                  ToolTip.visible: hovered
                  ToolTip.delay: 700
                  onClicked: bandsModel.move(bandRow.index, bandRow.index - 1, 1)
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
                  enabled: bandRow.index < bandsModel.count - 1
                  ToolTip.text: qsTr("Move down.")
                  ToolTip.visible: hovered
                  ToolTip.delay: 700
                  onClicked: bandsModel.move(bandRow.index, bandRow.index + 1, 1)
                }

                Widgets.IconButton {
                  iconSize: 14
                  padding: 2
                  icon.source: "qrc:/icons/buttons/trash.svg"
                  icon.color: Cpp_ThemeManager.colors["text"]
                  Layout.preferredWidth: root.colWDelete
                  Layout.alignment: Qt.AlignVCenter
                  ToolTip.text: qsTr("Remove this band.")
                  ToolTip.visible: hovered
                  ToolTip.delay: 700
                  onClicked: bandsModel.remove(bandRow.index)
                }
              }
            }

            //
            // Empty-state hint: visible only when no bands defined
            //
            Label {
              anchors.centerIn: parent
              visible: bandsModel.count === 0
              text: qsTr("No bands defined. Pick a preset above or add a band to get started.")
              color: Cpp_ThemeManager.colors["placeholder_text"]
              font: Cpp_Misc_CommonFonts.uiFont
            }
          }
        }
      }
    }

    //
    // Section 3: Live preview strip
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
        Layout.minimumWidth: 760
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
                model: bandsModel
                delegate: Rectangle {
                  id: previewBand

                  required property int index
                  required property double bandMin
                  required property double bandMax
                  required property int severity
                  required property string colorHex

                  readonly property real range:  Math.max(0.001, root.rangeMax - root.rangeMin)
                  readonly property real fracLo: Math.max(0, Math.min(1, (previewBand.bandMin - root.rangeMin) / previewBand.range))
                  readonly property real fracHi: Math.max(0, Math.min(1, (previewBand.bandMax - root.rangeMin) / previewBand.range))

                  y: 0
                  opacity: 0.85
                  height: parent.height
                  x: previewBand.fracLo * parent.width
                  color: root.bandColor(previewBand.colorHex, previewBand.severity)
                  width: Math.max(0, (previewBand.fracHi - previewBand.fracLo) * parent.width)
                }
              }
            }
          }

          //
          // Tick labels for min / max
          //
          RowLayout {
            spacing: 0
            Layout.fillWidth: true

            Label {
              text: root.rangeMin.toFixed(2)
              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }
            Item { Layout.fillWidth: true }
            Label {
              text: root.rangeMax.toFixed(2)
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
          Cpp_JSON_ProjectEditor.commitAlarmBands(root.collectBands())
          root.close()
        }
      }
    }
  }
}
