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
import QtQuick.Layouts
import QtQuick.Controls

import "../../../Widgets" as Widgets

//
// Master annunciator (spec 0087 R5): priority colour, unacknowledged count, fast/slow flash,
// steady under Reduce Motion; click acknowledges, the context menu holds the other actions.
//
Item {
  id: root

  signal navigateRequested(int windowId, int groupId)

  function formatTime(ms) {
    const d = new Date(ms)
    const pad = (n) => String(n).padStart(2, "0")
    return pad(d.getHours()) + ":" + pad(d.getMinutes()) + ":" + pad(d.getSeconds())
  }

  readonly property int priority: Cpp_UI_Alarms.testing ? Cpp_UI_Alarms.testPriority
                                                        : Cpp_UI_Alarms.highestPriority
  readonly property bool alarmsDisabled: Cpp_UI_Alarms.hasConfiguredAlarms
                                         && !Cpp_UI_Alarms.enabled
  readonly property bool fastFlash: Cpp_UI_Alarms.alerting && !Cpp_Misc_GraphicsBackend.reduceMotion
  readonly property bool slowFlash: !Cpp_UI_Alarms.alerting && Cpp_UI_Alarms.ringbackPending
                                    && !Cpp_Misc_GraphicsBackend.reduceMotion
  readonly property bool lit: !(fastFlash || slowFlash) || _flash.on
  readonly property color priorityColor: priority >= 2 ? Cpp_ThemeManager.colors["alarm"]
                                       : priority === 1 ? "#f0a020"
                                       : priority === 0 ? "#3daee9"
                                       : Cpp_ThemeManager.colors["taskbar_text"]

  Layout.preferredHeight: 24
  Layout.alignment: Qt.AlignVCenter
  Layout.preferredWidth: _bell.implicitWidth

  //
  // Flash clock: fast 450 ms half-period (matches the LED panels), slow 1000 ms (ringback)
  //
  QtObject {
    id: _flash

    property bool on: true
  }

  SequentialAnimation {
    loops: Animation.Infinite
    running: root.fastFlash || root.slowFlash
    onRunningChanged: if (!running) _flash.on = true

    PropertyAction { target: _flash; property: "on"; value: true }
    PauseAnimation { duration: root.fastFlash ? 450 : 1000 }
    PropertyAction { target: _flash; property: "on"; value: false }
    PauseAnimation { duration: root.fastFlash ? 450 : 1000 }
  }

  //
  // Bell button: the ICON carries the priority colour (and the flash), the background stays
  // the taskbar's own tray-button chrome. Left click opens the panel, right click acknowledges.
  //
  Widgets.IconButton {
    id: _bell

    padding: 4
    iconSize: 16
    anchors.centerIn: parent
    highlighted: _panel.opened
    text: Cpp_UI_Alarms.unacknowledgedCount > 0 ? String(Cpp_UI_Alarms.unacknowledgedCount) : ""
    font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
    color: root.lit ? root.priorityColor : Cpp_ThemeManager.colors["taskbar_text"]
    icon.color: root.lit ? root.priorityColor : Cpp_ThemeManager.colors["taskbar_text"]
    icon.source: Cpp_UI_Alarms.muted
                 ? "qrc:/icons/buttons/alarm-mute.svg"
                 : "qrc:/icons/buttons/alarm-annunciator.svg"

    Behavior on icon.color { ColorAnimation { duration: 120 } }

    background: Rectangle {
      radius: 3
      border.width: 1
      border.color: Cpp_ThemeManager.colors["taskbar_checked_button_border"]
      opacity: _bell.enabled && (_bell.highlighted || _bell.down) ? 1 : 0

      Behavior on opacity { NumberAnimation { duration: 100 } }

      gradient: Gradient {
        GradientStop {
          position: _bell.down ? 1 : 0
          color: Cpp_ThemeManager.colors["taskbar_checked_button_top"]
        }

        GradientStop {
          position: _bell.down ? 0 : 1
          color: Cpp_ThemeManager.colors["taskbar_checked_button_bottom"]
        }
      }
    }

    ToolTip.delay: 700
    ToolTip.visible: hovered && !_panel.opened
    ToolTip.text: root.alarmsDisabled
                  ? qsTr("Aural alerts are disabled, but this project defines alarms. Enable "
                         + "them in Preferences > Sounds.")
                  : Cpp_UI_Alarms.muted
                    ? qsTr("Alarm sounds are muted. Click to open the alarm panel.")
                    : Cpp_UI_Alarms.unacknowledgedCount > 0
                      ? qsTr("%1 unacknowledged alarm(s). Click for the alarm panel, right-click "
                             + "to acknowledge.").arg(Cpp_UI_Alarms.unacknowledgedCount)
                      : qsTr("No active alarms. Click for the alarm panel.")

    onClicked: _panel.opened ? _panel.close() : _panel.open()

    MouseArea {
      anchors.fill: parent
      acceptedButtons: Qt.RightButton
      onClicked: Cpp_UI_Alarms.acknowledge()
    }
  }

  //
  // Emissive bell (spec 0089): full intensity during the fast alarm flash, steady intensity
  // whenever a priority color is lit (ringback and acknowledged states included)
  //
  Widgets.HdrBoost {
    target: _bell
    anchors.fill: _bell
    active: root.lit && root.priority >= 0
    boost: root.fastFlash ? Cpp_Misc_GraphicsBackend.hdrAutoIntensity
                          : Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
  }

  //
  // Compact icon-only action button for the panel header
  //
  component PanelButton: Widgets.IconButton {
    padding: 3
    iconSize: 16
    Layout.minimumWidth: 28
    Layout.preferredWidth: 28
    Layout.preferredHeight: 26
  }

  //
  // Alarm panel: header with the operator actions, then a scrollable list of the active points,
  // most recently raised first; a row with a dashboard widget behind it jumps to that widget
  //
  Popup {
    id: _panel

    width: 380
    margins: 8
    padding: 8
    y: -height - 6
    x: _bell.width - width
    transformOrigin: Popup.BottomRight
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    enter: Widgets.PopupEnter {}
    exit: Widgets.PopupExit {}

    background: Rectangle {
      radius: 10
      border.width: 1
      color: Cpp_ThemeManager.colors["start_menu_background"]
      border.color: Cpp_ThemeManager.colors["start_menu_border"]
    }

    property var rows: []

    onAboutToShow: _panel.rows = Cpp_UI_Alarms.points

    Connections {
      target: Cpp_UI_Alarms
      function onStateChanged() {
        if (_panel.opened)
          _panel.rows = Cpp_UI_Alarms.points
      }
    }

    contentItem: ColumnLayout {
      spacing: 6

      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Label {
          text: qsTr("Alarms")
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["text"]
        }

        PanelButton {
          ToolTip.delay: 700
          ToolTip.visible: hovered
          ToolTip.text: qsTr("Acknowledge all")
          enabled: Cpp_UI_Alarms.unacknowledgedCount > 0
          icon.source: "qrc:/icons/buttons/alarm-acknowledge.svg"
          onClicked: Cpp_UI_Alarms.acknowledge()
        }

        PanelButton {
          ToolTip.delay: 700
          ToolTip.visible: hovered
          ToolTip.text: qsTr("Silence")
          enabled: Cpp_UI_Alarms.sounding
          icon.source: "qrc:/icons/buttons/alarm-silence.svg"
          onClicked: Cpp_UI_Alarms.silence()
        }

        PanelButton {
          ToolTip.delay: 700
          ToolTip.visible: hovered
          ToolTip.text: qsTr("Reset")
          enabled: Cpp_UI_Alarms.highestPriority >= 0
          icon.source: "qrc:/icons/buttons/refresh.svg"
          onClicked: Cpp_UI_Alarms.reset()
        }

        PanelButton {
          ToolTip.delay: 700
          ToolTip.visible: hovered
          enabled: Cpp_UI_Alarms.highestPriority >= 0
          icon.source: "qrc:/icons/buttons/clear.svg"
          ToolTip.text: qsTr("Clear every alarm from the list")
          onClicked: Cpp_UI_Alarms.clear()
        }

        PanelButton {
          ToolTip.delay: 700
          ToolTip.visible: hovered
          enabled: !Cpp_UI_Alarms.testing
          ToolTip.text: qsTr("Test sounds")
          icon.source: "qrc:/icons/buttons/alarm-test.svg"
          onClicked: Cpp_UI_Alarms.test()
        }

        PanelButton {
          ToolTip.delay: 700
          ToolTip.visible: hovered
          highlighted: Cpp_UI_Alarms.muted
          ToolTip.text: Cpp_UI_Alarms.muted ? qsTr("Unmute sounds") : qsTr("Mute sounds")
          icon.source: Cpp_UI_Alarms.muted
                       ? "qrc:/icons/buttons/alarm-mute.svg"
                       : "qrc:/icons/buttons/alarm-sound.svg"
          onClicked: Cpp_UI_Alarms.muted = !Cpp_UI_Alarms.muted
        }
      }

      Rectangle {
        clip: true
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["table_cell_bg"]
        border.color: Cpp_ThemeManager.colors["groupbox_hard_border"]
        Layout.preferredHeight: Math.min(320, Math.max(64, _list.contentHeight + 2))

        ListView {
          id: _list

          clip: true
          spacing: 0
          anchors.margins: 1
          model: _panel.rows
          anchors.fill: parent
          boundsBehavior: Flickable.StopAtBounds

          ScrollBar.vertical: ScrollBar {
            policy: _list.contentHeight > _list.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
          }

          delegate: Rectangle {
            id: row

            required property int index
            required property var modelData

            readonly property bool navigable: modelData.widgetIndex >= 0
            readonly property color tone: modelData.priority >= 2 ? Cpp_ThemeManager.colors["alarm"]
                                        : modelData.priority === 1 ? "#f0a020" : "#3daee9"

            width: _list.width
            height: rowContent.implicitHeight + 10
            color: rowArea.containsMouse && navigable
                   ? Cpp_ThemeManager.colors["highlight"]
                   : (index % 2 === 0) ? Cpp_ThemeManager.colors["table_cell_bg"]
                                       : Cpp_ThemeManager.colors["alternate_base"]

            MouseArea {
              id: rowArea

              hoverEnabled: true
              anchors.fill: parent
              cursorShape: row.navigable ? Qt.PointingHandCursor : Qt.ArrowCursor
              onClicked: {
                if (!row.navigable)
                  return

                _panel.close()
                root.navigateRequested(row.modelData.widgetIndex, row.modelData.groupId)
              }
            }

            RowLayout {
              id: rowContent

              spacing: 8
              anchors.fill: parent
              anchors.topMargin: 5
              anchors.leftMargin: 8
              anchors.rightMargin: 8
              anchors.bottomMargin: 5

              Image {
                sourceSize: Qt.size(18, 18)
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
                Layout.alignment: Qt.AlignTop
                source: row.modelData.priority >= 2
                        ? Cpp_Misc_IconRegistry.icon("notifications", "critical", 16)
                        : row.modelData.priority === 1
                          ? Cpp_Misc_IconRegistry.icon("notifications", "warning", 16)
                          : Cpp_Misc_IconRegistry.icon("notifications", "info", 16)
              }

              ColumnLayout {
                spacing: 2
                Layout.fillWidth: true

                RowLayout {
                  spacing: 6
                  Layout.fillWidth: true

                  Label {
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    Layout.minimumWidth: 90
                    text: row.modelData.title
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: rowArea.containsMouse && row.navigable
                           ? Cpp_ThemeManager.colors["highlighted_text"] : row.tone
                  }

                  Label {
                    leftPadding: 6
                    rightPadding: 6
                    elide: Text.ElideRight
                    Layout.maximumWidth: 130
                    text: row.modelData.kind === "band" ? row.modelData.label : row.modelData.channel
                    visible: text.length > 0
                    color: Cpp_ThemeManager.colors["placeholder_text"]
                    font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                    background: Rectangle {
                      radius: 3
                      border.width: 1
                      color: "transparent"
                      border.color: Cpp_ThemeManager.colors["groupbox_border"]
                    }
                  }

                  Label {
                    text: root.formatTime(row.modelData.sinceMs)
                    color: Cpp_ThemeManager.colors["placeholder_text"]
                    font: Cpp_Misc_CommonFonts.customMonoFont(0.85, false)
                  }
                }

                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                  color: rowArea.containsMouse && row.navigable
                         ? Cpp_ThemeManager.colors["highlighted_text"]
                         : Cpp_ThemeManager.colors["text"]
                  text: {
                    const state = row.modelData.state === "alert" ? qsTr("Unacknowledged")
                                : row.modelData.state === "acknowledged" ? qsTr("Acknowledged")
                                : qsTr("Returned to normal, awaiting reset")
                    return row.navigable ? state + " - " + qsTr("click to show the widget") : state
                  }
                }
              }
            }
          }
        }

        Label {
          anchors.centerIn: parent
          visible: _list.count === 0
          text: qsTr("No active alarms")
          color: Cpp_ThemeManager.colors["placeholder_text"]
        }
      }
    }
  }
}
