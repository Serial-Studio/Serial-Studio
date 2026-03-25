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
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  property var accelModel: null

  staysOnTop: true
  title: qsTr("Accelerometer Configuration")

  function openDialog(model) {
    root.accelModel = model

    if (model) {
      maxGField.text = model.maxG.toFixed(1)
      inputInGSwitch.checked = model.inputInG
    }

    root.show()
    root.raise()
  }

  function applyMaxG() {
    if (!accelModel)
      return

    const val = parseFloat(maxGField.text)
    if (!isNaN(val) && val >= 0.5)
      accelModel.maxG = val
    else
      maxGField.text = accelModel.maxG.toFixed(1)
  }

  dialogContent: ColumnLayout {
    spacing: 4

    Label {
      opacity: 0.7
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      Layout.minimumWidth: 300
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9)
      text: qsTr("Configure the accelerometer display range and input units.")
    }

    Item {
      implicitHeight: 4
    }

    Label {
      text: qsTr("Display Range")
      font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      GridLayout {
        columns: 2
        rowSpacing: 4
        columnSpacing: 8
        anchors.fill: parent

        Label {
          text: qsTr("Max G:")
          color: Cpp_ThemeManager.colors["text"]
        }

        TextField {
          id: maxGField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 140
          validator: DoubleValidator {
            bottom: 0.5
          }
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Enter max G value")
          onTextChanged: {
            if (activeFocus && acceptableInput)
              Qt.callLater(root.applyMaxG)
          }
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    Label {
      text: qsTr("Input Configuration")
      font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      RowLayout {
        spacing: 8
        anchors.fill: parent

        Label {
          Layout.fillWidth: true
          text: qsTr("Input already in G")
          color: Cpp_ThemeManager.colors["text"]
        }

        Switch {
          id: inputInGSwitch

          onToggled: {
            if (root.accelModel)
              root.accelModel.inputInG = checked
          }
        }
      }
    }

    Item {
      implicitHeight: 8
    }

    RowLayout {
      spacing: 8
      Layout.alignment: Qt.AlignRight

      Item {
        Layout.fillWidth: true
      }

      Button {
        icon.width: 18
        icon.height: 18
        text: qsTr("Close")
        onClicked: root.close()
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/close.svg"
      }
    }
  }
}
