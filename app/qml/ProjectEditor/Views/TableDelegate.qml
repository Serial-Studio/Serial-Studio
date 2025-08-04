/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Controls

import SerialStudio

ColumnLayout {
  id: root
  spacing: 0

  //
  // Custom properties
  //
  property bool headerVisible: true
  property var modelPointer: null
  property Component footerItem: null
  readonly property real tableHeight: header.height - view.height - 64

  //
  // Set width & height for column cells
  //
  readonly property real rowHeight: 30
  readonly property real iconWidth: 30
  property real parameterWidth: Math.min((root.width - root.iconWidth - 16) / 2, 420)

  //
  // Table header
  //
  Rectangle {
    id: header
    Layout.fillWidth: true
    visible: view.rows > 0 && headerVisible
    implicitHeight: root.rowHeight
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
      color: Cpp_ThemeManager.colors["table_border_header"]
      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
    }

    RowLayout {
      spacing: 0
      anchors.fill: parent

      Item {
        implicitWidth: root.iconWidth
        implicitHeight: root.iconWidth

        Canvas {
          opacity: 0.8
          anchors.centerIn: parent
          width: parent.width / 2
          height: parent.height / 2

          onPaint: {
            const size = root.iconWidth / 2

            var ctx = getContext("2d");
            ctx.clearRect(0, 0, size, size);

            ctx.beginPath();
            ctx.moveTo(size, size);
            ctx.lineTo(0, size);
            ctx.lineTo(size, 0);
            ctx.closePath();

            ctx.fillStyle = Cpp_ThemeManager.colors["table_separator"];
            ctx.fill();
          }

          Component.onCompleted: requestPaint()
        }
      }

      Rectangle {
        implicitWidth: 1
        Layout.fillHeight: true
        color: Cpp_ThemeManager.colors["table_separator"]
      }

      Item {
        implicitWidth: 8
      }

      Label {
        text: qsTr("Parameter")
        Layout.alignment: Qt.AlignVCenter
        horizontalAlignment: Label.AlignLeft
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.minimumWidth: root.parameterWidth
        Layout.maximumWidth: root.parameterWidth
        color: Cpp_ThemeManager.colors["table_fg_header"]
      }

      Rectangle {
        width: 1
        Layout.fillHeight: true
        color: Cpp_ThemeManager.colors["table_separator"]
      }

      Item {
        width: 8
      }

      Label {
        text: qsTr("Value")
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        horizontalAlignment: Label.AlignLeft
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["table_fg_header"]
      }
    }
  }

  //
  // Table items
  //
  TableView {
    id: view
    clip: true
    reuseItems: false
    interactive: false
    Layout.fillWidth: true
    model: root.modelPointer
    implicitHeight: view.rows * root.rowHeight

    //
    // Table row delegate
    //
    delegate: Item {
      id: item
      implicitWidth: view.width
      implicitHeight: root.rowHeight

      required property int row
      required property var model
      required property int column
      required property bool current

      //
      // Row background
      //
      Rectangle {
        id: background
        anchors.fill: parent
        color: Cpp_ThemeManager.colors["table_cell_bg"]

        Rectangle {
          height: 1
          color: Cpp_ThemeManager.colors["table_separator"]
          anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
          }
        }
      }

      //
      // Row items
      //
      RowLayout {
        spacing: 0
        anchors.fill: parent

        //
        // Section item
        //
        Loader {
          Layout.fillWidth: true
          Layout.rightMargin: -32
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.SectionHeader
          visible: model.widgetType === ProjectModel.SectionHeader

          sourceComponent: Rectangle {
            implicitHeight: root.rowHeight

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
              color: Cpp_ThemeManager.colors["table_border_header"]
              anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
              }
            }

            RowLayout {
              spacing: 0
              anchors.fill: parent

              Item {
                implicitHeight: root.rowHeight
                Layout.minimumWidth: root.iconWidth
                Layout.maximumWidth: root.iconWidth

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  anchors.centerIn: parent
                  source: visible ? model.parameterIcon : ""
                }
              }

              Label {
                text: model.placeholderValue
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Label.AlignLeft
                font: Cpp_Misc_CommonFonts.boldUiFont
                Layout.minimumWidth: root.parameterWidth
                Layout.maximumWidth: root.parameterWidth
                color: Cpp_ThemeManager.colors["table_fg_header"]
              }

              Item {
                Layout.fillWidth: true
              }
            }
          }
        }

        //
        // Parameter icon
        //
        Item {
          implicitHeight: root.rowHeight
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.iconWidth
          Layout.maximumWidth: root.iconWidth
          visible: model.parameterIcon !== undefined &&
                   model.widgetType !== ProjectModel.SectionHeader

          Rectangle {
            anchors.fill: parent
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
              implicitHeight: 1
              color: Cpp_ThemeManager.colors["table_separator"]

              anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
              }
            }
          }

          Image {
            id: parameterIcon
            sourceSize.width: 18
            sourceSize.height: 18
            visible: root.enabled
            anchors.centerIn: parent
            source: visible ? model.parameterIcon : ""
          }

          MultiEffect {
            saturation: -1.0
            source: parameterIcon
            visible: !root.enabled
            anchors.fill: parameterIcon
          }
        }

        //
        // Separator + Spacer
        //
        Rectangle {
          implicitWidth: 1
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["table_separator"]
          visible: model.parameterIcon !== undefined &&
                   model.widgetType !== ProjectModel.SectionHeader
        } Item {
          implicitWidth: 8
        }

        //
        // Parameter name
        //
        Label {
          opacity: model.active ? 1 : 0.5
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.parameterWidth
          Layout.maximumWidth: root.parameterWidth
          text: visible ? model.parameterName : ""
          color: Cpp_ThemeManager.colors["table_text"]

          ToolTip.delay: 700
          visible: model.widgetType !== ProjectModel.SectionHeader
          ToolTip.text: visible ? model.parameterDescription : ""
          ToolTip.visible: _paramMouseArea.containsMouse && ToolTip.text !== ""

          MouseArea {
            id: _paramMouseArea
            hoverEnabled: true
            anchors.fill: parent
          }
        }

        //
        // Separator + Spacer
        //
        Rectangle {
          width: 1
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["table_separator"]
          visible: model.widgetType !== ProjectModel.SectionHeader
        } Item {
          width: 8
          visible: model.widgetType !== ProjectModel.SectionHeader
        }

        //
        // Text field value editor
        //
        Loader {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.TextField
          visible: model.widgetType === ProjectModel.TextField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: Component {
            TextField {
              text: editableValue
              enabled: modelActive
              opacity: modelActive ? 1 : 0.5
              placeholderText: placeholderValue

              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]
              onTextEdited: {
                root.modelPointer.setData(
                      view.index(row, column),
                      text,
                      ProjectModel.EditableValue)
              }

              background: Item {}
            }
          }
        }

        //
        // Icon picker
        //
        Loader {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.IconPicker
          visible: model.widgetType === ProjectModel.IconPicker

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: Component {
            RowLayout {
              id: layout

              enabled: modelActive
              opacity: modelActive ? 1 : 0.5

              Connections {
                target: actionIconPicker

                function onIconSelected(icon) {
                  if (layout.visible)
                    model.editableValue = icon
                }
              }

              TextField {
                text: editableValue
                enabled: modelActive
                opacity: modelActive ? 1 : 0.5
                placeholderText: placeholderValue

                readOnly: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                font: Cpp_Misc_CommonFonts.monoFont
                color: Cpp_ThemeManager.colors["table_text"]
                background: Item {}
              }

              Item {
                width: 2
              }

              Button {
                enabled: modelActive
                opacity: modelActive ? 1 : 0.5

                icon.width: 16
                icon.height: 16
                Layout.maximumWidth: 32
                Layout.alignment: Qt.AlignVCenter
                icon.color: Cpp_ThemeManager.colors["table_text"]
                icon.source: "qrc:/rcc/icons/project-editor/open.svg"
                onClicked: {
                  actionIconPicker.selectedIcon = model.editableValue
                  actionIconPicker.showNormal()
                }

                background: Item {}
              }

              Item {
                width: 2
              }
            }
          }
        }

        //
        // Int number field value editor
        //
        Loader {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.IntField
          visible: model.widgetType === ProjectModel.IntField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: Component {
            TextField {
              text: editableValue
              enabled: modelActive
              opacity: modelActive ? 1 : 0.5
              placeholderText: placeholderValue

              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]

              onTextEdited: {
                const num = Number(text);
                if (!isNaN(num) && num > 0) {
                  root.modelPointer.setData(
                        view.index(row, column),
                        Number(text),
                        ProjectModel.EditableValue)
                }
              }

              validator: IntValidator {
                bottom: 1
                top: 1000000
              }

              background: Item {}
            }
          }
        }

        //
        // Double number field value editor
        //
        Loader {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.FloatField
          visible: model.widgetType === ProjectModel.FloatField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: Component {
            TextField {
              text: editableValue
              enabled: modelActive
              opacity: modelActive ? 1 : 0.5
              placeholderText: placeholderValue

              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]

              onTextEdited: {
                // Don't convert or set the data if it's an incomplete number
                if (text === "-" || text === "." || text === "-.")
                  return;

                // Convert to number only if the input is valid
                const num = Number(text);
                if (!isNaN(num)) {
                  root.modelPointer.setData(
                        view.index(row, column),
                        num,
                        ProjectModel.EditableValue
                        );
                }
              }

              validator: DoubleValidator {
                id: validator
                bottom: -1000000
                top: 1000000
              }

              background: Item {}
            }
          }
        }

        //
        // ComboBox value editor
        //
        Loader {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.ComboBox
          visible: model.widgetType === ProjectModel.ComboBox

          property var modelActive: model.active
          property var comboBoxData: model.comboBoxData
          property var editableValue: model.editableValue

          sourceComponent: Component {
            ComboBox {
              flat: true
              model: comboBoxData
              enabled: modelActive
              opacity: modelActive ? 1 : 0.5
              currentIndex: editableValue
              font: Cpp_Misc_CommonFonts.monoFont
              onCurrentIndexChanged: {
                if (currentIndex !== editableValue) {
                  root.modelPointer.setData(
                        view.index(row, column),
                        currentIndex,
                        ProjectModel.EditableValue)
                }
              }
            }
          }
        }

        //
        // CheckBox value editor
        //
        Loader {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.CheckBox
          visible: model.widgetType === ProjectModel.CheckBox

          property var modelActive: model.active
          property var comboBoxData: model.comboBoxData
          property var editableValue: model.editableValue

          sourceComponent: Component {
            ComboBox {
              flat: true
              enabled: modelActive
              opacity: modelActive ? 1 : 0.5
              model: [qsTr("No"), qsTr("Yes")]
              currentIndex: editableValue ? 1 : 0
              font: Cpp_Misc_CommonFonts.monoFont
              onCurrentIndexChanged: {
                root.modelPointer.setData(
                      view.index(row, column),
                      currentIndex === 1,
                      ProjectModel.EditableValue)
              }
            }
          }
        }

        //
        // HexTextEdit editor
        //
        Loader {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectModel.HexTextField
          visible: model.widgetType === ProjectModel.HexTextField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: Component {
            TextField {
              text: editableValue
              enabled: modelActive
              opacity: modelActive ? 1 : 0.5
              placeholderText: placeholderValue

              id: _hexComponent
              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]

              //
              // Add space automatically in hex view
              //
              onTextChanged: {
                // Get the current cursor position
                const currentCursorPosition = _hexComponent.cursorPosition;
                const cursorAtEnd = (currentCursorPosition === _hexComponent.text.length);

                // Format the text
                const originalText = _hexComponent.text;
                const formattedText = Cpp_IO_Console.formatUserHex(_hexComponent.text);
                const isValid = Cpp_IO_Console.validateUserHex(formattedText);

                // Update the text only if it has changed
                if (originalText !== formattedText) {
                  _hexComponent.text = formattedText;

                  // Restore the cursor position, adjusting for added spaces
                  if (!cursorAtEnd) {
                    // Remove spaces from originalText and formattedText to compare lengths
                    const cleanedOriginalText = originalText.replace(/ /g, '');
                    const cleanedFormattedText = formattedText.replace(/ /g, '');

                    // Calculate the difference in length due to formatting
                    const lengthDifference = cleanedFormattedText.length - cleanedOriginalText.length;

                    // Count spaces before the cursor in both texts
                    let spacesBeforeCursorOriginal = (originalText.slice(0, currentCursorPosition).match(/ /g) || []).length;
                    let spacesBeforeCursorFormatted = (formattedText.slice(0, currentCursorPosition).match(/ /g) || []).length;

                    // Calculate adjustment factor
                    const adjustment = spacesBeforeCursorFormatted - spacesBeforeCursorOriginal + lengthDifference;

                    // Restore the cursor position with adjustment
                    _hexComponent.cursorPosition = Math.min(currentCursorPosition + adjustment, _hexComponent.text.length);
                  }
                }

                // Update the palette based on validation
                _hexComponent.color = isValid
                    ? Cpp_ThemeManager.colors["table_text"]
                    : Cpp_ThemeManager.colors["alarm"]

                // Set model data
                root.modelPointer.setData(
                      view.index(row, column),
                      formattedText,
                      ProjectModel.EditableValue)
              }

              background: Item {}
            }
          }
        }

        //
        // Separator
        //
        Item {
          implicitWidth: 8
          visible: model.widgetType !== ProjectModel.SectionHeader
        }
      }
    }
  }

  //
  // Spacer
  //
  Item {
    Layout.fillHeight: true
    Layout.minimumHeight: 32
  }

  //
  // Extra item
  //
  Loader {
    active: root.footerItem !== null
    sourceComponent: root.footerItem
    Layout.alignment: Qt.AlignHCenter
  }

  //
  // Spacer
  //
  Item {
    Layout.fillHeight: true
    Layout.minimumHeight: 32
  }
}
