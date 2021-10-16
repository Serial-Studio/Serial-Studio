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

        delegate: ColumnLayout {
            id: groupDelegate
            readonly property var groupId: index

            spacing: app.spacing
            height: Cpp_JSON_Editor.datasetCount(groupDelegate.groupId) * 24 + 24

            anchors {
                left: parent.left
                right: parent.right
            }

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

                delegate: RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

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
                        Layout.fillWidth: true
                        elide: Label.ElideRight
                        Layout.alignment: Qt.AlignVCenter
                        text: Cpp_JSON_Editor.datasetTitle(groupDelegate.groupId, index)
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
