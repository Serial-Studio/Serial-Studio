import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../Widgets" as Widgets

Rectangle {
    id: root
    height: footer.implicitHeight + 4 * app.spacing

    //
    // Signals
    //
    signal closeWindow()
    signal scrollToBottom()

    //
    // Background & border
    //
    border.width: 1
    border.color: Cpp_ThemeManager.toolbarGradient1
    gradient: Gradient {
        GradientStop { position: 0; color: Cpp_ThemeManager.toolbarGradient1 }
        GradientStop { position: 1; color: Cpp_ThemeManager.toolbarGradient2 }
    }

    //
    // Dialog buttons
    //
    RowLayout {
        id: footer
        spacing: app.spacing

        anchors {
            left: parent.left
            right: parent.right
            margins: app.spacing * 2
            verticalCenter: parent.verticalCenter
        }

        Button {
            icon.width: 24
            icon.height: 24
            onClicked: root.closeWindow()
            text: qsTr("Close") + _btSpacer
            icon.source: "qrc:/icons/close.svg"
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            id: addGrp
            icon.width: 24
            icon.height: 24
            highlighted: true
            Layout.fillWidth: true
            text: qsTr("Add group")
            icon.source: "qrc:/icons/add.svg"
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
            onClicked: {
                Cpp_JSON_Editor.addGroup()
                root.scrollToBottom()
            }
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            icon.width: 24
            icon.height: 24
            icon.source: "qrc:/icons/open.svg"
            icon.color: Cpp_ThemeManager.brightText
            onClicked: Cpp_JSON_Editor.openJsonFile()
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
            text: qsTr("Open existing project...") + _btSpacer
        }

        Button {
            icon.width: 24
            icon.height: 24
            icon.source: "qrc:/icons/new.svg"
            icon.color: Cpp_ThemeManager.brightText
            onClicked: Cpp_JSON_Editor.newJsonFile()
            text: qsTr("Create new project") + _btSpacer
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
        }

        Button {
            icon.width: 24
            icon.height: 24
            opacity: enabled ? 1: 0.5
            enabled: Cpp_JSON_Editor.modified
            icon.source: "qrc:/icons/apply.svg"
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
            text: (Cpp_JSON_Editor.jsonFilePath.length > 0 ? qsTr("Apply") : qsTr("Save")) + _btSpacer

            onClicked: {
                if (Cpp_JSON_Editor.saveJsonFile())
                    root.closeWindow()
            }

            Behavior on opacity {NumberAnimation{}}
        }
    }
}
