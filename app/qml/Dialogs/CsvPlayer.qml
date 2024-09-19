/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Window options
  //
  width: minimumWidth
  height: minimumHeight
  title: qsTr("CSV Player")
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + 32
  maximumHeight: column.implicitHeight + 32
  x: (Screen.desktopAvailableWidth - width) / 2
  y: (Screen.desktopAvailableHeight - height) / 2
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint  |
        Qt.WindowCloseButtonHint
  }

  //
  // Close CSV file when window is closed
  //
  onVisibleChanged: {
    if (!visible && Cpp_CSV_Player.isOpen)
      Cpp_CSV_Player.closeFile()
  }

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
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.windowText: Cpp_ThemeManager.colors["text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    //
    // Window controls
    //
    ColumnLayout {
      id: column
      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

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
          if (value !== Cpp_CSV_Player.progress)
            Cpp_CSV_Player.setProgress(value)
        }
      }

      //
      // Spacer
      //
      Item {
        Layout.minimumHeight: 4
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
          icon.source: "qrc:/rcc/icons/buttons/media-prev.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          enabled: Cpp_CSV_Player.progress > 0 && !Cpp_CSV_Player.isPlaying
        }

        Button {
          icon.width: 32
          icon.height: 32
          onClicked: Cpp_CSV_Player.toggle()
          Layout.alignment: Qt.AlignVCenter
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: Cpp_CSV_Player.isPlaying ? "qrc:/rcc/icons/buttons/media-pause.svg" :
                                                  "qrc:/rcc/icons/buttons/media-play.svg"
        }

        Button {
          icon.width: 18
          icon.height: 18
          opacity: enabled ? 1 : 0.5
          Layout.alignment: Qt.AlignVCenter
          onClicked: Cpp_CSV_Player.nextFrame()
          icon.source: "qrc:/rcc/icons/buttons/media-next.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          enabled: Cpp_CSV_Player.progress < 1 && !Cpp_CSV_Player.isPlaying
        }
      }
    }
  }
}
