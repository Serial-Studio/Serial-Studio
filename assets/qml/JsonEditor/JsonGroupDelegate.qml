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

import "../Widgets" as Widgets

Widgets.Window {
    id: root

    //
    // Window properties
    //
    gradient: true
    Layout.minimumHeight: height
    headerDoubleClickEnabled: false
    icon.source: "qrc:/icons/group.svg"
    backgroundColor: Cpp_ThemeManager.paneWindowBackground
    height: column.implicitHeight + headerHeight + 4 * app.spacing
    title: qsTr("Group %1 - %2").arg(group + 1).arg(Cpp_JSON_Editor.groupTitle(group))

    //
    // Delete group button
    //
    altButtonEnabled: true
    altButtonIcon.source: "qrc:/icons/delete-item.svg"
    onAltButtonClicked: Cpp_JSON_Editor.deleteGroup(group)

    //
    // Custom properties
    //
    property int group

    //
    // Connections with JSON editor
    //
    Connections {
        target: Cpp_JSON_Editor

        function onGroupChanged(id) {
            if (id === group) {
                repeater.model = 0
                repeater.model = Cpp_JSON_Editor.datasetCount(group)
            }
        }
    }

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
        // Notes rectangle
        //
        Rectangle {
            id: notes
            radius: 16
            Layout.fillWidth: true
            color: Cpp_ThemeManager.highlight
            Layout.minimumHeight: 32 + 2 * app.spacing
            visible: widget.currentIndex === 1 || widget.currentIndex === 2

            RowLayout {
                spacing: app.spacing
                anchors.centerIn: parent
                Layout.alignment: Qt.AlignHCenter
                visible: widget.currentIndex === 1

                Widgets.Icon {
                    width: 32
                    height: 32
                    color: palette.highlightedText
                    Layout.alignment: Qt.AlignHCenter
                    source: "qrc:/icons/accelerometer.svg"
                } Label {
                    font.pixelSize: 18
                    wrapMode: Label.WordWrap
                    color: palette.highlightedText
                    text: "<b>" + qsTr("Note:") + "</b> " + qsTr("The accelerometer widget expects values in m/s².")
                }
            }

            RowLayout {
                spacing: app.spacing
                anchors.centerIn: parent
                Layout.alignment: Qt.AlignHCenter
                visible: widget.currentIndex === 2

                Widgets.Icon {
                    width: 32
                    height: 32
                    source: "qrc:/icons/gyro.svg"
                    color: palette.highlightedText
                    Layout.alignment: Qt.AlignHCenter
                } Label {
                    font.pixelSize: 18
                    wrapMode: Label.WordWrap
                    color: palette.highlightedText
                    text: "<b>" + qsTr("Note:") + "</b> " + qsTr("The gyroscope widget expects values in degrees (0° to 360°).")
                }
            }
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
                text: Cpp_JSON_Editor.groupTitle(group)
                onTextChanged: {
                    Cpp_JSON_Editor.setGroupTitle(group, text)
                    root.title = qsTr("Group %1 - %2").arg(group + 1).arg(Cpp_JSON_Editor.groupTitle(group))
                }
            }

            ComboBox {
                id: widget
                Layout.minimumWidth: 180
                model: Cpp_JSON_Editor.availableGroupLevelWidgets()
                currentIndex: Cpp_JSON_Editor.groupWidgetIndex(group)
                onCurrentIndexChanged: {
                    var prevIndex = Cpp_JSON_Editor.groupWidgetIndex(group)
                    if (currentIndex !== prevIndex) {
                        if (!Cpp_JSON_Editor.setGroupWidget(group, currentIndex))
                            currentIndex = prevIndex
                    }
                }
            }

            RoundButton {
                icon.width: 18
                icon.height: 18
                enabled: group > 0
                opacity: enabled ? 1 : 0.5
                icon.source: "qrc:/icons/up.svg"
                icon.color: Cpp_ThemeManager.text
                onClicked: Cpp_JSON_Editor.moveGroupUp(group)
            }

            RoundButton {
                icon.width: 18
                icon.height: 18
                opacity: enabled ? 1 : 0.5
                icon.color: Cpp_ThemeManager.text
                icon.source: "qrc:/icons/down.svg"
                enabled: group < Cpp_JSON_Editor.groupCount - 1
                onClicked: Cpp_JSON_Editor.moveGroupDown(group)
            }
        }

        //
        // Datasets
        //
        GridLayout {
            Layout.fillWidth: true
            rowSpacing: app.spacing
            columnSpacing: app.spacing
            Layout.fillHeight: repeater.model > 0
            columns: Math.floor(column.width / 320)
            Layout.minimumHeight: (repeater.model / columns) * 320

            Repeater {
                id: repeater
                model: Cpp_JSON_Editor.datasetCount(group)
                delegate: Item {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 320
                    Layout.minimumHeight: 420 + 2 * app.spacing

                    JsonDatasetDelegate {
                        id: datasetDelegate
                        dataset: index
                        group: root.group
                        anchors.fill: parent
                        multiplotGroup: widget.currentIndex === 4
                        showGroupWidget: widget.currentIndex > 0 && widget.currentIndex !== 4
                    }
                }
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
            icon.color: Cpp_ThemeManager.menubarText
            onClicked: Cpp_JSON_Editor.addDataset(group)
            palette.buttonText: Cpp_ThemeManager.menubarText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
            visible: widget.currentIndex === 0 || widget.currentIndex === 4
        }
    }
}
