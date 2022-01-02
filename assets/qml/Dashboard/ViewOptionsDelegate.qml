import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import "../Widgets" as Widgets

ColumnLayout {
    id: root
    visible: count > 0
    spacing: app.spacing

    property int count: 0
    property var titles:[""]
    property string icon: ""
    property string title: ""

    signal checkedChanged(var index, var checked)

    Connections {
        target: Cpp_UI_Dashboard

        function onDataReset() {
            hideAll.checked = false
        }
    }

    RowLayout {
        spacing: app.spacing
        visible: root.count > 0

        Widgets.Icon {
            width: 18
            height: 18
            source: root.icon
            color: palette.text
            opacity: hideAll.checked ? 0.5 : 1
        }

        Label {
            font.bold: true
            text: root.title
            opacity: hideAll.checked ? 0.5 : 1
        }

        Item {
            Layout.fillWidth: true
        }

        RoundButton {
            id: hideAll
            width: 24
            height: 24
            flat: true
            checkable: true
            icon.color: palette.text
            Layout.rightMargin: -app.spacing
            icon.source: checked ? "qrc:/icons/show-all.svg" : "qrc:/icons/hide-all.svg"
            onCheckedChanged: {
                for (var i = 0; i < root.count; ++i)
                    root.checkedChanged(i, !checked)
            }
        }
    }

    Repeater {
        model: hideAll.checked ? 0 : root.count
        delegate: Switch {
            checked: true
            Layout.fillWidth: true
            text: root.titles[index]
            onCheckedChanged: root.checkedChanged(index, checked)
            palette.highlight: Cpp_ThemeManager.alternativeHighlight
        }
    }

    Item {
        height: app.spacing
        visible: !hideAll.checked && count > 0
    }
}
