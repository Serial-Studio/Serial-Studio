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
      onTriggered: dropRectangle.opacity = 0
    }

    Behavior on opacity {NumberAnimation{}}
  }
}
