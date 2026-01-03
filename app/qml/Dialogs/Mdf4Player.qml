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

  //
  // Custom properties
  //
  property int titlebarHeight: 0

  //
  // Window options
  //
  width: minimumWidth
  height: minimumHeight
  title: qsTr("MDF4 Player")
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + 32 + titlebarHeight
  maximumHeight: column.implicitHeight + 32 + titlebarHeight
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.CustomizeWindowHint |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint |
        Qt.WindowCloseButtonHint
  }

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
  // Also handle native window integration
  //
  onVisibilityChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    else {
      Cpp_NativeWindow.removeWindow(root)
      if (Cpp_MDF4_Player.isOpen)
        Cpp_MDF4_Player.closeFile()
    }

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  //
  // Update colors when theme changes
  //
  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Top section
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["window"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  //
  // Titlebar text
  //
  Label {
    text: root.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Be able to drag/move the window
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        root.startSystemMove()
    }
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  Page {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
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
