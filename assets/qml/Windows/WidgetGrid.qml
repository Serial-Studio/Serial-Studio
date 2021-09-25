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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import SerialStudio 1.0
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
    backgroundColor: Cpp_ThemeManager.embeddedWindowBackground

    // Hacks for calculating cell width
    readonly property int columns: Math.floor(grid.width / cWidth)
    readonly property int cWidth: Math.min(Math.max(356, grid.width / Math.min(3, grid.count)), 480)

    //
    // Put all items inside a grid view (the column layout in combination with the "clip" property
    // of the grid is used to inhibit the generated widgets from escaping the window)
    //
    ColumnLayout {
        spacing: 0

        anchors {
            fill: parent
            margins: 0
            leftMargin: app.spacing
            rightMargin: app.spacing
        }

        GridView {
            id: grid
            clip: true
            contentWidth: -1
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Calculate cell size so that cells fill the grid space and heigth < width
            cellHeight: cellWidth * (2/3)
            cellWidth: cWidth + (grid.width - cWidth * columns) / columns

            // Model + delegate
            model: Cpp_UI_Dashboard.totalWidgetCount
            delegate: Item {
                id: cell
                width: grid.cellWidth
                height: grid.cellHeight

                visible: opacity > 0
                Behavior on opacity {NumberAnimation{}}

                Widgets.Window {
                    id: window
                    title: loader.widgetTitle
                    icon.source: loader.widgetIcon
                    onHeaderDoubleClicked: loader.displayWindow()
                    borderColor: Cpp_ThemeManager.datasetWindowBorder

                    anchors {
                        fill: parent
                        margins: app.spacing
                        bottomMargin: app.spacing
                        topMargin: 2 * app.spacing
                    }

                    WidgetLoader {
                        id: loader
                        widgetIndex: index
                        anchors.fill: parent
                        anchors.margins: app.spacing
                        onWidgetVisibleChanged: cell.opacity = widgetVisible ? 1 : 0
                    }
                }

                Widgets.Shadow {
                    source: window
                }
            }
        }

        Item {
            Layout.fillWidth: true
            height: root.borderWidth
        }
    }
}
