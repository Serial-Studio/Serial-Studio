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
  // Auto show/hide as the database is opened/closed
  //
  Connections {
    target: Cpp_Sessions_Player
    function onOpenChanged() {
      if (Cpp_Sessions_Player.isOpen)
        root.showNormal()
      else
        root.hide()
    }
  }

  //
  // Close the file when the dialog is dismissed
  //
  onVisibilityChanged: {
    if (!visible && Cpp_Sessions_Player.isOpen)
      Cpp_Sessions_Player.closeFile()
  }

  //
  // Window controls
  //
  dialogContent: ColumnLayout {
    spacing: 4
    anchors.centerIn: parent

    Label {
      Layout.alignment: Qt.AlignLeft
      text: Cpp_Sessions_Player.timestamp
      font: Cpp_Misc_CommonFonts.monoFont
    }

    Slider {
      Layout.fillWidth: true
      value: Cpp_Sessions_Player.progress
      onValueChanged: {
        if (!isNaN(value) && value !== Cpp_Sessions_Player.progress)
          Cpp_Sessions_Player.setProgress(value)
      }
    }

    Item { implicitHeight: 4 }

    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter

      Button {
        icon.width: 18
        icon.height: 18
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_Sessions_Player.previousFrame()
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/media-prev.svg"
        enabled: Cpp_Sessions_Player.framePosition > 0 && !Cpp_Sessions_Player.isPlaying
      }

      Button {
        icon.width: 32
        icon.height: 32
        onClicked: Cpp_Sessions_Player.toggle()
        Layout.alignment: Qt.AlignVCenter
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: (Cpp_Sessions_Player.framePosition >= Cpp_Sessions_Player.frameCount - 1)
                     ? "qrc:/rcc/icons/buttons/media-stop.svg"
                     : (Cpp_Sessions_Player.isPlaying
                        ? "qrc:/rcc/icons/buttons/media-pause.svg"
                        : "qrc:/rcc/icons/buttons/media-play.svg")
      }

      Button {
        icon.width: 18
        icon.height: 18
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_Sessions_Player.nextFrame()
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/media-next.svg"
        enabled: (Cpp_Sessions_Player.framePosition < Cpp_Sessions_Player.frameCount - 1)
                 && !Cpp_Sessions_Player.isPlaying
      }
    }
  }
}
