import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../Widgets" as Widgets

Rectangle {
    id: root
    height: header.implicitHeight + 4 * app.spacing

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
    // Main layout
    //
    GridLayout {
        id: header
        columns: 2
        rowSpacing: app.spacing
        columnSpacing: app.spacing * 2

        anchors {
            left: parent.left
            right: parent.right
            margins: app.spacing * 2
            verticalCenter: parent.verticalCenter
        }

        //
        // Project title
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            Widgets.Icon {
                color: Cpp_ThemeManager.brightText
                source: "qrc:/icons/registration.svg"
            }

            TextField {
                Layout.fillWidth: true
                Layout.minimumWidth: 320
                text: Cpp_JSON_Editor.title
                onTextChanged: Cpp_JSON_Editor.setTitle(text)
                placeholderText: qsTr("Project title (required)")

                palette {
                    base: "#fff"
                    text: "#000"
                    placeholderText: "#444"
                }
            }
        }

        //
        // Separator character
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            Widgets.Icon {
                color: Cpp_ThemeManager.brightText
                source: "qrc:/icons/separator.svg"
            }

            TextField {
                Layout.fillWidth: true
                Layout.minimumWidth: 420
                text: Cpp_JSON_Editor.separator
                onTextChanged: Cpp_JSON_Editor.setSeparator(text)
                placeholderText: qsTr("Data separator (default is ',')")

                palette {
                    base: "#fff"
                    text: "#000"
                    placeholderText: "#444"
                }
            }
        }

        //
        // Start sequence
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            Widgets.Icon {
                color: Cpp_ThemeManager.brightText
                source: "qrc:/icons/start-sequence.svg"
            }

            TextField {
                Layout.fillWidth: true
                Layout.minimumWidth: 256
                text: Cpp_JSON_Editor.frameStartSequence
                onTextChanged: Cpp_JSON_Editor.setFrameStartSequence(text)
                placeholderText: qsTr("Frame start sequence (default is '%1')").arg(Cpp_IO_Manager.startSequence)

                palette {
                    base: "#fff"
                    text: "#000"
                    placeholderText: "#444"
                }
            }
        }

        //
        // End sequence
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            Widgets.Icon {
                color: Cpp_ThemeManager.brightText
                source: "qrc:/icons/end-sequence.svg"
            }

            TextField {
                Layout.fillWidth: true
                Layout.minimumWidth: 256
                text: Cpp_JSON_Editor.frameEndSequence
                onTextChanged: Cpp_JSON_Editor.setFrameEndSequence(text)
                placeholderText: qsTr("Frame end sequence (default is '%1')").arg(Cpp_IO_Manager.finishSequence)

                palette {
                    base: "#fff"
                    text: "#000"
                    placeholderText: "#444"
                }
            }
        }
    }

    anchors {
        margins: 0
        top: parent.top
        left: parent.left
        right: parent.right
    }
}
