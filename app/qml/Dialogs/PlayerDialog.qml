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

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Player backend interface: subclasses bind the backend object plus the
  // optional busy/indexing/loading knobs their backend supports
  //
  required property var player
  property bool busy: false
  property bool indexing: false
  property real indexProgress: 0
  property bool loadingUi: false
  property string loadingText: ""

  //
  // Window options
  //
  staysOnTop: true

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: column.implicitWidth
  preferredHeight: column.implicitHeight

  //
  // Automatically display the window when the underlying file is opened
  //
  Connections {
    target: root.player
    function onOpenChanged() {
      if (root.player.isOpen && !root.visible)
        root.showNormal()
      else if (!root.player.isOpen && !root.busy)
        root.hide()
    }
  }

  //
  // Show the window while background work (decode/load) runs so its progress
  // is visible; hide it again when the work ends without the file opening
  //
  onBusyChanged: {
    if (busy && !visible)
      showNormal()
    else if (!busy && !player.isOpen && visible)
      hide()
  }

  //
  // Automatically close the file (or cancel in-flight work) when hidden
  //
  onVisibilityChanged: {
    if (!visible && (player.isOpen || busy))
      player.closeFile()
  }

  //
  // Window controls
  //
  dialogContent: ColumnLayout {
    id: column

    spacing: 4
    anchors.centerIn: parent

    //
    // Loading placeholder: shown while a worker thread loads the file,
    // swapped for the playback controls once isOpen flips true
    //
    ColumnLayout {
      spacing: 12
      Layout.fillWidth: true
      Layout.preferredWidth: 280
      Layout.alignment: Qt.AlignHCenter
      visible: root.loadingUi && root.busy && !root.player.isOpen

      BusyIndicator {
        running: parent.visible
        Layout.preferredWidth: 48
        Layout.preferredHeight: 48
        Layout.alignment: Qt.AlignHCenter
      }

      Label {
        text: root.loadingText
        Layout.alignment: Qt.AlignHCenter
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["text"]
      }
    }

    //
    // Timestamp display
    //
    Label {
      text: root.player.timestamp
      Layout.alignment: Qt.AlignLeft
      font: Cpp_Misc_CommonFonts.monoFont
      visible: !root.loadingUi || root.player.isOpen
    }

    //
    // Progress display
    //
    Slider {
      Layout.fillWidth: true
      value: root.player.progress
      visible: !root.loadingUi || root.player.isOpen
      onMoved: {
        if (!isNaN(value) && value !== root.player.progress)
          root.player.setProgress(value)
      }
    }

    //
    // Background indexing progress stripe
    //
    Item {
      clip: true
      visible: root.indexing
      Layout.fillWidth: true
      Layout.preferredHeight: 2

      Rectangle {
        opacity: 0.4
        anchors.fill: parent
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      Rectangle {
        height: parent.height
        radius: height / 2
        color: Cpp_ThemeManager.colors["highlight"]
        width: parent.width * Math.max(0, Math.min(1, root.indexProgress))
      }
    }

    //
    // Spacer
    //
    Item {
      implicitHeight: 4
      visible: !root.loadingUi || root.player.isOpen
    }

    //
    // Play/pause buttons
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter
      visible: !root.loadingUi || root.player.isOpen

      Widgets.IconButton {
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        onClicked: root.player.previousFrame()
        icon.source: "qrc:/icons/buttons/media-prev.svg"
        enabled: root.player.framePosition > 0 && !root.player.isPlaying
      }

      Widgets.IconButton {
        iconSize: 32
        onClicked: root.player.toggle()
        Layout.alignment: Qt.AlignVCenter
        icon.source: (root.player.framePosition >= root.player.frameCount - 1)
                     ? "qrc:/icons/buttons/media-stop.svg"
                     : (root.player.isPlaying
                        ? "qrc:/icons/buttons/media-pause.svg"
                        : "qrc:/icons/buttons/media-play.svg")
      }

      Widgets.IconButton {
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        onClicked: root.player.nextFrame()
        icon.source: "qrc:/icons/buttons/media-next.svg"
        enabled: (root.player.framePosition < root.player.frameCount - 1)
                 && !root.player.isPlaying
      }
    }
  }
}
