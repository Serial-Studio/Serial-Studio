/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

DropArea {
  //
  // Show rectangle and set color based on file extension on drag enter
  //
  onEntered: (drag) => {
    // Get file name & set color of rectangle accordingly
    if (drag.urls.length > 0) {
      var path = drag.urls[0].toString()
      if (path.endsWith(".json") || path.endsWith(".csv")) {
        drag.accept(Qt.LinkAction)
        dropRectangle.color = Qt.darker(palette.highlight, 1.4)
      }

      // Invalid file name, show red rectangle
      else
        dropRectangle.color = Cpp_ThemeManager.colors["error"]

      // Show drag&drop rectangle
      dropRectangle.opacity = 0.8
    }
  }

  //
  // Open *.json & *.csv files on drag drop
  //
  onDropped: (drop) => {
    // Hide rectangle
    dropRectangle.hide()

    // Get dropped file URL and remove prefixed "file://"
    var path = drop.urls[0].toString()
    if (Qt.platform.os !== "windows")
      path = path.replace(/^(file:\/{2})/,"");
    else
      path = path.replace(/^(file:\/{3})/,"");

    // Unescape html codes like '%23' for '#'
    var cleanPath = decodeURIComponent(path);

    // Process JSON files
    if (cleanPath.endsWith(".json")) {
      Cpp_JSON_FrameBuilder.operationMode = SerialStudio.ProjectFile
      Cpp_JSON_FrameBuilder.loadJsonMap(cleanPath)
    }

    // Process CSV files
    else if (cleanPath.endsWith(".csv"))
      Cpp_CSV_Player.openFile(cleanPath)
  }

  //
  // Hide drag & drop rectangle on drag exit
  //
  onExited: {
    dropRectangle.hide()
  }

  //
  // Drop rectangle + icon + text
  //
  Rectangle {
    id: dropRectangle

    function hide() {
      rectTimer.start()
    }

    opacity: 0
    border.width: 1
    anchors.centerIn: parent
    width: dropLayout.implicitWidth + 48
    height: dropLayout.implicitHeight + 48
    color: Cpp_ThemeManager.colors["highlight"]
    border.color: Cpp_ThemeManager.colors["text"]

    ColumnLayout {
      spacing: 16
      id: dropLayout
      anchors.centerIn: parent

      Image {
        sourceSize: Qt.size(128, 128)
        Layout.alignment: Qt.AlignHCenter
        source: "qrc:/rcc/images/drag-and-drop.svg"
      }

      Label {
        Layout.alignment: Qt.AlignHCenter
        text: qsTr("Drop JSON and CSV files here")
        font: Cpp_Misc_CommonFonts.customUiFont(2, true)
        color: Cpp_ThemeManager.colors["highlighted_text"]
      }
    }

    Timer {
      id: rectTimer
      interval: 200
      repeat: false
      onTriggered: dropRectangle.opacity = 0
    }

    Behavior on opacity {NumberAnimation{}}
  }
}
