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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Dialogs
import QtQuick.Controls

import SerialStudio
import "../../Widgets" as Widgets

ColumnLayout {
  id: root

  spacing: 0

  //
  // Custom properties
  //
  property var modelPointer: null
  property bool searchable: false
  property bool headerVisible: true
  property bool spacerVisible: true
  property Component footerItem: null
  property alias searchQuery: filterProxy.query

  //
  // Optional row filter, interposed only when searchable; every setData below goes through
  // view.model so filtered row indices stay mapped (search UI lives in EditorSearchBand)
  //
  RowFilterProxy {
    id: filterProxy

    sourceModel: root.searchable ? root.modelPointer : null
  }

  //
  // Set width & height for column cells
  //
  readonly property real rowHeight: 30
  readonly property real iconWidth: 30
  readonly property bool rtl: Cpp_Misc_Translator.rtl
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
      LayoutMirroring.enabled: root.rtl
      LayoutMirroring.childrenInherit: true

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

          //
          // The glyph is painted with a theme color, so it has to be repainted when the theme
          // changes; without this it kept the previous palette's separator color
          //
          Connections {
            target: Cpp_ThemeManager

            function onThemeChanged() {
              requestPaint()
            }
          }
        }
      }

      Rectangle {
        implicitWidth: 1
        Layout.fillHeight: true
        color: Cpp_ThemeManager.colors["table_separator"]
      }

      Item {
        implicitWidth: 6
      }

      Label {
        text: qsTr("Parameter")
        elide: Label.ElideRight
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.minimumWidth: root.parameterWidth
        Layout.maximumWidth: root.parameterWidth
        color: Cpp_ThemeManager.colors["table_fg_header"]
        horizontalAlignment: Label.AlignLeft
      }

      Rectangle {
        width: 1
        Layout.fillHeight: true
        color: Cpp_ThemeManager.colors["table_separator"]
      }

      Item {
        width: 6
      }

      Label {
        text: qsTr("Value")
        elide: Label.ElideRight
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["table_fg_header"]
        horizontalAlignment: Label.AlignLeft
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
    implicitHeight: view.rows * root.rowHeight
    model: root.searchable ? filterProxy : root.modelPointer

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

      readonly property bool isNav: model.widgetType === ProjectEditor.NavRow

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
        LayoutMirroring.enabled: root.rtl
        LayoutMirroring.childrenInherit: true

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
                elide: Label.ElideRight
                Layout.alignment: Qt.AlignVCenter
                font: Cpp_Misc_CommonFonts.boldUiFont
                Layout.fillWidth: true
                color: Cpp_ThemeManager.colors["table_fg_header"]
                horizontalAlignment: Label.AlignLeft
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
                   model.widgetType !== ProjectEditor.SectionHeader &&
                   !item.isNav

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
            anchors.centerIn: parent
            source: model.parameterIcon || ""
            visible: root.enabled || !Cpp_Misc_GraphicsBackend.effectsEnabled
          }

          MultiEffect {
            saturation: -1.0
            source: parameterIcon
            anchors.fill: parameterIcon
            enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
            visible: !root.enabled && Cpp_Misc_GraphicsBackend.effectsEnabled
          }
        }

        //
        // Separator + Spacer
        //
        Rectangle {
          implicitWidth: 1
          Layout.fillHeight: true
          visible: model.parameterIcon !== undefined &&
                   model.widgetType !== ProjectEditor.SectionHeader &&
                   !item.isNav
          color: Cpp_ThemeManager.colors["table_separator"]
        } Item {
          implicitWidth: 6
          visible: model.parameterIcon !== undefined &&
                   model.widgetType !== ProjectEditor.SectionHeader &&
                   !item.isNav
        }

        //
        // Navigation row icon (same per-dataset icon as the tree view)
        //
        Item {
          implicitWidth: 6
          visible: item.isNav
        } Image {
          visible: item.isNav
          sourceSize.width: 18
          sourceSize.height: 18
          source: model.parameterIcon || ""
          Layout.alignment: Qt.AlignVCenter
        } Item {
          implicitWidth: 2
          visible: item.isNav
        }

        //
        // Parameter name
        //
        Label {
          text: model.parameterName ?? ""
          elide: Label.ElideRight
          leftPadding: root.rtl ? 0 : 6
          rightPadding: root.rtl ? 6 : 0
          opacity: model.active ? 1 : 0.5
          Layout.fillWidth: item.isNav
          Layout.alignment: Qt.AlignVCenter
          color: Cpp_ThemeManager.colors["table_text"]
          horizontalAlignment: Label.AlignLeft
          Layout.minimumWidth: item.isNav ? 0 : root.parameterWidth
          Layout.maximumWidth: item.isNav ? Number.POSITIVE_INFINITY : root.parameterWidth

          ToolTip.delay: 700
          visible: model.widgetType !== ProjectEditor.SectionHeader
          ToolTip.text: truncated ? (model.parameterName ?? "")
                                  : (model.parameterDescription ?? "")
          ToolTip.visible: _paramMouseArea.containsMouse &&
                           ToolTip.text !== ""

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
          visible: model.widgetType !== ProjectEditor.SectionHeader && !item.isNav
        } Item {
          width: 6
          visible: model.widgetType !== ProjectEditor.SectionHeader && !item.isNav
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

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: TextField {
            enabled: textFieldLoader.modelActive
            opacity: textFieldLoader.modelActive ? 1 : 0.5
            placeholderText: textFieldLoader.modelPlaceholder ?? ""

            property var modelValue: textFieldLoader.editableValue

            Component.onCompleted: text = String(textFieldLoader.editableValue ?? "")
            onModelValueChanged: {
              if (!activeFocus)
                text = String(modelValue ?? "")
            }

            onTextEdited: {
              view.model.setData(
                    view.index(textFieldLoader.modelRow, textFieldLoader.modelColumn),
                    text,
                    ProjectEditor.EditableValue)
            }
            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["table_text"]
            horizontalAlignment: TextInput.AlignLeft

            background: Item {}
          }
        }

        //
        // Password field value editor (masked input)
        //
        Loader {
          id: passwordFieldLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.PasswordField
          visible: model.widgetType === ProjectEditor.PasswordField

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: TextField {
            echoMode: TextInput.Password
            enabled: passwordFieldLoader.modelActive
            opacity: passwordFieldLoader.modelActive ? 1 : 0.5
            placeholderText: passwordFieldLoader.modelPlaceholder ?? ""

            property var modelValue: passwordFieldLoader.editableValue

            Component.onCompleted: text = String(passwordFieldLoader.editableValue ?? "")
            onModelValueChanged: {
              if (!activeFocus)
                text = String(modelValue ?? "")
            }

            onTextEdited: {
              view.model.setData(
                    view.index(passwordFieldLoader.modelRow, passwordFieldLoader.modelColumn),
                    text,
                    ProjectEditor.EditableValue)
            }
            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["table_text"]
            horizontalAlignment: TextInput.AlignLeft

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

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: RowLayout {
            id: iconPickerRow

            enabled: iconPickerLoader.modelActive
            opacity: iconPickerLoader.modelActive ? 1 : 0.5

            Connections {
              target: actionIconPicker

              function onIconSelected(icon) {
                if (iconPickerRow.visible) {
                  view.model.setData(
                        view.index(iconPickerLoader.modelRow, iconPickerLoader.modelColumn),
                        icon,
                        ProjectEditor.EditableValue)
                }
              }
            }

            TextField {
              text: Cpp_Misc_IconEngine.isInlineSvg(iconPickerLoader.editableValue)
                    ? qsTr("(Custom Icon)")
                    : iconPickerLoader.editableValue
              enabled: iconPickerLoader.modelActive
              opacity: iconPickerLoader.modelActive ? 1 : 0.5
              placeholderText: iconPickerLoader.modelPlaceholder ?? ""

              readOnly: true
              background: Item {}
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]
              horizontalAlignment: TextInput.AlignLeft
            }

            Item {
              width: 2
            }

            Widgets.IconButton {
              iconSize: 16
              enabled: iconPickerLoader.modelActive
              opacity: iconPickerLoader.modelActive ? 1 : 0.5

              onClicked: {
                actionIconPicker.selectedIcon = Cpp_Misc_IconEngine.isInlineSvg(
                  iconPickerLoader.editableValue) ? "" : iconPickerLoader.editableValue
                actionIconPicker.showNormal()
              }
              Layout.maximumWidth: 32
              Layout.alignment: Qt.AlignVCenter
              icon.color: Cpp_ThemeManager.colors["table_text"]
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "open", 16)

              background: Item {}
            }

            Item {
              width: 2
            }
          }
        }

        //
        // Color picker (empty value = automatic theme color)
        //
        Loader {
          id: colorPickerLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.ColorPicker
          visible: model.widgetType === ProjectEditor.ColorPicker

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: RowLayout {
            id: colorPickerRow

            enabled: colorPickerLoader.modelActive
            opacity: colorPickerLoader.modelActive ? 1 : 0.5

            readonly property bool isAutomatic: !colorPickerLoader.editableValue

            function hexFromColor(c) {
              const r = Math.round(c.r * 255).toString(16).padStart(2, '0')
              const g = Math.round(c.g * 255).toString(16).padStart(2, '0')
              const b = Math.round(c.b * 255).toString(16).padStart(2, '0')
              return "#" + r + g + b
            }

            function commit(value) {
              view.model.setData(
                    view.index(colorPickerLoader.modelRow, colorPickerLoader.modelColumn),
                    value,
                    ProjectEditor.EditableValue)
            }

            ColorDialog {
              id: datasetColorDialog

              title: qsTr("Choose Dataset Color")
              onAccepted: colorPickerRow.commit(colorPickerRow.hexFromColor(selectedColor))
            }

            Rectangle {
              radius: 2
              implicitWidth: 16
              implicitHeight: 16
              border.width: 1
              visible: !colorPickerRow.isAutomatic
              Layout.alignment: Qt.AlignVCenter
              border.color: Cpp_ThemeManager.colors["table_separator"]
              color: colorPickerRow.isAutomatic ? "transparent"
                                                : colorPickerLoader.editableValue
            }

            TextField {
              enabled: colorPickerLoader.modelActive
              opacity: colorPickerLoader.modelActive ? 1 : 0.5
              text: colorPickerRow.isAutomatic ? "" : colorPickerLoader.editableValue
              placeholderText: colorPickerLoader.modelPlaceholder ?? qsTr("Automatic")

              readOnly: true
              background: Item {}
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]
              horizontalAlignment: TextInput.AlignLeft
            }

            Item {
              width: 2
            }

            Widgets.IconButton {
              iconSize: 16
              enabled: colorPickerLoader.modelActive
              opacity: colorPickerLoader.modelActive ? 1 : 0.5

              onClicked: {
                if (!colorPickerRow.isAutomatic)
                  datasetColorDialog.selectedColor = colorPickerLoader.editableValue

                datasetColorDialog.open()
              }

              Layout.maximumWidth: 32
              Layout.alignment: Qt.AlignVCenter
              icon.source: "qrc:/icons/buttons/color.svg"
              icon.color: Cpp_ThemeManager.colors["table_text"]

              background: Item {}
            }

            Widgets.IconButton {
              iconSize: 16
              visible: !colorPickerRow.isAutomatic
              enabled: colorPickerLoader.modelActive
              opacity: colorPickerLoader.modelActive ? 1 : 0.5

              Layout.maximumWidth: 32
              Layout.alignment: Qt.AlignVCenter
              onClicked: colorPickerRow.commit("")
              icon.source: "qrc:/icons/buttons/close.svg"
              icon.color: Cpp_ThemeManager.colors["table_text"]

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

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue
          property int modelMax: model.maxValue ?? 1000000
          property int modelMin: model.minValue ?? -1000000
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: TextField {
            leftPadding: 6
            rightPadding: 6
            enabled: intFieldLoader.modelActive
            opacity: intFieldLoader.modelActive ? 1 : 0.5
            placeholderText: intFieldLoader.modelPlaceholder ?? ""

            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["table_text"]
            horizontalAlignment: TextInput.AlignLeft

            //
            // Sync from the model only while unfocused (HexTextEdit pattern): a live binding
            // echoes the last commit back and refills the field the moment the user clears it
            //
            property var modelValue: intFieldLoader.editableValue

            Component.onCompleted: text = String(modelValue ?? "")
            onModelValueChanged: {
              if (!activeFocus)
                text = String(modelValue ?? "")
            }

            //
            // Bounds are enforced at commit, not per keystroke: a hard validator swallows the
            // leading "-" of a negative number, so only the shape is validated while typing
            //
            onTextEdited: {
              const num = parseInt(text, 10);
              if (!isNaN(num)) {
                const clamped = Math.max(intFieldLoader.modelMin,
                                         Math.min(intFieldLoader.modelMax, num));
                view.model.setData(
                      view.index(intFieldLoader.modelRow, intFieldLoader.modelColumn),
                      clamped,
                      ProjectEditor.EditableValue)
              }
            }

            onEditingFinished: text = String(intFieldLoader.editableValue ?? "")

            validator: RegularExpressionValidator {
              regularExpression: /^-?\d*$/
            }

            background: Item {}
          }
        }

        //
        // Int field with 0 = "Auto" placeholder, rendered as SpinBox
        //
        Loader {
          id: autoIntFieldLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.AutoIntField
          visible: model.widgetType === ProjectEditor.AutoIntField

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property int modelMin: model.minValue ?? 0
          property int modelMax: model.maxValue ?? 99
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: SpinBox {
            id: _autoIntSpin

            editable: true
            to: autoIntFieldLoader.modelMax
            from: autoIntFieldLoader.modelMin
            font: Cpp_Misc_CommonFonts.monoFont
            enabled: autoIntFieldLoader.modelActive

            property var modelValue: autoIntFieldLoader.editableValue

            Component.onCompleted: value = Number(autoIntFieldLoader.editableValue ?? 0)
            onModelValueChanged: {
              if (!activeFocus)
                value = Number(modelValue ?? 0)
            }
            opacity: autoIntFieldLoader.modelActive ? 1 : 0.5

            textFromValue: function(value, locale) {
              if (value === autoIntFieldLoader.modelMin)
                return autoIntFieldLoader.modelPlaceholder ?? qsTr("Auto")

              return Number(value).toLocaleString(locale, 'f', 0)
            }

            valueFromText: function(text, locale) {
              const auto = autoIntFieldLoader.modelPlaceholder ?? qsTr("Auto")
              if (text === auto || text === "" || text.toLowerCase() === "auto")
                return autoIntFieldLoader.modelMin

              try {
                const n = Number.fromLocaleString(locale, text)
                return isNaN(n) ? _autoIntSpin.value : Math.round(n)
              } catch (e) {
                const n = parseInt(text, 10)
                return isNaN(n) ? _autoIntSpin.value : n
              }
            }

            onValueModified: {
              view.model.setData(
                    view.index(autoIntFieldLoader.modelRow, autoIntFieldLoader.modelColumn),
                    value,
                    ProjectEditor.EditableValue)
            }

            contentItem: TextInput {
              leftPadding: 2
              rightPadding: 6
              text: _autoIntSpin.displayText
              font: _autoIntSpin.font
              color: Cpp_ThemeManager.colors["table_text"]
              selectionColor: Cpp_ThemeManager.colors["highlight"]
              selectedTextColor: Cpp_ThemeManager.colors["highlighted_text"]
              horizontalAlignment: Qt.AlignLeft
              verticalAlignment: Qt.AlignVCenter
              readOnly: !_autoIntSpin.editable
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

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue
          property real modelMax: model.maxValue ?? 1000000
          property real modelMin: model.minValue ?? -1000000
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: TextField {
            leftPadding: 6
            rightPadding: 6
            enabled: floatFieldLoader.modelActive
            opacity: floatFieldLoader.modelActive ? 1 : 0.5
            placeholderText: floatFieldLoader.modelPlaceholder ?? ""

            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["table_text"]
            horizontalAlignment: TextInput.AlignLeft

            //
            // Sync from the model only while unfocused, so clearing the field to retype a
            // number is never fought by the previous keystroke's commit echoing back
            //
            property var modelValue: floatFieldLoader.editableValue

            Component.onCompleted: text = String(floatFieldLoader.editableValue ?? "")
            onModelValueChanged: {
              if (!activeFocus)
                text = String(modelValue ?? "")
            }

            //
            // Same commit-time bounds policy as the int field: never key-block the minus sign
            //
            onTextEdited: {
              if (text === "-" || text === "." || text === "-.")
                return;

              const num = Number(text);
              if (!isNaN(num)) {
                const clamped = Math.max(floatFieldLoader.modelMin,
                                         Math.min(floatFieldLoader.modelMax, num));
                view.model.setData(
                      view.index(floatFieldLoader.modelRow, floatFieldLoader.modelColumn),
                      clamped,
                      ProjectEditor.EditableValue)
              }
            }

            onEditingFinished: text = String(floatFieldLoader.editableValue ?? "")

            validator: RegularExpressionValidator {
              regularExpression: /^-?\d*\.?\d*([eE][-+]?\d*)?$/
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

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var comboBoxData: model.comboBoxData
          property var editableValue: model.editableValue

          sourceComponent: ComboBox {
            id: _comboBox

            flat: true
            model: comboBoxLoader.comboBoxData
            enabled: comboBoxLoader.modelActive
            onActivated: {
              if (currentIndex !== comboBoxLoader.editableValue) {
                view.model.setData(
                      view.index(comboBoxLoader.modelRow, comboBoxLoader.modelColumn),
                      currentIndex,
                      ProjectEditor.EditableValue)
              }
            }
            font: Cpp_Misc_CommonFonts.monoFont
            currentIndex: comboBoxLoader.editableValue
            opacity: comboBoxLoader.modelActive ? 1 : 0.5

            contentItem: Text {
              font: _comboBox.font
              elide: Text.ElideRight
              text: _comboBox.displayText
              verticalAlignment: Text.AlignVCenter
              color: Cpp_ThemeManager.colors["table_text"]
              leftPadding: root.rtl ? _comboBox.indicator.width : 6
              rightPadding: root.rtl ? 6 : _comboBox.indicator.width
              horizontalAlignment: Text.AlignLeft
            }
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

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue

          sourceComponent: ComboBox {
            id: _checkBox

            flat: true
            enabled: checkBoxLoader.modelActive
            onActivated: {
              if (checkBoxLoader.editableValue === -1
                  || (currentIndex === 1) !== Boolean(checkBoxLoader.editableValue)) {
                view.model.setData(
                      view.index(checkBoxLoader.modelRow, checkBoxLoader.modelColumn),
                      currentIndex === 1,
                      ProjectEditor.EditableValue)
              }
            }
            opacity: checkBoxLoader.modelActive ? 1 : 0.5
            model: [qsTr("No"), qsTr("Yes")]
            currentIndex: checkBoxLoader.editableValue === -1
                          ? -1 : (checkBoxLoader.editableValue ? 1 : 0)
            font: Cpp_Misc_CommonFonts.monoFont

            contentItem: Text {
              font: _checkBox.font
              elide: Text.ElideRight
              text: _checkBox.displayText
              verticalAlignment: Text.AlignVCenter
              color: Cpp_ThemeManager.colors["table_text"]
              leftPadding: root.rtl ? _checkBox.indicator.width : 6
              rightPadding: root.rtl ? 6 : _checkBox.indicator.width
              horizontalAlignment: Text.AlignLeft
            }
          }
        }

        //
        // Button row: emits a setData click sentinel so the C++ side can route to a signal
        //
        Loader {
          id: buttonLoader

          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          active: model.widgetType === ProjectEditor.Button
          visible: model.widgetType === ProjectEditor.Button

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: Button {
            id: _btn

            enabled: buttonLoader.modelActive
            font: Cpp_Misc_CommonFonts.boldUiFont
            text: buttonLoader.modelPlaceholder ?? ""
            opacity: buttonLoader.modelActive ? 1 : 0.5

            onClicked: {
              // Monotonic stamp forces the dispatcher to fire even on repeat presses.
              view.model.setData(
                    view.index(buttonLoader.modelRow, buttonLoader.modelColumn),
                    Date.now(),
                    ProjectEditor.EditableValue)
            }
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

          property int modelRow: row
          property int modelColumn: column
          property var modelActive: model.active
          property var editableValue: model.editableValue
          property var modelPlaceholder: model.placeholderValue

          sourceComponent: TextField {
            id: _hexComponent

            enabled: hexFieldLoader.modelActive
            opacity: hexFieldLoader.modelActive ? 1 : 0.5
            placeholderText: hexFieldLoader.modelPlaceholder ?? ""

              font: Cpp_Misc_CommonFonts.monoFont
              color: Cpp_ThemeManager.colors["table_text"]
              horizontalAlignment: TextInput.AlignLeft

              //
              // Sync from model only when unfocused (onTextEdited reassigns
              // text imperatively, so a declarative text binding cannot refresh)
              //
              property var modelValue: hexFieldLoader.editableValue
              function refreshFromModel() {
                const formattedText = Cpp_Console_Handler.formatUserHex(
                                        _hexComponent.modelValue ?? "");
                const isValid = Cpp_Console_Handler.validateUserHex(formattedText);

                _hexComponent.text = formattedText;
                _hexComponent.color = isValid
                    ? Cpp_ThemeManager.colors["table_text"]
                    : Cpp_ThemeManager.colors["alarm"]
              }

              Component.onCompleted: refreshFromModel()
              onModelValueChanged: {
                if (!_hexComponent.activeFocus)
                  refreshFromModel();
              }

              //
              // Add space automatically in hex view
              //
              onTextEdited: {
                const currentCursorPosition = _hexComponent.cursorPosition;
                const cursorAtEnd = (currentCursorPosition === _hexComponent.text.length);

                const originalText = _hexComponent.text;
                const formattedText = Cpp_Console_Handler.formatUserHex(_hexComponent.text);
                const isValid = Cpp_Console_Handler.validateUserHex(formattedText);

                if (originalText !== formattedText) {
                  _hexComponent.text = formattedText;

                  //
                  // Restore the cursor position, adjusting for added spaces
                  //
                  if (!cursorAtEnd) {
                    //
                    // Remove spaces from originalText and formattedText to compare lengths
                    //
                    const cleanedOriginalText = originalText.replace(/ /g, '');
                    const cleanedFormattedText = formattedText.replace(/ /g, '');

                    //
                    // Calculate the difference in length due to formatting
                    //
                    const lengthDifference = cleanedFormattedText.length - cleanedOriginalText.length;

                    //
                    // Count spaces before the cursor in both texts
                    //
                    let spacesBeforeCursorOriginal = (originalText.slice(0, currentCursorPosition).match(/ /g) || []).length;
                    let spacesBeforeCursorFormatted = (formattedText.slice(0, currentCursorPosition).match(/ /g) || []).length;

                    //
                    // Calculate adjustment factor
                    //
                    const adjustment = spacesBeforeCursorFormatted - spacesBeforeCursorOriginal + lengthDifference;

                    //
                    // Restore the cursor position with adjustment
                    //
                    _hexComponent.cursorPosition = Math.min(currentCursorPosition + adjustment, _hexComponent.text.length);
                  }
                }

                _hexComponent.color = isValid
                    ? Cpp_ThemeManager.colors["table_text"]
                    : Cpp_ThemeManager.colors["alarm"]

                // Set model data
                view.model.setData(
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
          implicitWidth: 6
          visible: model.widgetType !== ProjectEditor.SectionHeader && !item.isNav
        }
      }

      //
      // Full-row click target for navigation rows (e.g. dataset links)
      //
      MouseArea {
        z: 2
        enabled: item.isNav
        visible: item.isNav
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
          view.model.setData(
                view.index(item.row, item.column),
                Date.now(),
                ProjectEditor.EditableValue)
        }
      }
    }
  }

  //
  // Filtered-empty state: without it a zero-match search leaves a blank pane
  //
  Label {
    opacity: 0.5
    Layout.topMargin: 16
    Layout.fillWidth: true
    wrapMode: Text.WordWrap
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["text"]
    text: qsTr("No options match your search.")
    visible: root.searchable && view.rows === 0 && filterProxy.query.trim().length > 0
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
