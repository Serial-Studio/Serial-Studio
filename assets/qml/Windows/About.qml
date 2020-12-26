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
import QtQuick.Window 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

ApplicationWindow {
    id: window
    flags: Qt.Dialog
    title: qsTr("About")
    minimumWidth: 320
    maximumWidth: 320
    minimumHeight: column.implicitHeight * 1.25
    maximumHeight: column.implicitHeight * 1.25

    //
    // Theme options
    //
    palette.text: Qt.rgba(1, 1, 1, 1)
    palette.buttonText: Qt.rgba(1, 1, 1, 1)
    palette.windowText: Qt.rgba(1, 1, 1, 1)
    background: Rectangle {
        color: Qt.rgba(18/255, 25/255, 32/255, 1)
    }

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: app.spacing
        anchors.margins: 2 * app.spacing

        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            Image {
                width: 96
                height: 96
                source: "qrc:/images/icon.png"
                Layout.alignment: Qt.AlignVCenter
                sourceSize: Qt.size(width, height)
            }

            ColumnLayout {
                spacing: app.spacing
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                Label {
                    font.bold: true
                    text: CppAppName
                    font.pixelSize: 28
                }

                Label {
                    opacity: 0.5
                    font.pixelSize: 16
                    text: qsTr("Version %1").arg(CppAppVersion)
                }
            }
        }

        Label {
            opacity: 0.8
            Layout.fillWidth: true
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Copyright © 2020 %1, released under the MIT License.").arg(CppAppOrganization)
        }

        Label {
            opacity: 0.8
            font.pixelSize: 12
            Layout.fillWidth: true
            color: palette.highlightedText
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            text: qsTr("The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.")
        }

        Item {
            height: app.spacing
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Contact author")
            onClicked: Qt.openUrlExternally("mailto:alex_spataru@outlook.com")
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Report bug")
            onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/issues")
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Check for updates")
            onClicked: {
                CppUpdater.setNotifyOnFinish(CppAppUpdaterUrl, true)
                CppUpdater.checkForUpdates(CppAppUpdaterUrl)
            }
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Documentation")
            onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
        }

        Item {
            Layout.fillHeight: true
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Close")
            onClicked: window.close()
        }

        Item {
            height: app.spacing
        }
    }
}
