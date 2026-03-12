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
  property var modelPointer: null
  property bool headerVisible: true
  property bool spacerVisible: true
  property Component footerItem: null

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
    implicitHeight: root.rowHeight
    visible: view.rows > 0 && headerVisible

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
      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
      height: 1
      color: Cpp_ThemeManager.colors["table_border_header"]
    }

    RowLayout {
      spacing: 0
      anchors.fill: parent

      Item {
        implicitWidth: root.iconWidth
        implicitHeight: root.iconWidth

        Canvas {
          opacity: 0.8
          width: parent.width / 2
          anchors.centerIn: parent
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
          anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
          }
          height: 1
          color: Cpp_ThemeManager.colors["table_separator"]
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
          active: model.widgetType === ProjectEditor.SectionHeader
          visible: model.widgetType === ProjectEditor.SectionHeader

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
              anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
              }
              color: Cpp_ThemeManager.colors["table_border_header"]
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
                  source: model.parameterIcon || ""
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
                   model.widgetType !== ProjectEditor.SectionHeader

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
            source: model.parameterIcon || ""
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
          visible: model.parameterIcon !== undefined &&
                   model.widgetType !== ProjectEditor.SectionHeader
          color: Cpp_ThemeManager.colors["table_separator"]
        } Item {
          implicitWidth: 8
        }

        //
        // Parameter name
        //
        Label {
          text: model.parameterName ?? ""
          opacity: model.active ? 1 : 0.5
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.parameterWidth
          Layout.maximumWidth: root.parameterWidth
          color: Cpp_ThemeManager.colors["table_text"]

          ToolTip.delay: 700
          ToolTip.text: model.parameterDescription ?? ""
          visible: model.widgetType !== ProjectEditor.SectionHeader
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
          visible: model.widgetType !== ProjectEditor.SectionHeader
        } Item {
          width: 8
          visible: model.widgetType !== ProjectEditor.SectionHeader
        }

        //
        // Text field value editor
        //
        Loader {
          id: textFieldLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.TextField
          visible: model.widgetType === ProjectEditor.TextField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue
          property int modelRow: row
          property int modelColumn: column

          sourceComponent: TextField {
            text: textFieldLoader.editableValue
            enabled: textFieldLoader.modelActive
            opacity: textFieldLoader.modelActive ? 1 : 0.5
            placeholderText: textFieldLoader.modelPlaceholder ?? ""

            onTextEdited: {
              root.modelPointer.setData(
                    view.index(textFieldLoader.modelRow, textFieldLoader.modelColumn),
                    text,
                    ProjectEditor.EditableValue)
            }
            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["table_text"]

            background: Item {}
          }
        }

        //
        // Icon picker
        //
        Loader {
          id: iconPickerLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.IconPicker
          visible: model.widgetType === ProjectEditor.IconPicker

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue
          property int modelRow: row
          property int modelColumn: column

          sourceComponent: RowLayout {
            id: iconPickerRow

            enabled: iconPickerLoader.modelActive
            opacity: iconPickerLoader.modelActive ? 1 : 0.5

            Connections {
              target: actionIconPicker

              function onIconSelected(icon) {
                if (iconPickerRow.visible) {
                  root.modelPointer.setData(
                        view.index(iconPickerLoader.modelRow, iconPickerLoader.modelColumn),
                        icon,
                        ProjectEditor.EditableValue)
                }
              }
            }

            TextField {
              text: iconPickerLoader.editableValue
              enabled: iconPickerLoader.modelActive
              opacity: iconPickerLoader.modelActive ? 1 : 0.5
              placeholderText: iconPickerLoader.modelPlaceholder ?? ""

              readOnly: true
              background: Item {}
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]
            }

            Item {
              width: 2
            }

            Button {
              enabled: iconPickerLoader.modelActive
              opacity: iconPickerLoader.modelActive ? 1 : 0.5

              onClicked: {
                actionIconPicker.selectedIcon = iconPickerLoader.editableValue
                actionIconPicker.showNormal()
              }
              icon.width: 16
              icon.height: 16
              Layout.maximumWidth: 32
              Layout.alignment: Qt.AlignVCenter
              icon.color: Cpp_ThemeManager.colors["table_text"]
              icon.source: "qrc:/rcc/icons/project-editor/open.svg"

              background: Item {}
            }

            Item {
              width: 2
            }
          }
        }

        //
        // Int number field value editor
        //
        Loader {
          id: intFieldLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.IntField
          visible: model.widgetType === ProjectEditor.IntField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue
          property int modelRow: row
          property int modelColumn: column
          property int modelMin: model.minValue ?? 0
          property int modelMax: model.maxValue ?? 1000000

          sourceComponent: TextField {
            text: intFieldLoader.editableValue
            enabled: intFieldLoader.modelActive
            opacity: intFieldLoader.modelActive ? 1 : 0.5
            placeholderText: intFieldLoader.modelPlaceholder ?? ""

            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["table_text"]

            onTextEdited: {
              const num = parseInt(text, 10);
              if (!isNaN(num)) {
                root.modelPointer.setData(
                      view.index(intFieldLoader.modelRow, intFieldLoader.modelColumn),
                      num,
                      ProjectEditor.EditableValue)
              }
            }

            validator: IntValidator {
              bottom: intFieldLoader.modelMin
              top: intFieldLoader.modelMax
            }

            background: Item {}
          }
        }

        //
        // Double number field value editor
        //
        Loader {
          id: floatFieldLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.FloatField
          visible: model.widgetType === ProjectEditor.FloatField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue
          property int modelRow: row
          property int modelColumn: column
          property real modelMin: model.minValue ?? -1000000
          property real modelMax: model.maxValue ?? 1000000

          sourceComponent: TextField {
            text: floatFieldLoader.editableValue
            enabled: floatFieldLoader.modelActive
            opacity: floatFieldLoader.modelActive ? 1 : 0.5
            placeholderText: floatFieldLoader.modelPlaceholder ?? ""

            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["table_text"]

            onTextEdited: {
              if (text === "-" || text === "." || text === "-.")
                return;

              const num = Number(text);
              if (!isNaN(num)) {
                root.modelPointer.setData(
                      view.index(floatFieldLoader.modelRow, floatFieldLoader.modelColumn),
                      num,
                      ProjectEditor.EditableValue)
              }
            }

            validator: DoubleValidator {
              bottom: floatFieldLoader.modelMin
              top: floatFieldLoader.modelMax
            }

            background: Item {}
          }
        }

        //
        // ComboBox value editor
        //
        Loader {
          id: comboBoxLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.ComboBox
          visible: model.widgetType === ProjectEditor.ComboBox

          property var modelActive: model.active
          property var comboBoxData: model.comboBoxData
          property var editableValue: model.editableValue
          property int modelRow: row
          property int modelColumn: column

          sourceComponent: ComboBox {
            flat: true
            model: comboBoxLoader.comboBoxData
            enabled: comboBoxLoader.modelActive
            onCurrentIndexChanged: {
              if (currentIndex !== comboBoxLoader.editableValue) {
                root.modelPointer.setData(
                      view.index(comboBoxLoader.modelRow, comboBoxLoader.modelColumn),
                      currentIndex,
                      ProjectEditor.EditableValue)
              }
            }
            currentIndex: comboBoxLoader.editableValue
            opacity: comboBoxLoader.modelActive ? 1 : 0.5
            font: Cpp_Misc_CommonFonts.monoFont
          }
        }

        //
        // CheckBox value editor
        //
        Loader {
          id: checkBoxLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.CheckBox
          visible: model.widgetType === ProjectEditor.CheckBox

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property int modelRow: row
          property int modelColumn: column

          sourceComponent: ComboBox {
            flat: true
            enabled: checkBoxLoader.modelActive
            onCurrentIndexChanged: {
              root.modelPointer.setData(
                    view.index(checkBoxLoader.modelRow, checkBoxLoader.modelColumn),
                    currentIndex === 1,
                    ProjectEditor.EditableValue)
            }
            opacity: checkBoxLoader.modelActive ? 1 : 0.5
            model: [qsTr("No"), qsTr("Yes")]
            currentIndex: checkBoxLoader.editableValue ? 1 : 0
            font: Cpp_Misc_CommonFonts.monoFont
          }
        }

        //
        // HexTextEdit editor
        //
        Loader {
          id: hexFieldLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.HexTextField
          visible: model.widgetType === ProjectEditor.HexTextField

          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue
          property int modelRow: row
          property int modelColumn: column

          sourceComponent: TextField {
            id: _hexComponent

            text: hexFieldLoader.editableValue
            enabled: hexFieldLoader.modelActive
            opacity: hexFieldLoader.modelActive ? 1 : 0.5
            placeholderText: hexFieldLoader.modelPlaceholder ?? ""

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
                const formattedText = Cpp_Console_Handler.formatUserHex(_hexComponent.text);
                const isValid = Cpp_Console_Handler.validateUserHex(formattedText);

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
                      view.index(hexFieldLoader.modelRow, hexFieldLoader.modelColumn),
                      formattedText,
                      ProjectEditor.EditableValue)
              }

              background: Item {}
          }
        }

        //
        // Separator
        //
        Item {
          implicitWidth: 8
          visible: model.widgetType !== ProjectEditor.SectionHeader
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
    visible: root.spacerVisible
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
    visible: root.spacerVisible
  }
}
