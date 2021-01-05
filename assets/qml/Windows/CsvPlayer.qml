/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

ApplicationWindow {
    id: root

    //
    // Window options
    //
    minimumWidth: 420
    maximumWidth: 420
    flags: Qt.Dialog | Qt.WindowStaysOnTopHint
    title: qsTr("CSV Player") + CppTranslator.dummy
    minimumHeight: column.implicitHeight + 4 * app.spacing
    maximumHeight: column.implicitHeight + 4 * app.spacing

    //
    // Theme options
    //
    palette.text: Qt.rgba(1, 1, 1, 1)
    palette.buttonText: Qt.rgba(1, 1, 1, 1)
    palette.windowText: Qt.rgba(1, 1, 1, 1)
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Window controls
    //
    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: app.spacing * 2
        anchors.margins: 2 * app.spacing

        //
        // Timestamp display
        //
        RowLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                font.bold: true
                text: qsTr("Timestamp") + ": "
                Layout.alignment: Qt.AlignVCenter
            }

            Label {
                text: qsTr("Timestamp")
                font.family: app.monoFont
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }
        }

        //
        // Progressbar display
        //
        ProgressBar {
            value: 0.5
            Layout.fillWidth: true
        }

        //
        // Play/pause buttons
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            Button {
                icon.color: "#fff"
                Layout.alignment: Qt.AlignVCenter
                icon.source: "qrc:/icons/media-prev.svg"
            }

            Button {
                icon.width: 32
                icon.height: 32
                icon.color: "#fff"
                Layout.alignment: Qt.AlignVCenter
                icon.source: "qrc:/icons/media-play.svg"
            }

            Button {
                icon.color: "#fff"
                Layout.alignment: Qt.AlignVCenter
                icon.source: "qrc:/icons/media-next.svg"
            }
        }
    }
}
