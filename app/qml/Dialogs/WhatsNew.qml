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

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  title: qsTr("What's New in %1").arg(Cpp_AppName)

  //
  // Size hints
  //
  minimumWidth: 560
  minimumHeight: 460
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Open a help page anchor on serial-studio.com
  //
  function learnMore(helpId) {
    if (helpId.length > 0)
      Qt.openUrlExternally("https://serial-studio.com/help#" + helpId)
  }

  //
  // Per-release highlights. icon points to the whats-new asset set; helpId maps to a
  // help.json page; topColor/bottomColor tint the icon badge with a friendly gradient.
  //
  property var features: [
    {
      icon: "qrc:/icons/whats-new/scripting.svg",
      topColor: "#5dade2",
      bottomColor: "#2471a3",
      title: qsTr("Lua & Built-In Parsers"),
      description: qsTr("Parse frames in Lua 5.4 or with code-free Built-In templates, " +
                        "alongside JavaScript."),
      helpId: "javascript-api"
    },
    {
      icon: "qrc:/icons/whats-new/ai.svg",
      topColor: "#a569bd",
      bottomColor: "#6c3483",
      title: qsTr("AI Assistant"),
      description: qsTr("An in-app assistant across eight providers that can build and " +
                        "edit projects for you."),
      helpId: "ai-assistant"
    },
    {
      icon: "qrc:/icons/whats-new/control-script.svg",
      topColor: "#26c6da",
      bottomColor: "#00838f",
      title: qsTr("Device Control Loops"),
      description: qsTr("Automate your device with an Arduino-style setup() and loop() " +
                        "routine that can read, write, and drive the dashboard."),
      helpId: "control-script"
    },
    {
      icon: "qrc:/icons/whats-new/oscilloscope.svg",
      topColor: "#48c9b0",
      bottomColor: "#148f77",
      title: qsTr("Oscilloscope Sweep & Trigger"),
      description: qsTr("Scope-style sweep with an animated trigger cursor you can drag " +
                        "on the plot."),
      helpId: "plots"
    },
    {
      icon: "qrc:/icons/whats-new/output.svg",
      topColor: "#58d68d",
      bottomColor: "#1e8449",
      title: qsTr("Output Controls"),
      description: qsTr("Transmit back to your device with control widgets and a " +
                        "protocol-helper engine."),
      helpId: "output-controls"
    },
    {
      icon: "qrc:/icons/whats-new/workspaces.svg",
      topColor: "#f5b041",
      bottomColor: "#b9770e",
      title: qsTr("Dashboard Workspaces"),
      description: qsTr("Group widgets into your own dashboards and find them from the " +
                        "taskbar search."),
      helpId: "widget-reference"
    },
    {
      icon: "qrc:/icons/whats-new/sessions.svg",
      topColor: "#ec7063",
      bottomColor: "#922b21",
      title: qsTr("Session Database & Reports"),
      description: qsTr("Record sessions to SQLite, replay them, and export HTML or PDF " +
                        "reports."),
      helpId: "session-database"
    },
    {
      icon: "qrc:/icons/whats-new/deployments.svg",
      topColor: "#5d6d7e",
      bottomColor: "#2e4053",
      title: qsTr("Operator Deployments"),
      description: qsTr("Ship a locked-down, single-purpose build to operators with a " +
                        "runtime-only mode."),
      helpId: "operator-deployments"
    },
    {
      icon: "qrc:/icons/whats-new/widgets.svg",
      topColor: "#af7ac5",
      bottomColor: "#7d3c98",
      title: qsTr("New Dashboard Widgets"),
      description: qsTr("Gauge and Meter faces with live readouts, plus Clock, Stopwatch, " +
                        "and Waterfall."),
      helpId: "widget-reference"
    },
    {
      icon: "qrc:/icons/whats-new/transforms.svg",
      topColor: "#7986cb",
      bottomColor: "#3949ab",
      title: qsTr("Dataset Transforms"),
      description: qsTr("Calibrate, filter, and convert each channel with a per-dataset " +
                        "function, or add virtual datasets that compute new channels."),
      helpId: "dataset-transforms"
    },
    {
      icon: "qrc:/icons/whats-new/painter.svg",
      topColor: "#f06292",
      bottomColor: "#ad1457",
      title: qsTr("Painter Widget"),
      description: qsTr("Draw fully custom graphics from incoming data with your own " +
                        "drawing script."),
      helpId: "painter-widget"
    },
    {
      icon: "qrc:/icons/whats-new/tables.svg",
      topColor: "#4dd0e1",
      bottomColor: "#00838f",
      title: qsTr("Data Tables"),
      description: qsTr("Live register-style tables with virtual datasets and editable " +
                        "cells."),
      helpId: "data-tables"
    },
    {
      icon: "qrc:/icons/whats-new/image.svg",
      topColor: "#4db6ac",
      bottomColor: "#00695c",
      title: qsTr("Image Widget"),
      description: qsTr("Decode and display image frames streamed straight from your " +
                        "device."),
      helpId: "widget-reference"
    },
    {
      icon: "qrc:/icons/whats-new/alarms.svg",
      topColor: "#ff8a65",
      bottomColor: "#bf360c",
      title: qsTr("Notifications & Alarms"),
      description: qsTr("Multi-band dataset alarms with severity tiers, routed to the " +
                        "Notification Center."),
      helpId: "notifications"
    },
    {
      icon: "qrc:/icons/whats-new/lock.svg",
      topColor: "#a1887f",
      bottomColor: "#4e342e",
      title: qsTr("Project Lock"),
      description: qsTr("Lock a project to separate operator use from editing, with an " +
                        "access code."),
      helpId: "project-lock"
    },
    {
      icon: "qrc:/icons/whats-new/connectivity.svg",
      topColor: "#f0932b",
      bottomColor: "#a04000",
      title: qsTr("MQTT, Protobuf & File Transfer"),
      description: qsTr("MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / " +
                        "ZMODEM transfers."),
      helpId: "drivers-mqtt"
    }
  ]

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12

    //
    // Header groupbox
    //
    Rectangle {
      radius: 6
      border.width: 1
      Layout.fillWidth: true
      Layout.minimumWidth: 620
      Layout.maximumWidth: 620
      implicitHeight: headerRow.implicitHeight + 24
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      RowLayout {
        id: headerRow

        spacing: 16
        anchors.margins: 12
        anchors.fill: parent

        Image {
          sourceSize.width: 56
          sourceSize.height: 56
          source: "qrc:/logo/icon.svg"
          Layout.alignment: Qt.AlignVCenter
        }

        ColumnLayout {
          spacing: 2
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter

          Label {
            Layout.fillWidth: true
            wrapMode: Label.WordWrap
            text: qsTr("Welcome to %1!").arg(Cpp_AppName)
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
          }

          Label {
            Layout.fillWidth: true
            wrapMode: Label.WordWrap
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1.05, false)
            text: qsTr("Here's what's new in version %1.")
                    .arg(Cpp_Misc_WhatsNew.currentVersion)
          }
        }
      }
    }

    //
    // Scrollable, colorful feature card grid
    //
    Rectangle {
      radius: 6
      border.width: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 620
      Layout.maximumWidth: 620
      Layout.minimumHeight: 340
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      ScrollView {
        clip: true
        anchors.margins: 8
        anchors.fill: parent

        GridView {
          id: grid

          clip: true
          cellWidth: 298
          cellHeight: 116
          model: root.features
          anchors.leftMargin: 2
          boundsBehavior: Flickable.StopAtBounds

          delegate: Item {
            width: grid.cellWidth
            height: grid.cellHeight

            Rectangle {
              id: card

              clip: true
              radius: 6
              border.width: 1
              anchors.fill: parent
              anchors.margins: 4
              border.color: cardMouse.containsMouse
                            ? modelData.topColor
                            : Cpp_ThemeManager.colors["mid"]

              //
              // Soft per-feature gradient tint; brighter on hover
              //
              gradient: Gradient {
                GradientStop {
                  position: 0
                  color: Qt.tint(Cpp_ThemeManager.colors["base"],
                                 Qt.rgba(card.tintR, card.tintG, card.tintB,
                                         cardMouse.containsMouse ? 0.45 : 0.22))
                }
                GradientStop {
                  position: 1
                  color: Qt.tint(Cpp_ThemeManager.colors["base"],
                                 Qt.rgba(card.tintR, card.tintG, card.tintB,
                                         cardMouse.containsMouse ? 0.30 : 0.10))
                }
              }

              //
              // Pre-resolved tint channels from the feature's top color
              //
              readonly property real tintR: tintColor.r
              readonly property real tintG: tintColor.g
              readonly property real tintB: tintColor.b
              readonly property color tintColor: modelData.topColor

              Behavior on border.color {
                ColorAnimation { duration: 150 }
              }

              //
              // Colored left accent stripe
              //
              Rectangle {
                width: 5
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom

                gradient: Gradient {
                  GradientStop { position: 0; color: modelData.topColor }
                  GradientStop { position: 1; color: modelData.bottomColor }
                }
              }

              RowLayout {
                spacing: 12
                anchors.margins: 10
                anchors.fill: parent
                anchors.leftMargin: 14

                Image {
                  sourceSize.width: 44
                  sourceSize.height: 44
                  source: modelData.icon
                  Layout.alignment: Qt.AlignVCenter
                }

                //
                // Title + description
                //
                ColumnLayout {
                  spacing: 3
                  Layout.fillWidth: true
                  Layout.fillHeight: true
                  Layout.alignment: Qt.AlignVCenter

                  Label {
                    Layout.fillWidth: true
                    text: modelData.title
                    elide: Text.ElideRight
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                  }

                  Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Label.WordWrap
                    maximumLineCount: 3
                    text: modelData.description
                    verticalAlignment: Text.AlignTop
                    color: Cpp_ThemeManager.colors["text"]
                    font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
                  }
                }
              }

              MouseArea {
                id: cardMouse

                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.learnMore(modelData.helpId)
              }
            }
          }
        }
      }
    }

    //
    // Footer: startup toggle + close
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 4
      Layout.fillWidth: true

      Switch {
        id: startupSwitch

        Layout.leftMargin: -6
        checked: Cpp_Misc_WhatsNew.showWhatsNewOnStartup
        onToggled: Cpp_Misc_WhatsNew.showWhatsNewOnStartup = checked
      }

      Label {
        Layout.fillWidth: true
        wrapMode: Label.WordWrap
        text: qsTr("Show on Startup")
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.uiFont
      }

      Widgets.IconButton {
        highlighted: true
        text: qsTr("Close")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/apply.svg"
      }
    }
  }
}
