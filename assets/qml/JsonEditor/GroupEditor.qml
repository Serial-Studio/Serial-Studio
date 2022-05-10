/*
 * Copyright (c) 2020-2022 Alex Spataru <https://github.com/alex-spataru>
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

import "../Widgets" as Widgets

ColumnLayout {
    id: root
    spacing: 0

    //
    // Connections with JSON editor model
    //
    Connections {
        target: Cpp_JSON_Editor

        function onGroupCountChanged() {
            tabRepeater.model = 0
            stackRepeater.model = 0
            tabRepeater.model = Cpp_JSON_Editor.groupCount
            stackRepeater.model = Cpp_JSON_Editor.groupCount
        }

        function onGroupOrderChanged() {
            tabRepeater.model = 0
            stackRepeater.model = 0
            tabRepeater.model = Cpp_JSON_Editor.groupCount
            stackRepeater.model = Cpp_JSON_Editor.groupCount
        }
    }

    //
    // Function to scroll to the last group
    //
    function selectLastGroup() {
        tabBar.currentIndex = tabBar.count - 1
    }

    //
    // Spacer
    //
    Item {
        height: app.spacing
    }

    //
    // Tab widget
    //
    TabBar {
        id: tabBar
        Layout.fillWidth: true
        visible: tabRepeater.model > 0

        Repeater {
            id: tabRepeater
            delegate: TabButton {
                height: 24
                text: qsTr("Group %1 - %2").arg(index + 1).arg(Cpp_JSON_Editor.groupTitle(index))
            }
        }
    }

    //
    // StackView
    //
    StackLayout {
        id: swipe
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabBar.currentIndex
        Layout.topMargin: -parent.spacing - 1

        Repeater {
            id: stackRepeater

            delegate: Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true
                active: swipe.currentIndex == index

                sourceComponent: JsonGroupDelegate {
                    group: index
                }
            }
        }
    }

    //
    // Spacer
    //
    Item {
        height: app.spacing
    }
}
