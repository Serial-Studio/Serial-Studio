/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only
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
  title: qsTr("MDF4 Player")
  staysOnTop: true

  //
  // Automatically display the window when the MDF file is opened
  //
  Connections {
    target: Cpp_MDF4_Player
    function onOpenChanged() {
      if (Cpp_MDF4_Player.isOpen)
        root.showNormal()
      else
        root.hide()
    }
  }

  //
  // Automatically close MDF file when dialog is hidden
  //
  onVisibilityChanged: {
    if (!visible && Cpp_MDF4_Player.isOpen)
      Cpp_MDF4_Player.closeFile()
  }

  //
  // Window controls
  //
  contentItem: ColumnLayout {
    id: column
    spacing: 4
    anchors.centerIn: parent

      Label {
        Layout.alignment: Qt.AlignLeft
        text: Cpp_MDF4_Player.timestamp
        font: Cpp_Misc_CommonFonts.monoFont
      }

      Slider {
        Layout.fillWidth: true
        value: Cpp_MDF4_Player.progress
        onValueChanged: {
          if (!isNaN(value) && value !== Cpp_MDF4_Player.progress)
            Cpp_MDF4_Player.setProgress(value)
        }
      }

      Item {
        implicitHeight: 4
      }

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
          onClicked: Cpp_MDF4_Player.previousFrame()
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/buttons/media-prev.svg"
          enabled: Cpp_MDF4_Player.framePosition > 0 && !Cpp_MDF4_Player.isPlaying
        }

        Button {
          icon.width: 32
          icon.height: 32
          onClicked: Cpp_MDF4_Player.toggle()
          Layout.alignment: Qt.AlignVCenter
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: (Cpp_MDF4_Player.framePosition >= Cpp_MDF4_Player.frameCount - 1) ?
                         "qrc:/rcc/icons/buttons/media-stop.svg" :
                         (Cpp_MDF4_Player.isPlaying ? "qrc:/rcc/icons/buttons/media-pause.svg" :
                                                     "qrc:/rcc/icons/buttons/media-play.svg")
        }

        Button {
          icon.width: 18
          icon.height: 18
          opacity: enabled ? 1 : 0.5
          Layout.alignment: Qt.AlignVCenter
          onClicked: Cpp_MDF4_Player.nextFrame()
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/buttons/media-next.svg"
          enabled: (Cpp_MDF4_Player.framePosition < Cpp_MDF4_Player.frameCount - 1) && !Cpp_MDF4_Player.isPlaying
        }
      }
    }
}
