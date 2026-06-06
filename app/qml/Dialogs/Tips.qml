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

  title: qsTr("Did You Know?")

  //
  // Size hints: content-sized so the window stays compact
  //
  fixedSize: false
  minimumWidth: 540
  minimumHeight: 240
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Index of the tip currently shown
  //
  property int currentTip: 0

  //
  // Tips sourced from the help index. helpId maps to a help.json page.
  //
  property var tips: [
    {
      text: qsTr("Keep your firmware simple by sending raw data and letting Serial Studio " +
                 "parse it in JavaScript, Lua, or code-free Built-In templates."),
      helpId: "javascript-api"
    },
    {
      text: qsTr("Give each channel its own function to calibrate, filter, or convert " +
                 "units. Offload the math to Serial Studio and keep your firmware lean."),
      helpId: "dataset-transforms"
    },
    {
      text: qsTr("Need a value your device never sends? A virtual dataset computes its own " +
                 "channel, like power from voltage and current, plotted and logged as data."),
      helpId: "dataset-transforms"
    },
    {
      text: qsTr("Catch glitches like a bench scope. Time-axis plots have a sweep and " +
                 "trigger mode, and you can drag the trigger level right on the plot."),
      helpId: "plots"
    },
    {
      text: qsTr("Stop scrolling to find the right widget. Group them into your own " +
                 "workspaces and jump between them from the taskbar search."),
      helpId: "widget-reference"
    },
    {
      text: qsTr("Never lose a test run again. Record sessions to a local database, then " +
                 "browse, tag, and replay them whenever you need them."),
      helpId: "session-database"
    },
    {
      text: qsTr("Hand a polished report to your team in seconds. Export any session to " +
                 "HTML or PDF, complete with charts and min/max/mean stats."),
      helpId: "session-reports"
    },
    {
      text: qsTr("Close the loop without extra tooling. Output Controls let you send " +
                 "commands back to your device straight from the dashboard."),
      helpId: "output-controls"
    },
    {
      text: qsTr("Build a visualization nobody else has. The Painter widget runs your own " +
                 "script to draw fully custom graphics from incoming data."),
      helpId: "painter-widget"
    },
    {
      text: qsTr("One tool for every link. Serial Studio reads from UART, TCP/UDP, " +
                 "Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O."),
      helpId: "data-sources"
    },
    {
      text: qsTr("Skip the terminal dance. Send and receive files over your serial link " +
                 "with the built-in XMODEM, YMODEM, and ZMODEM protocols."),
      helpId: "file-transmission"
    },
    {
      text: qsTr("Already have a Modbus register map or a DBC file? Generate a ready-to-use " +
                 "project from it automatically instead of building one by hand."),
      helpId: "auto-generating-projects"
    },
    {
      text: qsTr("Describe what you want and let the AI Assistant build it. It can create " +
                 "and edit projects for you across eight model providers."),
      helpId: "ai-assistant"
    }
  ]

  //
  // Pick a random starting tip each time the dialog opens
  //
  onVisibleChanged: {
    if (visible)
      root.currentTip = Math.floor(Math.random() * root.tips.length)
  }

  //
  // Step to the next/previous tip with wraparound
  //
  function nextTip() {
    root.currentTip = (root.currentTip + 1) % root.tips.length
  }

  function previousTip() {
    root.currentTip = (root.currentTip + root.tips.length - 1) % root.tips.length
  }

  //
  // Open the matching help page anchor
  //
  function learnMore() {
    var helpId = root.tips[root.currentTip].helpId
    if (helpId.length > 0)
      Qt.openUrlExternally("https://serial-studio.com/help#" + helpId)
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 10

    //
    // Tip card: lightbulb + text in a tinted, bordered panel
    //
    Rectangle {
      radius: 8
      border.width: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 500
      Layout.minimumHeight: 150
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
      color: Qt.tint(Cpp_ThemeManager.colors["base"], Qt.rgba(0.96, 0.78, 0.20, 0.10))

      RowLayout {
        spacing: 16
        anchors.margins: 16
        anchors.fill: parent

        Image {
          sourceSize.width: 64
          sourceSize.height: 64
          source: "qrc:/images/tip.svg"
          Layout.alignment: Qt.AlignTop
        }

        ColumnLayout {
          spacing: 8
          Layout.fillWidth: true
          Layout.fillHeight: true

          Label {
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
            text: qsTr("Tip %1 of %2").arg(root.currentTip + 1).arg(root.tips.length)
          }

          Label {
            Layout.fillWidth: true
            wrapMode: Label.WordWrap
            verticalAlignment: Text.AlignTop
            text: root.tips[root.currentTip].text
            font: Cpp_Misc_CommonFonts.customUiFont(1.05, false)
            color: Cpp_ThemeManager.colors["text"]
          }

          Item { Layout.fillHeight: true }

          Widgets.IconButton {
            horizontalPadding: 8
            text: qsTr("Learn More")
            onClicked: root.learnMore()
            Layout.alignment: Qt.AlignLeft
            icon.source: "qrc:/icons/buttons/website.svg"
          }
        }
      }
    }

    //
    // Footer: startup toggle + navigation + close
    //
    RowLayout {
      id: buttons

      spacing: 8
      Layout.fillWidth: true

      Switch {
        id: startupSwitch

        Layout.leftMargin: -6
        checked: Cpp_Misc_WhatsNew.showTipsOnStartup
        onToggled: Cpp_Misc_WhatsNew.showTipsOnStartup = checked
      }

      Label {
        Layout.fillWidth: true
        wrapMode: Label.WordWrap
        text: qsTr("Show Tips on Startup")
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.uiFont
      }

      Widgets.IconButton {
        horizontalPadding: 8
        text: qsTr("Previous")
        onClicked: root.previousTip()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/backward.svg"
      }

      Widgets.IconButton {
        text: qsTr("Next")
        horizontalPadding: 8
        onClicked: root.nextTip()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/forward.svg"
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
