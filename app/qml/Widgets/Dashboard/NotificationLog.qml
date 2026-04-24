/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  implicitWidth: 420
  implicitHeight: 320

  //
  // Widget data inputs (unused — NotificationLog is global, not per-group)
  //
  property var color
  property var model
  property var windowRoot
  property string widgetId

  //
  // Blink state — armed for kBlinkMs whenever a non-Info event arrives
  //
  property bool blinking: false
  readonly property int kBlinkMs: 10000

  //
  // Level → icon path mapping. Icons live in app/rcc/icons/notifications/.
  //
  function iconForLevel(level) {
    switch (level) {
    case 2: return "qrc:/rcc/icons/notifications/critical.svg"
    case 1: return "qrc:/rcc/icons/notifications/warning.svg"
    default: return "qrc:/rcc/icons/notifications/info.svg"
    }
  }

  //
  // Level → display color. Critical + Warning share `alarm`; Info uses `link`.
  //
  function colorForLevel(level) {
    switch (level) {
    case 2: return Cpp_ThemeManager.colors["alarm"]
    case 1: return Cpp_ThemeManager.colors["alarm"]
    default: return Cpp_ThemeManager.colors["link"]
    }
  }

  //
  // Format ms-since-epoch as HH:MM:SS.mmm in local time.
  //
  function formatTime(ms) {
    const d = new Date(ms)
    const pad = (n, w) => String(n).padStart(w, "0")
    return pad(d.getHours(), 2) + ":" + pad(d.getMinutes(), 2) + ":"
        + pad(d.getSeconds(), 2) + "." + pad(d.getMilliseconds(), 3)
  }

  //
  // Seed the ListModel from the NotificationCenter ring buffer on load
  //
  Component.onCompleted: {
    const events = Cpp_Notifications.history("", 0)
    for (var i = events.length - 1; i >= 0; --i)
      notifications.append(events[i])

    Qt.callLater(listView.positionAtTail)
  }

  //
  // Timer for blink effect
  //
  Timer {
    id: blinkTimer

    repeat: false
    interval: root.kBlinkMs
    onTriggered: root.blinking = false
  }

  //
  // Notifications model
  //
  ListModel {
    id: notifications
  }

  //
  // Connections with the C++ NotificationCenter
  //
  Connections {
    target: Cpp_Notifications

    function onNotificationPosted(event) {
      // Track whether user was viewing the tail before we append
      const wasAtTail = listView.atYEnd || listView.count === 0

      notifications.append(event)

      // Drop oldest entry when the model exceeds the C++ ring cap
      while (notifications.count > Cpp_Notifications.maxHistory())
        notifications.remove(0)

      // Auto-scroll only if the user hadn't scrolled up
      if (wasAtTail)
        Qt.callLater(listView.positionAtTail)

      // Arm the border blink for Warning/Critical events
      if (event.level > 0) {
        root.blinking = true
        blinkTimer.restart()
      }
    }

    function onHistoryCleared() {
      notifications.clear()
      root.blinking = false
      blinkTimer.stop()
    }

    function onChannelCleared(channel) {
      for (var i = notifications.count - 1; i >= 0; --i) {
        if (notifications.get(i).channel === channel)
          notifications.remove(i)
      }
    }
  }

  //
  // Main layout
  //
  ColumnLayout {
    spacing: 6
    anchors.margins: 8
    anchors.fill: parent

    //
    // Header row: channel filter + unread count + clear button
    //
    RowLayout {
      spacing: 6
      Layout.fillWidth: true

      TextField {
        id: filterField

        selectByMouse: true
        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.uiFont
        placeholderText: qsTr("Filter by channel…")
      }

      Label {
        leftPadding: 8
        rightPadding: 8
        Layout.preferredHeight: 22
        font: Cpp_Misc_CommonFonts.boldUiFont
        visible: Cpp_Notifications.unreadCount > 0
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: Cpp_ThemeManager.colors["highlighted_text"]
        text: Cpp_Notifications.unreadCount > 99
              ? "99+"
              : Cpp_Notifications.unreadCount.toString()
        background: Rectangle {
          radius: 11
          color: Cpp_ThemeManager.colors["alarm"]
        }
      }

      ToolButton {
        ToolTip.visible: hovered
        icon.source: "qrc:/rcc/icons/buttons/clear.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        ToolTip.text: qsTr("Clear all notifications")
        onClicked: Cpp_Notifications.clearAll()
      }
    }

    //
    // List area
    //
    Rectangle {
      clip: true
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      color: Cpp_ThemeManager.colors["table_cell_bg"]
      border.color: Cpp_ThemeManager.colors["groupbox_hard_border"]

      ListView {
        id: listView

        clip: true
        spacing: 0
        anchors.margins: 1
        model: notifications
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        function positionAtTail() {
          if (count > 0)
            positionViewAtIndex(count - 1, ListView.End)
        }

        delegate: Rectangle {
          id: row

          required property int index
          required property var model

          readonly property bool filtered:
              filterField.text.length > 0
              && row.model.channel.indexOf(filterField.text) === -1

          width: listView.width
          visible: !filtered
          height: filtered ? 0 : rowContent.implicitHeight + 10
          color: (index % 2 === 0)
                 ? Cpp_ThemeManager.colors["table_cell_bg"]
                 : Cpp_ThemeManager.colors["alternate_base"]

          RowLayout {
            id: rowContent

            spacing: 8
            anchors.fill: parent
            anchors.topMargin: 5
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            anchors.bottomMargin: 5

            Image {
              Layout.preferredWidth: 18
              Layout.preferredHeight: 18
              sourceSize: Qt.size(18, 18)
              Layout.alignment: Qt.AlignTop
              source: root.iconForLevel(row.model.level)
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
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  color: root.colorForLevel(row.model.level)
                  text: row.model.title || qsTr("(no title)")
                }

                Label {
                  leftPadding: 6
                  rightPadding: 6
                  text: row.model.channel
                  visible: row.model.channel.length > 0
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
                  text: root.formatTime(row.model.timestamp)
                  color: Cpp_ThemeManager.colors["placeholder_text"]
                  font: Cpp_Misc_CommonFonts.customMonoFont(0.85, false)
                }
              }

              Label {
                Layout.fillWidth: true
                text: row.model.subtitle
                wrapMode: Text.WordWrap
                font: Cpp_Misc_CommonFonts.uiFont
                visible: row.model.subtitle.length > 0
                color: Cpp_ThemeManager.colors["text"]
              }
            }
          }
        }

        //
        // Empty-state placeholder — large icon, heading, hint text
        //
        ColumnLayout {
          spacing: 12
          anchors.centerIn: parent
          visible: listView.count === 0
          width: Math.min(parent.width - 48, 320)

          Image {
            opacity: 0.35
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            sourceSize: Qt.size(64, 64)
            Layout.alignment: Qt.AlignHCenter
            fillMode: Image.PreserveAspectFit
            source: "qrc:/rcc/icons/dashboard-large/notification-log.svg"
          }

          Label {
            Layout.fillWidth: true
            text: qsTr("No notifications yet")
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
            horizontalAlignment: Text.AlignHCenter
          }

          Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            color: Cpp_ThemeManager.colors["placeholder_text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
            text: qsTr("Dataset transforms and output widget scripts can post events "
                       + "here via notifyInfo / notifyWarning / notifyCritical.")
          }
        }
      }
    }
  }
}
