/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

ColumnLayout {
  id: root
  spacing: 0

  //
  // Custom properties
  //
  property var modelPointer: null
  property Component footerItem: null
  readonly property real tableHeight: header.height - view.height - 64

  //
  // Set width & height for column cells
  //
  property real rowHeight: 30
  property real valueWidth: Math.min(root.width * 0.3, 256)
  property real parameterWidth: Math.min(root.width * 0.3, 256)
  property real descriptionWidth: root.width - parameterWidth - valueWidth - 14

  //
  // Table header
  //
  Rectangle {
    id: header
    Layout.fillWidth: true
    visible: view.rows > 0
    implicitHeight: root.rowHeight
    color: Cpp_ThemeManager.colors["table_bg_header"]

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
      anchors.leftMargin: 8
      anchors.rightMargin: 8

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
        color: Cpp_ThemeManager.colors["table_separator_header"]
      }

      Item {
        width: 4
      }

      TextField {
        text: qsTr("Value")
        readOnly: true
        selectByMouse: false
        Layout.alignment: Qt.AlignVCenter
        Layout.minimumWidth: root.valueWidth
        Layout.maximumWidth: root.valueWidth
        horizontalAlignment: Label.AlignLeft
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["table_fg_header"]
        background: Item {}
      }

      Rectangle {
        width: 1
        Layout.fillHeight: true
        color: Cpp_ThemeManager.colors["table_separator_header"]
      }

      Item {
        width: 8
      }

      Label {
        text: qsTr("Parameter Description")
        Layout.alignment: Qt.AlignVCenter
        horizontalAlignment: Label.AlignLeft
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.minimumWidth: root.descriptionWidth
        Layout.maximumWidth: root.descriptionWidth
        color: Cpp_ThemeManager.colors["table_fg_header"]
      }
    }
  }

  //
  // Table items
  //
  TreeView {
    id: view
    clip: true
    reuseItems: false
    Layout.fillWidth: true
    model: root.modelPointer
    boundsBehavior: TreeView.StopAtBounds
    Layout.minimumHeight: root.rowHeight * rows
    Layout.maximumHeight: root.rowHeight * rows

    //
    // Table "row" delegate
    //
    delegate: Item {
      id: item
      implicitWidth: view.width
      implicitHeight: root.rowHeight

      required property int row
      required property int column
      required property int depth
      required property bool current
      required property bool expanded
      required property bool isTreeNode
      required property int hasChildren
      required property TreeView treeView

      //
      // Row background
      //
      Rectangle {
        id: background
        anchors.fill: parent
        color: row % 2 ? Cpp_ThemeManager.colors["table_bg_a"] :
                         Cpp_ThemeManager.colors["table_bg_b"]

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
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        //
        // Parameter name
        //
        Label {
          text: model.parameterName
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.parameterWidth
          Layout.maximumWidth: root.parameterWidth
          color: Cpp_ThemeManager.colors["table_text"]
        }

        //
        // Separator + Spacer
        //
        Rectangle {
          width: 1
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["table_separator"]
        } Item {
          width: 4
        }

        //
        // Text field value editor
        //
        Loader {
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.valueWidth
          Layout.maximumWidth: root.valueWidth
          active: model.widgetType === ProjectModel.TextField
          visible: model.widgetType === ProjectModel.TextField

          sourceComponent: TextField {
            text: model.editableValue
            font: Cpp_Misc_CommonFonts.monoFont
            placeholderText: model.placeholderValue
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

        //
        // Icon picker
        //
        Loader {
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.valueWidth
          Layout.maximumWidth: root.valueWidth
          active: model.widgetType === ProjectModel.IconPicker
          visible: model.widgetType === ProjectModel.IconPicker

          sourceComponent: RowLayout {
            id: layout

            Connections {
              target: actionIconPicker

              function onIconSelected(icon) {
                if (layout.visible)
                  model.editableValue = icon
              }
            }

            TextField {
              readOnly: true
              Layout.fillWidth: true
              text: model.editableValue
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              placeholderText: model.placeholderValue
              color: Cpp_ThemeManager.colors["table_text"]
              background: Item {}
            }

            Item {
              width: 2
            }

            Button {
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

        //
        // Int number field value editor
        //
        Loader {
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.valueWidth
          Layout.maximumWidth: root.valueWidth
          active: model.widgetType === ProjectModel.IntField
          visible: model.widgetType === ProjectModel.IntField

          sourceComponent: TextField {
            text: model.editableValue
            font: Cpp_Misc_CommonFonts.monoFont
            placeholderText: model.placeholderValue
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

        //
        // Double number field value editor
        //
        Loader {
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.valueWidth
          Layout.maximumWidth: root.valueWidth
          active: model.widgetType === ProjectModel.FloatField
          visible: model.widgetType === ProjectModel.FloatField

          sourceComponent: TextField {
            text: model.editableValue
            font: Cpp_Misc_CommonFonts.monoFont
            placeholderText: model.placeholderValue
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

        //
        // ComboBox value editor
        //
        Loader {
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.valueWidth
          Layout.maximumWidth: root.valueWidth
          active: model.widgetType === ProjectModel.ComboBox
          visible: model.widgetType === ProjectModel.ComboBox

          property var comboBoxData: model.comboBoxData
          property var editableValue: model.editableValue

          sourceComponent: ComboBox {
            flat: true
            model: comboBoxData
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

        //
        // CheckBox value editor
        //
        Loader {
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.valueWidth
          Layout.maximumWidth: root.valueWidth
          active: model.widgetType === ProjectModel.CheckBox
          visible: model.widgetType === ProjectModel.CheckBox

          property var comboBoxData: model.comboBoxData
          property var editableValue: model.editableValue

          sourceComponent: ComboBox {
            flat: true
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

        //
        // HexTextEdit editor
        //
        Loader {
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.valueWidth
          Layout.maximumWidth: root.valueWidth
          active: model.widgetType === ProjectModel.HexTextField
          visible: model.widgetType === ProjectModel.HexTextField

          sourceComponent: TextField {
            id: _hexComponent
            text: model.editableValue
            font: Cpp_Misc_CommonFonts.monoFont
            placeholderText: model.placeholderValue
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

        //
        // Separator + Spacer
        //
        Rectangle {
          width: 1
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["table_separator"]
        } Item {
          width: 8
        }

        //
        // Parameter description/hint
        //
        Label {
          opacity: 0.8
          text: model.parameterDescription
          Layout.alignment: Qt.AlignVCenter
          Layout.minimumWidth: root.descriptionWidth
          Layout.maximumWidth: root.descriptionWidth
          color: Cpp_ThemeManager.colors["table_text"]
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
