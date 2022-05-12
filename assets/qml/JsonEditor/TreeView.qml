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

Widgets.Window {
    id: root

    //
    // Window properties
    //
    gradient: true
    headerDoubleClickEnabled: false
    title: qsTr("JSON Project Tree")
    icon.source: "qrc:/icons/json.svg"

    //
    // Connections with JSON editor model
    //
    Connections {
        target: Cpp_JSON_Editor

        function onGroupCountChanged() {
            view.model = 0
            view.model = Cpp_JSON_Editor.groupCount
        }

        function onGroupOrderChanged() {
            view.model = 0
            view.model = Cpp_JSON_Editor.groupCount
        }

        function onDatasetChanged() {
            view.model = 0
            view.model = Cpp_JSON_Editor.groupCount
        }
    }

    //
    // List view
    //
    ListView {
        id: view
        anchors.fill: parent
        spacing: app.spacing * 2
        anchors.margins: app.spacing
        model: Cpp_JSON_Editor.groupCount

        delegate: Loader {
            width: view.width - app.spacing * 2
            height: Cpp_JSON_Editor.datasetCount(index) * 24 + 24

            sourceComponent: ColumnLayout {
                id: groupDelegate
                spacing: app.spacing

                readonly property var groupId: index

                RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    Widgets.Icon {
                        width: 18
                        height: 18
                        color: Cpp_ThemeManager.text
                        source: "qrc:/icons/group.svg"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Label {
                        font.bold: true
                        Layout.fillWidth: true
                        elide: Label.ElideRight
                        Layout.alignment: Qt.AlignVCenter
                        text: Cpp_JSON_Editor.groupTitle(groupDelegate.groupId)
                    }

                    Label {
                        opacity: 0.5
                        visible: text !== "[]"
                        font.family: app.monoFont
                        Layout.alignment: Qt.AlignVCenter
                        text: "[" + Cpp_JSON_Editor.groupWidget(groupDelegate.groupId) + "]"
                    }
                }

                Repeater {
                    model: Cpp_JSON_Editor.datasetCount(groupDelegate.groupId)

                    delegate: Loader {
                        Layout.fillWidth: true
                        sourceComponent: RowLayout {
                            spacing: app.spacing

                            Item {
                                width: 2 * app.spacing
                            }

                            Widgets.Icon {
                                width: 18
                                height: 18
                                color: Cpp_ThemeManager.text
                                source: "qrc:/icons/dataset.svg"
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Label {
                                id: label
                                Layout.fillWidth: true
                                elide: Label.ElideRight
                                Layout.alignment: Qt.AlignVCenter
                                text: Cpp_JSON_Editor.datasetTitle(groupDelegate.groupId, index)

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }

                            Label {
                                opacity: 0.5
                                visible: text !== "[]"
                                font.family: app.monoFont
                                Layout.alignment: Qt.AlignVCenter
                                text: "[" + Cpp_JSON_Editor.datasetWidget(groupDelegate.groupId, index) + "]"
                            }
                        }
                    }
                }
            }
        }
    }
}
