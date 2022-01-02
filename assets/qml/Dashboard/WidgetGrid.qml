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

Widgets.Window {
    id: root

    //
    // Window properties
    //
    gradient: true
    title: qsTr("Data")
    headerDoubleClickEnabled: false
    icon.source: "qrc:/icons/dataset.svg"
    backgroundColor: Cpp_ThemeManager.paneWindowBackground

    // Hacks for calculating cell width
    property int maxSize: 480
    readonly property int minSize: maxSize * 356/480
    readonly property int cellHeight: cellWidth * (2/3)
    readonly property int columns: Math.floor((grid.width - 2 * scroll.width) / cWidth)
    readonly property int cellWidth: cWidth + ((grid.width - 2 * scroll.width) - (cWidth) * columns) / columns
    readonly property int cWidth: Math.min(Math.max(minSize, (grid.width - 2 * scroll.width) / model.count), maxSize)

    //
    // Put everything into a flickable to enable scrolling
    //
    Item {
        clip: true
        anchors.fill: parent

        Flickable {
            contentWidth: width
            contentHeight: grid.height

            anchors {
                fill: parent
                margins: app.spacing * 2
                rightMargin: app.spacing
            }

            ScrollBar.vertical: ScrollBar {
                id: scroll
            }

            Grid {
                id: grid
                width: parent.width
                columns: root.columns
                rowSpacing: app.spacing
                columnSpacing: app.spacing
                height: childrenRect.height

                Timer {
                    id: timer
                    interval: 200
                    onTriggered: transition.enabled = false
                }

                Connections {
                    target: Cpp_UI_Dashboard
                    function onWidgetVisibilityChanged() {
                        transition.enabled = true
                        timer.start()
                    }
                }

                move: Transition {
                    id: transition
                    enabled: false

                    NumberAnimation {
                        duration: 200
                        properties: "x,y"
                        easing.type: Easing.OutSine
                    }
                }

                WidgetModel {
                    id: model
                    cellWidth: root.cellWidth
                    cellHeight: root.cellHeight
                    model: Cpp_UI_Dashboard.totalWidgetCount
                }
            }
        }
    }

    //
    // Redraw bottom window border over flickable items
    //
    Rectangle {
        height: root.borderWidth
        color: root.gradientColor1
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: root.radius
            rightMargin: root.radius
        }
    }
}
