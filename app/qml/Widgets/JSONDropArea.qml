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

import SerialStudio

DropArea {
  //
  // Show rectangle and set color based on file extension on drag enter
  //
  onEntered: (drag) => {
    // Get file name & set color of rectangle accordingly
    if (drag.urls.length > 0) {
      var path = drag.urls[0].toString()
      if (path.endsWith(".json") || path.endsWith(".csv") || path.endsWith(".ssproj")) {
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
    if (cleanPath.endsWith(".json") || path.endsWith(".ssproj")) {
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
        text: qsTr("Drop Projects and CSV files here")
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
