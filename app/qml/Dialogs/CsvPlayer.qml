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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets"

SmartDialog {
  id: root

  //
  // Window options
  //
  title: qsTr("CSV Player")
  staysOnTop: true

  //
  // Automatically display the window when the CSV file is opened
  //
  Connections {
    target: Cpp_CSV_Player
    function onOpenChanged() {
      if (Cpp_CSV_Player.isOpen)
        root.showNormal()
      else
        root.hide()
    }
  }

  //
  // Automatically close CSV file when dialog is hidden
  //
  onVisibilityChanged: {
    if (!visible && Cpp_CSV_Player.isOpen)
      Cpp_CSV_Player.closeFile()
  }

  //
  // Window controls
  //
  contentItem: ColumnLayout {
    id: column
    spacing: 4
    anchors.centerIn: parent

      //
      // Timestamp display
      //
      Label {
        text: Cpp_CSV_Player.timestamp
        Layout.alignment: Qt.AlignLeft
        font: Cpp_Misc_CommonFonts.monoFont
      }

      //
      // Progress display
      //
      Slider {
        Layout.fillWidth: true
        value: Cpp_CSV_Player.progress
        onValueChanged: {
          if (!isNaN(value) && value !== Cpp_CSV_Player.progress)
            Cpp_CSV_Player.setProgress(value)
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Play/pause buttons
      //
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
          onClicked: Cpp_CSV_Player.previousFrame()
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/buttons/media-prev.svg"
          enabled: Cpp_CSV_Player.framePosition > 0 && !Cpp_CSV_Player.isPlaying
        }

        Button {
          icon.width: 32
          icon.height: 32
          onClicked: Cpp_CSV_Player.toggle()
          Layout.alignment: Qt.AlignVCenter
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: (Cpp_CSV_Player.framePosition >= Cpp_CSV_Player.frameCount - 1) ?
                         "qrc:/rcc/icons/buttons/media-stop.svg" :
                         (Cpp_CSV_Player.isPlaying ? "qrc:/rcc/icons/buttons/media-pause.svg" :
                                                     "qrc:/rcc/icons/buttons/media-play.svg")
        }

        Button {
          icon.width: 18
          icon.height: 18
          opacity: enabled ? 1 : 0.5
          Layout.alignment: Qt.AlignVCenter
          onClicked: Cpp_CSV_Player.nextFrame()
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/buttons/media-next.svg"
          enabled: (Cpp_CSV_Player.framePosition < Cpp_CSV_Player.frameCount - 1) && !Cpp_CSV_Player.isPlaying
        }
      }
    }
}
