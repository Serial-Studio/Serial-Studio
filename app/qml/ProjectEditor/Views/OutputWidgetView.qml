/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_JSON_ProjectEditor.selectedIcon
  title: Cpp_JSON_ProjectEditor.selectedText

  Connections {
    target: actionIconPicker

    function onIconSelected(icon) {
      if (Cpp_JSON_ProjectEditor.currentView === ProjectEditor.OutputWidgetView
          && Cpp_JSON_ProjectEditor.outputWidgetType === SerialStudio.OutputButton) {
        Cpp_JSON_ProjectModel.setOutputWidgetIcon(icon)
      }
    }
  }

  Page {
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -10
    }

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      //
      // Pro notice
      //
      Widgets.ProNotice {
        Layout.margins: -1
        Layout.bottomMargin: 0
        Layout.fillWidth: true
        closeButtonEnabled: false
        titleText: qsTr("Output widgets require a Pro license.")
        activationFlag: Cpp_JSON_ProjectModel.containsCommercialFeatures
        subtitleText: qsTr("You can configure output widgets, but they only appear on the dashboard with a Pro license.")
      }

      //
      // Type toggle buttons + actions toolbar
      //
      Rectangle {
        id: header

        z: 2
        Layout.fillWidth: true
        height: headerLayout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        RowLayout {
          id: headerLayout

          spacing: 4

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          //
          // Widget type toggles (radio-style)
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Button")
            Layout.alignment: Qt.AlignVCenter
            checked: Cpp_JSON_ProjectEditor.outputWidgetType === SerialStudio.OutputButton
            onClicked: Cpp_JSON_ProjectModel.setOutputWidgetType(SerialStudio.OutputButton)
            ToolTip.text: qsTr("Send a command on click")
            icon.source: "qrc:/rcc/icons/project-editor/actions/output-button.svg"
          } Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Slider")
            Layout.alignment: Qt.AlignVCenter
            checked: Cpp_JSON_ProjectEditor.outputWidgetType === SerialStudio.OutputSlider
            onClicked: Cpp_JSON_ProjectModel.setOutputWidgetType(SerialStudio.OutputSlider)
            ToolTip.text: qsTr("Send scaled numeric values")
            icon.source: "qrc:/rcc/icons/project-editor/actions/output-slider.svg"
          } Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Toggle")
            Layout.alignment: Qt.AlignVCenter
            checked: Cpp_JSON_ProjectEditor.outputWidgetType === SerialStudio.OutputToggle
            onClicked: Cpp_JSON_ProjectModel.setOutputWidgetType(SerialStudio.OutputToggle)
            ToolTip.text: qsTr("Send on/off commands")
            icon.source: "qrc:/rcc/icons/project-editor/actions/output-toggle.svg"
          } Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Text Field")
            Layout.alignment: Qt.AlignVCenter
            checked: Cpp_JSON_ProjectEditor.outputWidgetType === SerialStudio.OutputTextField
            onClicked: Cpp_JSON_ProjectModel.setOutputWidgetType(SerialStudio.OutputTextField)
            ToolTip.text: qsTr("Type and send arbitrary commands")
            icon.source: "qrc:/rcc/icons/project-editor/actions/output-textfield.svg"
          } Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Knob")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            checked: Cpp_JSON_ProjectEditor.outputWidgetType === SerialStudio.OutputKnob
            onClicked: Cpp_JSON_ProjectModel.setOutputWidgetType(SerialStudio.OutputKnob)
            ToolTip.text: qsTr("Rotary input for setpoints")
            icon.source: "qrc:/rcc/icons/project-editor/actions/output-knob.svg"
          }

          //
          // Spacer
          //
          Item {
            Layout.fillWidth: true
          }

          //
          // Duplicate
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Duplicate")
            Layout.alignment: Qt.AlignVCenter
            onClicked: Cpp_JSON_ProjectModel.duplicateCurrentOutputWidget()
            ToolTip.text: qsTr("Duplicate this output widget")
            icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
          }

          //
          // Delete
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Delete")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Delete this output widget")
            onClicked: Cpp_JSON_ProjectModel.deleteCurrentOutputWidget()
            icon.source: "qrc:/rcc/icons/project-editor/actions/delete.svg"
          }
        }

        Rectangle {
          height: 1
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }
      }

      //
      // Properties table (minimum height, no scroll)
      //
      TableDelegate {
        id: delegate

        spacerVisible: false
        headerVisible: false
        Layout.fillWidth: true
        Layout.maximumHeight: implicitHeight
        parameterWidth: Math.min(delegate.width * 0.3, 200)

        Binding {
          target: delegate
          property: "modelPointer"
          value: Cpp_JSON_ProjectEditor.outputWidgetModel
        }
      }

      //
      // Transmit Function header
      //
      Rectangle {
        id: codeHeader

        z: 999
        implicitHeight: 30
        Layout.fillWidth: true

        gradient: Gradient {
          GradientStop {
            position: 0
            color: Cpp_ThemeManager.colors["table_bg_header_top"]
          }

          GradientStop {
            position: 1
            color: Cpp_ThemeManager.colors["table_bg_header_bottom"]
          }
        }

        Rectangle {
          height: 1
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["table_border_header"]
        }

        RowLayout {
          spacing: 4
          anchors.fill: parent
          anchors.leftMargin: 8
          anchors.rightMargin: 8

          Image {
            sourceSize: Qt.size(18, 18)
            Layout.alignment: Qt.AlignVCenter
            source: "qrc:/rcc/icons/project-editor/treeview/code.svg"
          }

          Label {
            text: qsTr("Transmit Function")
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["table_fg_header"]
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("Import")
            toolbarButton: false
            horizontalLayout: true
            onClicked: outputEditor.import()
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/code-editor/open.svg"
            ToolTip.text: qsTr("Import transmit function from a .js file")
          }

          Widgets.ToolbarButton {
            iconSize: 16
            toolbarButton: false
            horizontalLayout: true
            text: qsTr("Template")
            Layout.alignment: Qt.AlignVCenter
            onClicked: outputEditor.selectTemplate()
            icon.source: "qrc:/rcc/icons/code-editor/template.svg"
            ToolTip.text: qsTr("Select a pre-built transmit function template")
          }

          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("Test")
            toolbarButton: false
            horizontalLayout: true
            Layout.alignment: Qt.AlignVCenter
            onClicked: outputEditor.testTransmitFunction()
            icon.source: "qrc:/rcc/icons/code-editor/test.svg"
            ToolTip.text: qsTr("Test the transmit function with sample input")
          }
        }
      }

      //
      // Code editor (fills remaining height)
      //
      Item {
        Layout.topMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true

        OutputCodeEditor {
          id: outputEditor
          anchors.fill: parent
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.IBeamCursor
          propagateComposedEvents: true
          acceptedButtons: Qt.RightButton

          onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
              contextMenu.popup()
              mouse.accepted = true
            }
          }
        }
      }

      //
      // Context menu for code editor
      //
      Menu {
        id: contextMenu

        MenuItem {
          text: qsTr("Undo")
          enabled: outputEditor.undoAvailable
          onTriggered: outputEditor.undo()
        }

        MenuItem {
          text: qsTr("Redo")
          enabled: outputEditor.redoAvailable
          onTriggered: outputEditor.redo()
        }

        MenuSeparator {}

        MenuItem {
          text: qsTr("Cut")
          onTriggered: outputEditor.cut()
        }

        MenuItem {
          text: qsTr("Copy")
          onTriggered: outputEditor.copy()
        }

        MenuItem {
          text: qsTr("Paste")
          onTriggered: outputEditor.paste()
        }

        MenuSeparator {}

        MenuItem {
          text: qsTr("Select All")
          onTriggered: outputEditor.selectAll()
        }
      }
    }
  }
}
