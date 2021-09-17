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

import "../Widgets" as Widgets

Widgets.Window {
    id: root
    gradient: true
    Layout.minimumHeight: height
    headerDoubleClickEnabled: false
    icon.source: "qrc:/icons/group.svg"
    title: qsTr("Group %1").arg(groupIndex + 1)
    backgroundColor: Cpp_ThemeManager.embeddedWindowBackground
    height: column.implicitHeight + headerHeight + 4 * app.spacing

    //
    // Custom properties
    //
    property int groupIndex
    property int totalGroupCount

    //
    // Main layout
    //
    ColumnLayout {
        id: column
        spacing: app.spacing
        anchors {
            left: parent.left
            right: parent.right
            margins: 2 * app.spacing
            verticalCenter: parent.verticalCenter
        }

        //
        // Group title
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Title")
            }

            ComboBox {
                id: widget
                Layout.minimumWidth: 180
                model: [
                    qsTr("Dataset widgets"),
                    qsTr("Accelerometer"),
                    qsTr("Gyroscope"),
                    qsTr("Map")
                ]
            }

            RoundButton {
                icon.width: 18
                icon.height: 18
                enabled: groupIndex > 0
                opacity: enabled ? 1 : 0.5
                icon.color: Cpp_ThemeManager.text
                icon.source: "qrc:/icons/up.svg"

                Behavior on opacity {NumberAnimation{}}
            }

            RoundButton {
                icon.width: 18
                icon.height: 18
                opacity: enabled ? 1 : 0.5
                icon.color: Cpp_ThemeManager.text
                icon.source: "qrc:/icons/down.svg"
                enabled: groupIndex < totalGroupCount - 1
                Behavior on opacity {NumberAnimation{}}
            }

            RoundButton {
                icon.width: 18
                icon.height: 18
                icon.color: Cpp_ThemeManager.text
                icon.source: "qrc:/icons/delete.svg"
            }
        }

        //
        // Datasets
        //
        Repeater {
            id: repeater
            model: 0
            delegate: JsonDatasetDelegate {
                datasetIndex: index
                Layout.fillWidth: true
            }
        }

        //
        // Add dataset button
        //
        Button {
            icon.width: 24
            icon.height: 24
            Layout.fillWidth: true
            text: qsTr("Add dataset")
            icon.source: "qrc:/icons/add.svg"
            icon.color: Cpp_ThemeManager.text
            visible: widget.currentIndex == 0
            onClicked: repeater.model = repeater.model + 1
        }
    }
}
