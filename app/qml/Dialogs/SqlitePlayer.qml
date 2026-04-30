/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets"

SmartDialog {
  id: root

  staysOnTop: true
  title: qsTr("Session Player")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Auto show/hide as the database is opened/closed (or while it's loading,
  // so the user gets visible feedback while the worker thread builds the
  // timestamp index of a large recording)
  //
  Connections {
    target: Cpp_Sessions_Player
    function onOpenChanged() {
      if (Cpp_Sessions_Player.isOpen)
        root.showNormal()
      else if (!Cpp_Sessions_Player.loading)
        root.hide()
    }

    function onLoadingChanged() {
      if (Cpp_Sessions_Player.loading)
        root.showNormal()
      else if (!Cpp_Sessions_Player.isOpen)
        root.hide()
    }
  }

  //
  // Close the file when the dialog is dismissed (also aborts an in-flight
  // load so the worker thread doesn't keep building a timestamp index for
  // a session the user already abandoned)
  //
  onVisibilityChanged: {
    if (!visible && (Cpp_Sessions_Player.isOpen || Cpp_Sessions_Player.loading))
      Cpp_Sessions_Player.closeFile()
  }

  //
  // Window controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 4
    anchors.centerIn: parent

    //
    // Loading placeholder — shown while the worker thread is loading the
    // session, hidden once isOpen flips true
    //
    ColumnLayout {
      spacing: 12
      Layout.fillWidth: true
      Layout.preferredWidth: 280
      Layout.alignment: Qt.AlignHCenter
      visible: Cpp_Sessions_Player.loading && !Cpp_Sessions_Player.isOpen

      BusyIndicator {
        running: parent.visible
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: 48
        Layout.preferredHeight: 48
      }

      Label {
        Layout.alignment: Qt.AlignHCenter
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.uiFont
        text: qsTr("Loading session…")
      }
    }

    //
    // Playback controls — visible only once the worker has finished loading
    //
    Label {
      Layout.alignment: Qt.AlignLeft
      visible: Cpp_Sessions_Player.isOpen
      text: Cpp_Sessions_Player.timestamp
      font: Cpp_Misc_CommonFonts.monoFont
    }

    Slider {
      Layout.fillWidth: true
      visible: Cpp_Sessions_Player.isOpen
      value: Cpp_Sessions_Player.progress
      onValueChanged: {
        if (!isNaN(value) && value !== Cpp_Sessions_Player.progress)
          Cpp_Sessions_Player.setProgress(value)
      }
    }

    Item { implicitHeight: 4; visible: Cpp_Sessions_Player.isOpen }

    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter
      visible: Cpp_Sessions_Player.isOpen

      Button {
        icon.width: 18
        icon.height: 18
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_Sessions_Player.previousFrame()
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/icons/buttons/media-prev.svg"
        enabled: Cpp_Sessions_Player.framePosition > 0 && !Cpp_Sessions_Player.isPlaying
      }

      Button {
        icon.width: 32
        icon.height: 32
        onClicked: Cpp_Sessions_Player.toggle()
        Layout.alignment: Qt.AlignVCenter
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: (Cpp_Sessions_Player.framePosition >= Cpp_Sessions_Player.frameCount - 1)
                     ? "qrc:/icons/buttons/media-stop.svg"
                     : (Cpp_Sessions_Player.isPlaying
                        ? "qrc:/icons/buttons/media-pause.svg"
                        : "qrc:/icons/buttons/media-play.svg")
      }

      Button {
        icon.width: 18
        icon.height: 18
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_Sessions_Player.nextFrame()
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/icons/buttons/media-next.svg"
        enabled: (Cpp_Sessions_Player.framePosition < Cpp_Sessions_Player.frameCount - 1)
                 && !Cpp_Sessions_Player.isPlaying
      }
    }
  }
}
