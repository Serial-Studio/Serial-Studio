/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  width: minimumWidth
  height: minimumHeight
  title: qsTr("MDF4 Player")
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + 32
  maximumHeight: column.implicitHeight + 32
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint |
        Qt.WindowCloseButtonHint
  }

  Connections {
    target: Cpp_MDF4_Player
    function onOpenChanged() {
      if (Cpp_MDF4_Player.isOpen)
        root.showNormal()
      else
        root.hide()
    }
  }

  onVisibilityChanged: {
    if (!visible && Cpp_MDF4_Player.isOpen)
      Cpp_MDF4_Player.closeFile()
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  Page {
    anchors.fill: parent
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

    ColumnLayout {
      id: column
      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

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
          icon.source: "qrc:/rcc/icons/buttons/media-prev.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
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
          icon.source: "qrc:/rcc/icons/buttons/media-next.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          enabled: (Cpp_MDF4_Player.framePosition < Cpp_MDF4_Player.frameCount - 1) && !Cpp_MDF4_Player.isPlaying
        }
      }
    }
  }
}
