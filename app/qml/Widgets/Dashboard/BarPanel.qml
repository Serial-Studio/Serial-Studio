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
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property BarPanelModel model

  //
  // Freeze-mode title gate (same contract as the other painted-title widgets)
  //
  readonly property bool titleFrozenOut: windowRoot && windowRoot.frozen === true
                                         && windowRoot.effectiveFreezeTitle !== "painted"
  readonly property string displayTitle: windowRoot && windowRoot.title ? windowRoot.title
                                                                        : ""

  //
  // Change counter: the sole notify dependency of every per-row binding below. Rows are read
  // through the model's scalar accessors, so one tick converts scalars, never whole lists.
  //
  readonly property int rev: model ? model.revision : 0

  //
  // Orientation: the persisted style wins, auto picks columns only when every channel
  // gets a usable column width on a panel that is clearly wider than tall
  //
  readonly property int rowCount: model.count
  readonly property bool vertical: {
    if (model.styleMode === "vertical")
      return true

    if (model.styleMode === "horizontal")
      return false

    return root.width > root.height * 1.2
        && rowCount > 0 && (root.width / rowCount) >= 64
  }

  //
  // Shared metrics
  //
  readonly property real valueWidth: Math.max(70, root.width * 0.20)
  readonly property real labelWidth: Math.min(160, Math.max(64, root.width * 0.26))

  //
  // Severity tint: active band color when bands exist, else the dataset accent (the group
  // widget's own color is transparent); alarmColorForSeverity(-1) is WARNING, gate -1 explicitly
  //
  function fillColor(index) {
    const severity = model.severity(index)
    if (severity >= 0)
      return Cpp_ThemeManager.alarmColorForSeverity(severity)

    return (Cpp_ThemeManager.colors, model.rowColor(index))
  }

  function valueColor(index) {
    const severity = model.severity(index)
    if (severity >= 2)
      return Cpp_ThemeManager.alarmColorForSeverity(severity)

    return Cpp_ThemeManager.colors["widget_text"]
  }

  function bandColor(band) {
    if (band.customColor && band.customColor.length > 0)
      return band.customColor

    return Cpp_ThemeManager.alarmColorForSeverity(band.severity)
  }

  ColumnLayout {
    spacing: 4
    anchors.fill: parent

    //
    // Panel title (hidden on small widgets and while frozen without painted titles). The layout
    // hugs the widget so the scrolling views clip at its border; padding lives in the margins here
    //
    WidgetTitleBar {
      Layout.topMargin: 8
      Layout.leftMargin: 8
      Layout.rightMargin: 8
      Layout.fillWidth: true
      text: root.displayTitle
      active: root.height >= 110 && !root.titleFrozenOut && root.displayTitle.length > 0
    }

    //
    // HORIZONTAL: one labeled row per dataset; rows scroll once they no longer fit
    //
    Flickable {
      id: rowsFlick

      readonly property bool overflowing: contentHeight > height

      clip: true
      contentWidth: width
      Layout.fillWidth: true
      visible: !root.vertical
      Layout.fillHeight: true
      interactive: overflowing
      boundsBehavior: Flickable.StopAtBounds
      contentHeight: Math.max(rowsLayout.implicitHeight, height)

      ScrollBar.vertical: ScrollBar {
        id: rowsScroll

        policy: rowsFlick.overflowing ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
      }

      ColumnLayout {
        id: rowsLayout

        x: 8
        spacing: 2
        height: rowsFlick.contentHeight
        width: rowsFlick.width - 16 - (rowsFlick.overflowing ? rowsScroll.width : 0)

        Item {
          Layout.preferredHeight: 8 - rowsLayout.spacing
        }

        Repeater {
          model: root.vertical ? 0 : root.rowCount

          delegate: RowLayout {
            id: barRow

            required property int index
            property real frac: (root.rev, root.model.frac(index))

            Behavior on frac { SpringAnimation { spring: 4.5; damping: 0.4; epsilon: 0.001 } }

            spacing: 6
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 22
            Layout.preferredHeight: 28

            Text {
              id: hTitle

              elide: Text.ElideRight
              visible: root.width >= 220
              text: root.model.titles[barRow.index]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.9, false))
              color: Cpp_ThemeManager.colors["widget_text"]
              Layout.minimumWidth: root.labelWidth
              Layout.maximumWidth: root.labelWidth
              Layout.preferredWidth: root.labelWidth

              HoverHandler {
                id: hTitleHover

                enabled: hTitle.truncated
              }
              ToolTip.delay: 600
              ToolTip.text: hTitle.text
              ToolTip.visible: hTitleHover.hovered && hTitle.truncated
            }

            Item {
              id: hTrack

              Layout.fillWidth: true
              Layout.fillHeight: true
              Layout.maximumHeight: 22
              Layout.alignment: Qt.AlignVCenter
              visible: root.model.ranged[barRow.index]

              Rectangle {
                id: hWell

                radius: 2
                border.width: 1
                antialiasing: true
                anchors.fill: parent
                border.color: Qt.rgba(0, 0, 0, 0.25)
                color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.12)
              }

              Repeater {
                model: root.model.bands[barRow.index]

                delegate: Item {
                  id: hBandHost

                  required property var modelData

                  Rectangle {
                    id: hBandZone

                    opacity: 0.32
                    y: hWell.y + 1
                    antialiasing: true
                    height: hWell.height - 2
                    color: root.bandColor(hBandHost.modelData)
                    x: hWell.x + 1 + hBandHost.modelData.fracMin * (hWell.width - 2)
                    width: Math.max(0, (hBandHost.modelData.fracMax - hBandHost.modelData.fracMin)
                                       * (hWell.width - 2))
                  }

                  HdrBoost {
                    target: hBandZone
                    anchors.fill: hBandZone
                    boost: Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
                  }
                }
              }

              //
              // Recess shading over track and bands: top-edge falloff plus a bottom
              // catchlight, so the well reads as sunken instead of a painted stripe
              //
              Rectangle {
                radius: 1
                x: hWell.x + 1
                y: hWell.y + 1
                antialiasing: true
                width: hWell.width - 2
                height: hWell.height - 2
                gradient: Gradient {
                  GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.28) }
                  GradientStop { position: 0.55; color: "transparent" }
                  GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.10) }
                }
              }

              Rectangle {
                id: hFill

                radius: 1
                x: hWell.x + 1
                y: hWell.y + 1
                antialiasing: true
                height: hWell.height - 2
                color: (root.rev, root.fillColor(barRow.index))
                width: Math.max(0, barRow.frac * (hWell.width - 2))
                visible: (root.rev, root.model.isNumeric(barRow.index))

                //
                // Cylindrical sheen across the juice (highlight top, shade bottom)
                //
                Rectangle {
                  antialiasing: true
                  anchors.fill: parent
                  gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.30) }
                    GradientStop { position: 0.5; color: Qt.rgba(1, 1, 1, 0.04) }
                    GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.18) }
                  }
                }

                Rectangle {
                  width: 2
                  opacity: 0.85
                  anchors.top: parent.top
                  anchors.right: parent.right
                  anchors.bottom: parent.bottom
                  color: Cpp_ThemeManager.colors["bright_text"]
                }
              }

              HdrBoost {
                target: hFill
                anchors.fill: hFill
                boost: Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
              }

              Repeater {
                model: 2

                delegate: Rectangle {
                  required property int index
                  readonly property real markerFrac:
                      (root.rev, index === 0 ? root.model.minSeenFrac(barRow.index)
                                             : root.model.maxSeenFrac(barRow.index))
                  width: 2
                  opacity: 0.9
                  y: hWell.y - 2
                  antialiasing: true
                  height: hWell.height + 4
                  color: Cpp_ThemeManager.colors["widget_text"]
                  visible: (root.rev, root.model.hasExtremes(barRow.index))
                  x: hWell.x + 1 + markerFrac * (hWell.width - 2) - 1
                }
              }

              //
              // Well ring redrawn opaque above juice, bands and markers so the track
              // edge stays crisp and nothing bleeds into the border
              //
              Rectangle {
                radius: 2
                border.width: 1
                antialiasing: true
                anchors.fill: hWell
                color: "transparent"
                border.color: Qt.rgba(0, 0, 0, 0.28)
              }
            }

            Item {
              Layout.fillWidth: true
              visible: !root.model.ranged[barRow.index]
            }

            Text {
              id: hValue

              elide: Text.ElideLeft
              horizontalAlignment: Text.AlignRight
              Layout.minimumWidth: root.valueWidth
              Layout.maximumWidth: root.valueWidth
              Layout.preferredWidth: root.valueWidth
              color: (root.rev, root.valueColor(barRow.index))
              text: (root.rev, root.model.valueText(barRow.index))
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.9, true))

              HoverHandler {
                id: hValueHover

                enabled: hValue.truncated
              }
              ToolTip.delay: 600
              ToolTip.text: hValue.text
              ToolTip.visible: hValueHover.hovered && hValue.truncated
            }

            //
            // Pop-out buttons for the other widgets showing this dataset, beside the value
            //
            DatasetWidgetButtons {
              buttonSize: 24
              windowRoot: root.windowRoot
              Layout.alignment: Qt.AlignVCenter
              widgets: root.model.widgets[barRow.index]
              visible: root.width >= 220 && widgets.length > 0
            }
          }
        }

        Item {
          Layout.preferredHeight: 8 - rowsLayout.spacing
        }
      }
    }

    //
    // VERTICAL: rake-style columns, value above, label below; columns scroll once they
    // no longer fit
    //
    Flickable {
      id: columnsFlick

      readonly property bool overflowing: contentWidth > width

      clip: true
      contentHeight: height
      visible: root.vertical
      Layout.fillWidth: true
      Layout.fillHeight: true
      interactive: overflowing
      boundsBehavior: Flickable.StopAtBounds
      contentWidth: Math.max(columnsLayout.implicitWidth, width)

      ScrollBar.horizontal: ScrollBar {
        id: columnsScroll

        policy: columnsFlick.overflowing ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
      }

      RowLayout {
        id: columnsLayout

        y: 8
        spacing: 6
        width: columnsFlick.contentWidth
        height: columnsFlick.height - 16 - (columnsFlick.overflowing ? columnsScroll.height : 0)

        Item {
          Layout.preferredWidth: 8 - columnsLayout.spacing
        }

        Repeater {
          model: root.vertical ? root.rowCount : 0

          delegate: ColumnLayout {
            id: barColumn

            required property int index
            property real frac: (root.rev, root.model.frac(index))
            Behavior on frac { SpringAnimation { spring: 4.5; damping: 0.4; epsilon: 0.001 } }

            spacing: 2
            Layout.fillWidth: true
            Layout.fillHeight: true

            //
            // Equal preferred width keeps the rake evenly divided; without it each column
            // inherits its own text's implicitWidth and they redistribute as values change
            //
            Layout.minimumWidth: 64
            Layout.preferredWidth: 72

            Text {
              id: vValue

              elide: Text.ElideRight
              Layout.fillWidth: true
              Layout.preferredWidth: 1
              horizontalAlignment: Text.AlignHCenter
              color: (root.rev, root.valueColor(barColumn.index))
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.9, true))
              text: (root.rev, root.model.valueText(barColumn.index))

              HoverHandler {
                id: vValueHover

                enabled: vValue.truncated
              }
              ToolTip.delay: 600
              ToolTip.text: vValue.text
              ToolTip.visible: vValueHover.hovered && vValue.truncated
            }

            //
            // Pop-out buttons for the other widgets showing this dataset, under the value
            //
            DatasetWidgetButtons {
              buttonSize: 22
              windowRoot: root.windowRoot
              Layout.alignment: Qt.AlignHCenter
              widgets: root.model.widgets[barColumn.index]
              visible: root.height >= 140 && widgets.length > 0
            }

            Item {
              id: vTrack

              Layout.fillWidth: true
              Layout.fillHeight: true
              Layout.alignment: Qt.AlignHCenter
              visible: root.model.ranged[barColumn.index]

              Rectangle {
                id: vWell

                radius: 2
                border.width: 1
                antialiasing: true
                height: parent.height
                border.color: Qt.rgba(0, 0, 0, 0.25)
                width: Math.min(parent.width * 0.55, 40)
                anchors.horizontalCenter: parent.horizontalCenter
                color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.12)
              }

              Repeater {
                model: root.model.bands[barColumn.index]

                delegate: Item {
                  id: vBandHost

                  required property var modelData

                  Rectangle {
                    id: vBandZone

                    opacity: 0.32
                    x: vWell.x + 1
                    antialiasing: true
                    width: vWell.width - 2
                    color: root.bandColor(vBandHost.modelData)
                    y: vWell.y + 1 + (1 - vBandHost.modelData.fracMax) * (vWell.height - 2)
                    height: Math.max(0, (vBandHost.modelData.fracMax - vBandHost.modelData.fracMin)
                                        * (vWell.height - 2))
                  }

                  HdrBoost {
                    target: vBandZone
                    anchors.fill: vBandZone
                    boost: Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
                  }
                }
              }

              //
              // Recess shading over track and bands: left-edge falloff plus a right
              // catchlight, the vertical analog of the row wells
              //
              Rectangle {
                radius: 1
                x: vWell.x + 1
                y: vWell.y + 1
                antialiasing: true
                width: vWell.width - 2
                height: vWell.height - 2
                gradient: Gradient {
                  orientation: Gradient.Horizontal
                  GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.28) }
                  GradientStop { position: 0.55; color: "transparent" }
                  GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.10) }
                }
              }

              Rectangle {
                id: vFill

                radius: 1
                x: vWell.x + 1
                antialiasing: true
                width: vWell.width - 2
                color: (root.rev, root.fillColor(barColumn.index))
                height: Math.max(0, barColumn.frac * (vWell.height - 2))
                visible: (root.rev, root.model.isNumeric(barColumn.index))
                y: vWell.y + 1 + (1 - barColumn.frac) * (vWell.height - 2)

                //
                // Cylindrical sheen across the juice (highlight left, shade right)
                //
                Rectangle {
                  antialiasing: true
                  anchors.fill: parent
                  gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.30) }
                    GradientStop { position: 0.5; color: Qt.rgba(1, 1, 1, 0.04) }
                    GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.18) }
                  }
                }

                Rectangle {
                  height: 2
                  opacity: 0.85
                  anchors.top: parent.top
                  anchors.left: parent.left
                  anchors.right: parent.right
                  color: Cpp_ThemeManager.colors["bright_text"]
                }
              }

              HdrBoost {
                target: vFill
                anchors.fill: vFill
                boost: Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
              }

              Repeater {
                model: 2

                delegate: Rectangle {
                  required property int index
                  readonly property real markerFrac:
                      (root.rev, index === 0 ? root.model.minSeenFrac(barColumn.index)
                                             : root.model.maxSeenFrac(barColumn.index))
                  height: 2
                  opacity: 0.9
                  x: vWell.x - 2
                  antialiasing: true
                  width: vWell.width + 4
                  color: Cpp_ThemeManager.colors["widget_text"]
                  visible: (root.rev, root.model.hasExtremes(barColumn.index))
                  y: vWell.y + 1 + (1 - markerFrac) * (vWell.height - 2) - 1
                }
              }

              //
              // Well ring redrawn opaque above juice, bands and markers so the track
              // edge stays crisp and nothing bleeds into the border
              //
              Rectangle {
                radius: 2
                border.width: 1
                antialiasing: true
                anchors.fill: vWell
                color: "transparent"
                border.color: Qt.rgba(0, 0, 0, 0.28)
              }
            }

            Text {
              id: vTitle

              elide: Text.ElideRight
              Layout.fillWidth: true
              Layout.preferredWidth: 1
              visible: root.height >= 140
              horizontalAlignment: Text.AlignHCenter
              text: root.model.titles[barColumn.index]
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.85, false))

              HoverHandler {
                id: vTitleHover

                enabled: vTitle.truncated
              }
              ToolTip.delay: 600
              ToolTip.text: vTitle.text
              ToolTip.visible: vTitleHover.hovered && vTitle.truncated
            }
          }
        }

        Item {
          Layout.preferredWidth: 8 - columnsLayout.spacing
        }
      }
    }
  }
}
