import QtQuick 2.12
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

ColumnLayout {
    id: root
    visible: count > 0
    spacing: app.spacing

    property int count: 0
    property var titles:[""]
    property string icon: ""
    property string title: ""

    signal checkedChanged(var index, var checked)

    RowLayout {
        spacing: app.spacing
        visible: root.count > 0

        Image {
            source: root.icon
            width: sourceSize.width
            height: sourceSize.height
            sourceSize: Qt.size(18, 18)
            opacity: hideAll.checked ? 0.5 : 1
            Behavior on opacity {NumberAnimation{}}

            ColorOverlay {
                source: parent
                color: palette.text
                anchors.fill: parent
            }
        }

        Label {
            font.bold: true
            text: root.title
            opacity: hideAll.checked ? 0.5 : 1
            Behavior on opacity {NumberAnimation{}}
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
