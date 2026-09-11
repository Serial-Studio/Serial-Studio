/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "."
import "../Views" as Views
import "../../Widgets" as Widgets
import "../../Widgets/Dashboard/Output" as OutputControls

Window {
  id: root

  //
  // Same accent rule as the dashboard panel: the widget's own color, else the dataset accent,
  // so the preview shows the control the way it will look on the dashboard.
  //
  readonly property string widgetColor: preview.widget ? (preview.widget.color || "") : ""
  readonly property color accentColor: root.widgetColor.length > 0
                                       ? root.widgetColor
                                       : SerialStudioHelpers.getDatasetAccentColor()

  Widgets.WindowMirror {}

  width: 1280
  height: 760
  minimumWidth: 900
  minimumHeight: 520
  flags: Qt.Dialog
  modality: Qt.ApplicationModal
  title: qsTr("Transmit Function Editor")
  color: Cpp_ThemeManager.colors["window"]

  //
  // Set while a template is being loaded, so the text change it causes is not mistaken for the
  // user typing and does not reset the combo back to its placeholder.
  //
  property bool applyingTemplate: false

  //
  // Raised only when the window really went away, so the Loader that owns it does not tear it
  // down while onClosing is still refusing the close.
  //
  signal dismissed()

  function showDialog() {
    editor.bindPreview(preview)

    root.applyingTemplate = true
    templateCombo.currentIndex = -1
    root.applyingTemplate = false

    show()
    raise()
    requestActivate()

    editor.readCode()
  }

  onClosing: root.dismissed()

  Shortcut {
    sequences: ["Ctrl+I"]
    onActivated: editor.formatSelection()
  } Shortcut {
    sequences: ["Ctrl+Shift+I"]
    onActivated: editor.formatDocument()
  }

  ColumnLayout {
    spacing: 0
    anchors.fill: parent

    RowLayout {
      spacing: 0
      Layout.fillWidth: true
      Layout.fillHeight: true

      //
      // Code
      //
      ColumnLayout {
        spacing: 4
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 420

        Item {
          Layout.margins: 8
          Layout.rightMargin: 0
          Layout.fillWidth: true
          Layout.fillHeight: true

          OutputCodeEditor {
            id: editor

            anchors.fill: parent

            onScriptAccepted: (code) => preview.applyScript(code)

            onScriptValidChanged: {
              if (!editor.scriptValid)
                preview.setPayloadStale(true)
            }

            onTextChanged: {
              if (!root.applyingTemplate)
                templateCombo.currentIndex = -1
            }
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
      }

      //
      // Live preview: the script's actions and its output, beside the control they belong to.
      //
      ColumnLayout {
        spacing: 8
        Layout.margins: 8
        Layout.fillHeight: true
        Layout.minimumWidth: 340
        Layout.preferredWidth: 400

        RowLayout {
          spacing: 4
          Layout.fillWidth: true
          Layout.fillHeight: false

          ComboBox {
            id: templateCombo

            Layout.fillWidth: true
            model: editor.templateNames
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
            displayText: currentIndex < 0 ? qsTr("Select Template") : currentText

            onActivated: (index) => {
              root.applyingTemplate = true
              editor.applyTemplate(index)
              root.applyingTemplate = false
            }
          }

          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("Import")
            toolbarButton: false
            horizontalLayout: true
            onClicked: editor.importFile()
            Layout.alignment: Qt.AlignVCenter
            icon.source: Cpp_Misc_IconRegistry.icon("code", "open", 16)
            ToolTip.text: qsTr("Import a transmit function from a .js file")
          }

          Widgets.ToolbarButton {
            iconSize: 16
            toolbarButton: false
            text: qsTr("Validate")
            horizontalLayout: true
            onClicked: editor.reportVerdict()
            Layout.alignment: Qt.AlignVCenter
            icon.source: Cpp_Misc_IconRegistry.icon("code", "reload", 16)
            ToolTip.text: qsTr("Verify that the script compiles and defines transmit(value)")
          }
        }

        Rectangle {
          Layout.fillWidth: true
          Layout.preferredHeight: 148
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          OutputWidgetPreview {
            id: preview

            anchors.fill: parent
          }

          Loader {
            anchors.margins: 8
            anchors.fill: parent
            active: preview.model !== null

            //
            // Every type named explicitly, null for none: falling back to the slider built one
            // against whatever model was current, and a Button carries no currentValue.
            //
            sourceComponent: {
              const type = preview.widget.type
              if (type === SerialStudio.OutputButton)    return buttonControl
              if (type === SerialStudio.OutputSlider)    return sliderControl
              if (type === SerialStudio.OutputToggle)    return toggleControl
              if (type === SerialStudio.OutputTextField) return textFieldControl
              if (type === SerialStudio.OutputKnob)      return knobControl
              return null
            }
          }
        }

        Label {
          text: qsTr("Produced Bytes")
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["text"]
        }

        Rectangle {
          Layout.fillWidth: true
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["console_base"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          ColumnLayout {
            spacing: 6
            anchors.margins: 8
            anchors.fill: parent

            Label {
              Layout.fillWidth: true
              wrapMode: Text.WrapAnywhere
              font: Cpp_Misc_CommonFonts.customMonoFont(0.9, false)
              color: Cpp_ThemeManager.colors["console_text"]
              text: preview.hasPayload ? preview.payloadHex
                                       : qsTr("Interact with the control to see its output")
            }

            Rectangle {
              implicitHeight: 1
              Layout.fillWidth: true
              visible: preview.hasPayload
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            Label {
              Layout.fillWidth: true
              visible: preview.hasPayload
              wrapMode: Text.WrapAnywhere
              font: Cpp_Misc_CommonFonts.customMonoFont(0.9, false)
              color: Cpp_ThemeManager.colors["console_text"]
              text: preview.payloadAscii
            }

            Item { Layout.fillHeight: true }

            Label {
              Layout.fillWidth: true
              visible: preview.hasPayload
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
              color: preview.payloadStale ? Cpp_ThemeManager.colors["alarm"]
                                          : Cpp_ThemeManager.colors["placeholder_text"]
              text: preview.payloadStale
                    ? qsTr("%1 bytes, from the last version that compiled").arg(preview.payloadSize)
                    : qsTr("%1 bytes").arg(preview.payloadSize)
            }
          }
        }
      }
    }

    Rectangle {
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    RowLayout {
      spacing: 4
      Layout.margins: 8
      Layout.fillWidth: true

      Item { Layout.fillWidth: true }

      Widgets.IconButton {
        highlighted: true
        text: qsTr("Save")
        horizontalPadding: 8
        icon.source: "qrc:/icons/buttons/apply.svg"
        ToolTip.text: qsTr("Validate and store the transmit function")

        onClicked: {
          if (editor.save())
            root.close()
        }
      }

      Widgets.IconButton {
        text: qsTr("Close")
        horizontalPadding: 8
        icon.source: "qrc:/icons/buttons/close.svg"
        ToolTip.text: qsTr("Close without storing this edit")
        onClicked: root.close()
      }
    }
  }

  Component {
    id: buttonControl

    OutputControls.DashboardButton {
      model: preview.model
      widget: preview.widget
      color: root.accentColor
    }
  }

  Component {
    id: sliderControl

    OutputControls.DashboardSlider {
      model: preview.model
      widget: preview.widget
      color: root.accentColor
    }
  }

  Component {
    id: toggleControl

    OutputControls.DashboardToggle {
      model: preview.model
      widget: preview.widget
      color: root.accentColor
    }
  }

  Component {
    id: textFieldControl

    OutputControls.DashboardTextField {
      model: preview.model
      widget: preview.widget
      color: root.accentColor
    }
  }

  Component {
    id: knobControl

    OutputControls.DashboardKnob {
      model: preview.model
      widget: preview.widget
      color: root.accentColor
    }
  }

  Views.CodeEditorMenu {
    id: contextMenu

    codeEditor: editor
  }
}
