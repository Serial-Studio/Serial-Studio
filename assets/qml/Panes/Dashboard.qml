/*
 * Copyright (c) 2021 Alex Spataru <https://github.com/alex-spataru>
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

import "../Widgets" as Widgets
import "../Dashboard" as DashboardItems

Item {
    id: root

    //
    // Main layout
    //
    ColumnLayout {
        anchors.fill: parent
        spacing: app.spacing
        anchors.margins: (app.spacing * 1.5) - 5

        //
        // Widget selector + widgets
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true
            Layout.fillHeight: true

            //
            // View options window
            //
            DashboardItems.ViewOptions {
                Layout.fillHeight: true
                Layout.minimumWidth: 280
                onWidgetSizeChanged: widgetGrid.maxSize = maxSize
            }

            //
            // Widget grid + console
            //
            ColumnLayout {
                spacing: terminalView.enabled ? app.spacing : 0

                DashboardItems.WidgetGrid {
                    id: widgetGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 240
                }


                Item {
                    enabled: false
                    id: terminalView
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    Layout.maximumHeight: 280
                    Layout.minimumHeight: 280
                    visible: Layout.bottomMargin > -Layout.minimumHeight
                    Layout.bottomMargin: enabled ? 0 : -Layout.minimumHeight

                    Behavior on Layout.bottomMargin { NumberAnimation{} }

                    Widgets.Window {
                        id: terminal
                        gradient: true
                        anchors.fill: parent
                        title: qsTr("Console")
                        altButtonEnabled: true
                        headerDoubleClickEnabled: false
                        icon.source: "qrc:/icons/code.svg"
                        altButtonIcon.source: "qrc:/icons/scroll-down.svg"
                        backgroundColor: Cpp_ThemeManager.paneWindowBackground
                        onAltButtonClicked: {
                            terminalView.enabled = false
                            dbTitle.consoleChecked = false
                        }

                        Widgets.Terminal {
                            anchors.fill: parent
                            widgetEnabled: terminalView.enabled
                        }
                    }
                }
            }
        }

        //
        // Dashboard title window
        //
        DashboardItems.DashboardTitle {
            id: dbTitle
            height: 32
            Layout.fillWidth: true
            onConsoleCheckedChanged: {
                if (terminalView.enabled != consoleChecked)
                    terminalView.enabled = consoleChecked
            }
        }
    }
}
