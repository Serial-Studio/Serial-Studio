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
            view.model = 0
            view.model = Cpp_JSON_Editor.groupCount
        }

        function onGroupOrderChanged() {
            view.model = 0
            view.model = Cpp_JSON_Editor.groupCount
        }
    }

    //
    // Function to scroll to the last group
    //
    function scrollToBottom() {
        scroll.position = 1
    }

    //
    // Spacer
    //
    Item {
        height: app.spacing
    }

    //
    // List view
    //
    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 320
        Layout.leftMargin: -2 * app.spacing
        Layout.rightMargin: -2 * app.spacing

        ListView {
            id: view
            anchors.fill: parent
            model: Cpp_JSON_Editor.groupCount
            anchors.bottomMargin: app.spacing

            ScrollBar.vertical: ScrollBar {
                id: scroll
                policy: ScrollBar.AsNeeded
            }

            delegate: Item {
                x: (parent.width - width) / 2
                height: group.height + app.spacing
                width: parent.width - 4 * app.spacing

                //
                // Window shadow
                //
                Widgets.Shadow {
                    source: group
                    anchors.fill: group
                }

                //
                // Group window
                //
                JsonGroupDelegate {
                    id: group
                    group: index
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
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
