/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

DropArea {
    //
    // Show rectangle and set color based on file extension on drag enter
    //
    onEntered: {
        // Get file name & set color of rectangle accordingly
        if (drag.urls.length > 0) {
            var path = drag.urls[0].toString()
            if (path.endsWith(".json") || path.endsWith(".csv")) {
                drag.accept(Qt.LinkAction)
                dropRectangle.color = Qt.darker(palette.highlight, 1.4)
            }

            // Invalid file name, show red rectangle
            else
                dropRectangle.color = Cpp_ThemeManager.alternativeHighlight

            // Show drag&drop rectangle
            dropRectangle.opacity = 0.8
        }
    }

    //
    // Open *.json & *.csv files on drag drop
    //
    onDropped: {
        // Hide rectangle
        dropRectangle.hide()

        // Get dropped file URL and remove prefixed "file://"
        var path = drop.urls[0].toString()
        if (!Cpp_IsWin)
            path = path.replace(/^(file:\/{2})/,"");
        else
            path = path.replace(/^(file:\/{3})/,"");

        // Unescape html codes like '%23' for '#'
        var cleanPath = decodeURIComponent(path);

        // Process JSON files
        if (cleanPath.endsWith(".json")) {
            Cpp_JSON_Generator.setOperationMode(0)
            Cpp_JSON_Generator.loadJsonMap(cleanPath)
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
        color: Cpp_ThemeManager.highlight
        border.color: Cpp_ThemeManager.text
        width: dropLayout.implicitWidth + 6 * app.spacing
        height: dropLayout.implicitHeight + 6 * app.spacing

        ColumnLayout {
            id: dropLayout
            spacing: app.spacing * 2
            anchors.centerIn: parent

            ToolButton {
                flat: true
                enabled: false
                icon.width: 128
                icon.height: 128
                Layout.alignment: Qt.AlignHCenter
                icon.source: "qrc:/icons/drag-drop.svg"
                icon.color: Cpp_ThemeManager.highlightedText
            }

            Label {
                font.bold: true
                font.pixelSize: 24
                Layout.alignment: Qt.AlignHCenter
                color: Cpp_ThemeManager.highlightedText
                text: qsTr("Drop JSON and CSV files here")
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
