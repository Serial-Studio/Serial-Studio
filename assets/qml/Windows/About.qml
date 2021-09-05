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

import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../Config/Colors.js" as Colors

Window {
    id: root

    //
    // Custom properties
    //
    readonly property int year: new Date().getFullYear()

    //
    // Window options
    //
    title: qsTr("About")
    minimumWidth: column.implicitWidth + 4 * app.spacing
    maximumWidth: column.implicitWidth + 4 * app.spacing
    minimumHeight: column.implicitHeight + 4 * app.spacing
    maximumHeight: column.implicitHeight + 4 * app.spacing
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

    //
    // Use page item to set application palette
    //
    Page {
        anchors.margins: 0
        anchors.fill: parent
        palette.text: Colors.Foreground
        palette.window: Colors.Background
        palette.buttonText: Colors.Foreground
        palette.windowText: Colors.Foreground

        //
        // Window controls
        //
        ColumnLayout {
            id: column
            spacing: app.spacing
            anchors.centerIn: parent

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
                        text: Cpp_AppName
                        font.pixelSize: 28
                    }

                    Label {
                        opacity: 0.5
                        font.pixelSize: 16
                        text: qsTr("Version %1").arg(Cpp_AppVersion)
                    }
                }
            }

            Label {
                opacity: 0.8
                Layout.fillWidth: true
                Layout.maximumWidth: 288
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Copyright © 2020-%1 %2, released under the MIT License.").arg(root.year).arg(Cpp_AppOrganization)
            }

            Label {
                opacity: 0.8
                font.pixelSize: 12
                Layout.fillWidth: true
                Layout.maximumWidth: 288
                color: palette.highlightedText
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                text: qsTr("The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.")
            }

            Item {
                height: app.spacing
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("Website")
                onClicked: Qt.openUrlExternally("https://www.alex-spataru.com/serial-studio")
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("Make a donation")
                onClicked: app.showDonationsDialog()
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("Report bug")
                onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/issues")
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("Documentation")
                onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("Acknowledgements")
                onClicked: acknowledgements.show()
            }

            Item {
                height: app.spacing
            }

            Button {
                Layout.fillWidth: true
                onClicked: root.close()
                text: qsTr("Close")
            }

            Item {
                height: app.spacing
            }
        }
    }

    //
    // Acknowledgements dialog
    //
    Acknowledgements {
        id: acknowledgements
    }
}
