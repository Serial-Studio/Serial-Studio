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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

import SerialStudio 1.0

Window {
    id: root

    spacing: -1
    showIcon: false
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    borderColor: Cpp_ThemeManager.datasetTextSecondary
    title: group != null ? group.title : ""

    property int groupId: 0
    property Group group: null

    Connections {
        target: Cpp_UI_Provider

        //*! Optimize this function
        function onUpdated() {
            if (root.enabled) {
                var g = Cpp_UI_Provider.getGroup(groupId)
                if (g !== null)
                    root.group = g
            }
        }
    }

    ScrollView {
        id: _sv
        clip: true
        contentWidth: -1
        anchors.fill: parent
        anchors.margins: app.spacing

        ScrollBar.vertical.z: 5
        enabled: ScrollBar.vertical.size < 1
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.visible: ScrollBar.vertical.size < 1

        ColumnLayout {
            x: 0
            spacing: app.spacing
            width: _sv.width - (_sv.ScrollBar.vertical.visible ? 10 : 0)

            Repeater {
                model: group != null  ? group.datasetCount : 0
                delegate: DataDelegate {
                    Layout.fillWidth: true

                    //*! Optimize this function
                    //   About 11% of UI thread time is spent here
                    dataset: group.datasets[index]
                }
            }
        }
    }
}
