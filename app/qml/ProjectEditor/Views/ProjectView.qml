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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets
import "." as Views

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  title: qsTr("Project Summary")
  icon: Cpp_JSON_ProjectEditor.selectedIcon
  Component.onCompleted: Cpp_JSON_ProjectEditor.buildProjectModel()

  //
  // User interface elements
  //
  Page {
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -10
    }

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      Widgets.ProNotice {
        Layout.margins: -1
        Layout.bottomMargin: 0
        Layout.fillWidth: true
        closeButtonEnabled: false
        titleText: qsTr("Pro features detected in this project.")
        activationFlag: Cpp_JSON_ProjectModel.containsCommercialFeatures
        subtitleText: qsTr("Using fallback widgets. Buy a license to unlock full functionality.")
      }

      Rectangle {
        id: titleBar

        implicitHeight: 48
        Layout.topMargin: -1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.top: parent.top
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        RowLayout {
          anchors {
            leftMargin: 8
            rightMargin: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Label {
            text: qsTr("Project Title:")
            Layout.alignment: Qt.AlignVCenter
          }

          Widgets.LineField {
            id: titleField

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.uiFont
            text: Cpp_JSON_ProjectModel.title
            placeholderText: qsTr("Untitled Project")
            onTextEdited: Cpp_JSON_ProjectModel.setTitle(text)

            Connections {
              target: Cpp_JSON_ProjectModel
              function onTitleChanged() {
                if (!titleField.activeFocus)
                  titleField.text = Cpp_JSON_ProjectModel.title
              }
            }
          }

          Widgets.IconButton {
            id: settingsButton

            iconSize: 18
            text: qsTr("Settings")
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/toolbar/settings.svg"
            onClicked: settingsPopup.visible ? settingsPopup.close() : settingsPopup.open()

            Popup {
              id: settingsPopup

              margins: 8
              padding: 12
              y: settingsButton.height + 4
              closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

              background: Rectangle {
                radius: 4
                color: Cpp_ThemeManager.colors["window"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              contentItem: GridLayout {
                columns: 2
                rowSpacing: 10
                columnSpacing: 12

                Label {
                  text: qsTr("Time Range:")
                  Layout.alignment: Qt.AlignVCenter
                }

                SpinBox {
                  id: timeRangeField

                  readonly property var presets: [0.001, 0.002, 0.005, 0.01, 0.02, 0.05,
                                                  0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 30, 60, 120, 300]

                  function nearestIndex(seconds) {
                    var best = 0
                    for (var i = 0; i < presets.length; ++i)
                      if (Math.abs(presets[i] - seconds) < Math.abs(presets[best] - seconds))
                        best = i

                    return best
                  }

                  function formatSeconds(s) {
                    return s < 1 ? (Math.round(s * 1000) + " ms") : (parseFloat(s.toFixed(3)) + " s")
                  }

                  from: 0
                  to: presets.length - 1
                  editable: true
                  Layout.fillWidth: true
                  value: nearestIndex(Cpp_JSON_ProjectModel.plotTimeRange)
                  textFromValue: function(value, locale) { return timeRangeField.formatSeconds(presets[value]) }
                  valueFromText: function(text, locale) {
                    var t = String(text).toLowerCase()
                    var num = parseFloat(t.replace(/[^0-9.]/g, ""))
                    if (isNaN(num))
                      return timeRangeField.value

                    var secs = (t.indexOf("ms") >= 0) ? num / 1000 : num
                    return timeRangeField.nearestIndex(secs)
                  }
                  onValueModified: Cpp_JSON_ProjectModel.setPlotTimeRange(presets[value])
                }

                Label {
                  text: qsTr("Point Count:")
                  Layout.alignment: Qt.AlignVCenter
                }

                SpinBox {
                  id: pointCountField

                  from: 10
                  to: 100000
                  stepSize: 10
                  editable: true
                  Layout.fillWidth: true
                  value: Cpp_JSON_ProjectModel.pointCount
                  onValueModified: Cpp_JSON_ProjectModel.setPointCount(value)
                }

                Label {
                  Layout.alignment: Qt.AlignVCenter
                  text: qsTr("Change-Driven Transforms:")
                }

                Switch {
                  Layout.alignment: Qt.AlignVCenter
                  checked: Cpp_JSON_ProjectModel.changeDrivenTransforms

                  ToolTip.delay: 700
                  ToolTip.visible: hovered
                  ToolTip.text: qsTr("Run a dataset's transform only when one of its inputs changes. "
                                     + "Speeds up large table-driven projects; off by default.")

                  onToggled: Cpp_JSON_ProjectModel.changeDrivenTransforms = checked
                }
              }
            }
          }
        }
      }

      Rectangle {
        id: statsBar

        implicitHeight: 64
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        property int actionCount: 0

        Connections {
          target: Cpp_JSON_ProjectModel
          function onActionsChanged() {
            statsBar.actionCount = Cpp_JSON_ProjectModel.actionsForDiagram().length
          }
        }

        Component.onCompleted: {
          statsBar.actionCount = Cpp_JSON_ProjectModel.actionsForDiagram().length
        }

        Row {
          spacing: 0
          anchors.fill: parent

          Repeater {
            model: [
              {
                icon:     "qrc:/icons/project-editor/summary/device.svg",
                value:    Cpp_JSON_ProjectModel.sourceCount,
                singular: qsTr("Source"),
                plural:   qsTr("Sources")
              },
              {
                icon:     "qrc:/icons/project-editor/summary/group.svg",
                value:    Cpp_JSON_ProjectModel.groupCount,
                singular: qsTr("Group"),
                plural:   qsTr("Groups")
              },
              {
                icon:     "qrc:/icons/project-editor/summary/dataset.svg",
                value:    Cpp_JSON_ProjectModel.datasetCount,
                singular: qsTr("Dataset"),
                plural:   qsTr("Datasets")
              },
              {
                icon:     "qrc:/icons/project-editor/summary/action.svg",
                value:    statsBar.actionCount,
                singular: qsTr("Action"),
                plural:   qsTr("Actions")
              }
            ]

            delegate: Item {
              height: statsBar.height
              width:  statsBar.width / 4

              Rectangle {
                width: 1
                height: parent.height * 0.6
                visible: index > 0
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              RowLayout {
                spacing: 8
                anchors.centerIn: parent

                Image {
                  source: modelData.icon
                  sourceSize: Qt.size(42, 42)
                  Layout.alignment: Qt.AlignVCenter
                }

                ColumnLayout {
                  spacing: 2
                  Layout.alignment: Qt.AlignVCenter

                  Item {
                    Layout.fillHeight: true
                  }

                  Label {
                    text: modelData.value
                    color: Cpp_ThemeManager.colors["text"]
                    font: Cpp_Misc_CommonFonts.customUiFont(1.2)
                  }

                  Label {
                    opacity: 0.55
                    color: Cpp_ThemeManager.colors["text"]
                    text: modelData.value === 1 ? modelData.singular : modelData.plural
                  }

                  Item {
                    Layout.fillHeight: true
                  }
                }
              }
            }
          }
        }
      }

      Views.FlowDiagram {
        id: diagram

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 280
      }

      Rectangle {
        id: diagramToolbar

        implicitHeight: 36
        Layout.topMargin: -1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.top: parent.top
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        //
        // Always-visible discovery hint (LEFT side, Apple HIG sentence case)
        //
        Label {
          opacity: 0.65
          elide: Label.ElideRight
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.uiFont
          text: qsTr("Double-click a block to edit it. Right-click anywhere "
                     + "to add a group, dataset, action, data table, or device.")

          anchors {
            leftMargin: 12
            rightMargin: 12
            left: parent.left
            right: zoomRow.left
            verticalCenter: parent.verticalCenter
          }
        }

        Row {
          id: zoomRow

          spacing: 2
          anchors {
            rightMargin: 8
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Rectangle {
            radius: 5
            width: 26
            height: 26
            border.width: 1
            border.color: Cpp_ThemeManager.colors["groupbox_border"]
            color: zoomOutArea.pressed
                   ? Cpp_ThemeManager.colors["highlight"]
                   : Cpp_ThemeManager.colors["groupbox_background"]

            Text {
              // code-verify off
              text: "−"
              // code-verify on
              anchors.centerIn: parent
              font: Cpp_Misc_CommonFonts.customUiFont(Cpp_Misc_CommonFonts.kScaleLarge)
              color: zoomOutArea.pressed
                     ? Cpp_ThemeManager.colors["highlighted_text"]
                     : Cpp_ThemeManager.colors["text"]
            }

            MouseArea {
              id: zoomOutArea

              hoverEnabled: true
              anchors.fill: parent
              cursorShape: Qt.PointingHandCursor
              onClicked: diagram.zoom = Math.max(diagram.minZoom, diagram.zoom - 0.15)
            }
          }

          Rectangle {
            radius: 5
            width: 44
            height: 26
            border.width: 1
            border.color: Cpp_ThemeManager.colors["groupbox_border"]
            color: resetArea.pressed
                   ? Cpp_ThemeManager.colors["highlight"]
                   : Cpp_ThemeManager.colors["groupbox_background"]

            Text {
              anchors.centerIn: parent
              text: Math.round(diagram.zoom * 100) + "%"
              font: Cpp_Misc_CommonFonts.customUiFont(Cpp_Misc_CommonFonts.kScaleSmall)
              color: resetArea.pressed
                     ? Cpp_ThemeManager.colors["highlighted_text"]
                     : Cpp_ThemeManager.colors["text"]
            }

            MouseArea {
              id: resetArea

              hoverEnabled: true
              anchors.fill: parent
              cursorShape: Qt.PointingHandCursor
              onClicked: diagram.resetZoom()
            }
          }

          Rectangle {
            radius: 5
            width: 26
            height: 26
            border.width: 1
            border.color: Cpp_ThemeManager.colors["groupbox_border"]
            color: zoomInArea.pressed
                   ? Cpp_ThemeManager.colors["highlight"]
                   : Cpp_ThemeManager.colors["groupbox_background"]

            Text {
              text: "+"
              anchors.centerIn: parent
              font: Cpp_Misc_CommonFonts.customUiFont(Cpp_Misc_CommonFonts.kScaleLarge)
              color: zoomInArea.pressed
                     ? Cpp_ThemeManager.colors["highlighted_text"]
                     : Cpp_ThemeManager.colors["text"]
            }

            MouseArea {
              id: zoomInArea

              hoverEnabled: true
              anchors.fill: parent
              cursorShape: Qt.PointingHandCursor
              onClicked: diagram.zoom = Math.min(diagram.maxZoom, diagram.zoom + 0.15)
            }
          }
        }
      }
    }
  }
}
