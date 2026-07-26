/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
import QtQuick.Controls

import "../" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window properties
  //
  staysOnTop: true
  title: qsTr("Widget Settings")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Declarations rendered, and the widget id ("type:groupId:datasetIndex") they persist under
  //
  property string packageId: ""
  property string settingsId: ""
  property var declarations: []
  property var values: ({})

  //
  // Opens the form for one widget instance
  //
  function openDialog(packageId, widgetId, widgetTitle) {
    root.packageId = packageId
    root.settingsId = widgetId
    root.declarations = Cpp_UI_WidgetExtensions.configProperties(packageId)
    root.reload()

    if (widgetTitle && widgetTitle.length > 0)
      root.title = qsTr("%1 Settings").arg(widgetTitle)

    root.show()
    root.raise()
  }

  //
  // Package defaults overlaid with the values stored in the project
  //
  function reload() {
    const stored = Cpp_JSON_ProjectModel.widgetSettings(root.settingsId)
    var merged = ({})
    for (var i = 0; i < root.declarations.length; ++i) {
      const key = root.declarations[i]["id"]
      merged[key] = stored[key] !== undefined ? stored[key]
                                              : root.declarations[i]["default"]
    }

    root.values = merged
  }

  //
  // Writes one declared value through the project's per-widget settings store
  //
  function apply(key, value) {
    if (!key || key.length <= 0 || root.settingsId.length <= 0)
      return

    var updated = root.values
    if (updated[key] === value)
      return

    updated[key] = value
    root.values = updated
    Cpp_JSON_ProjectModel.saveWidgetSetting(root.settingsId, key, value)
  }

  //
  // Restores every declared default
  //
  function restoreDefaults() {
    for (var i = 0; i < root.declarations.length; ++i)
      root.apply(root.declarations[i]["id"], root.declarations[i]["default"])

    root.reload()
  }

  //
  // Editors, one per declared property kind
  //
  Component {
    id: boolEditor

    Switch {
      required property var declaration

      checked: root.values[declaration["id"]] === true
      onClicked: root.apply(declaration["id"], checked)
    }
  }

  Component {
    id: numberEditor

    SpinBox {
      id: spin

      required property var declaration

      readonly property bool real: declaration["type"] === "double"
      readonly property int factor: spin.real ? 100 : 1

      editable: true
      stepSize: spin.factor
      from: declaration["min"] !== undefined ? Math.round(declaration["min"] * spin.factor)
                                             : -2147483647
      to: declaration["max"] !== undefined ? Math.round(declaration["max"] * spin.factor)
                                           : 2147483647
      value: Math.round((root.values[declaration["id"]] !== undefined
                         ? root.values[declaration["id"]] : 0) * spin.factor)

      textFromValue: (value) => spin.real ? (value / spin.factor).toFixed(2)
                                          : String(value)
      valueFromText: (text) => Math.round(parseFloat(text) * spin.factor)
      onValueModified: root.apply(declaration["id"], spin.real ? spin.value / spin.factor
                                                               : spin.value)
    }
  }

  Component {
    id: stringEditor

    TextField {
      required property var declaration

      Layout.minimumWidth: 200
      text: root.values[declaration["id"]] !== undefined
            ? String(root.values[declaration["id"]]) : ""
      onEditingFinished: root.apply(declaration["id"], text)
    }
  }

  Component {
    id: choiceEditor

    ComboBox {
      id: combo

      required property var declaration

      Layout.minimumWidth: 200
      model: declaration["options"]
      currentIndex: Math.max(0, declaration["options"].indexOf(
                               String(root.values[declaration["id"]])))
      onActivated: root.apply(declaration["id"], combo.currentText)
    }
  }

  //
  // Dialog contents
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 8

    Label {
      opacity: 0.7
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: 420
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9)
      text: root.declarations.length > 0
            ? qsTr("These settings are declared by the widget package and stored in the project.")
            : qsTr("This widget declares no settings.")
    }

    //
    // One row per declared property; no widget-specific UI code exists anywhere
    //
    Repeater {
      model: root.declarations

      delegate: RowLayout {
        id: row

        required property var modelData

        spacing: 12
        Layout.fillWidth: true

        ColumnLayout {
          spacing: 0
          Layout.fillWidth: true

          Label {
            Layout.fillWidth: true
            text: row.modelData["label"]
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["text"]
          }

          Label {
            opacity: 0.7
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.maximumWidth: 260
            text: row.modelData["description"]
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85)
            visible: row.modelData["description"] !== undefined
                     && row.modelData["description"].length > 0
          }
        }

        Loader {
          Layout.alignment: Qt.AlignVCenter
          property var declaration: row.modelData
          sourceComponent: {
            if (row.modelData["type"] === "bool")
              return boolEditor

            if (row.modelData["type"] === "choice")
              return choiceEditor

            if (row.modelData["type"] === "string")
              return stringEditor

            return numberEditor
          }
        }
      }
    }

    //
    // Actions
    //
    RowLayout {
      spacing: 12
      Layout.topMargin: 4
      Layout.fillWidth: true

      Widgets.IconButton {
        horizontalPadding: 8
        text: qsTr("Restore Defaults")
        onClicked: root.restoreDefaults()
        font: Cpp_Misc_CommonFonts.uiFont
        visible: root.declarations.length > 0
        icon.source: "qrc:/icons/buttons/refresh.svg"
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("Close")
        horizontalPadding: 8
        onClicked: root.close()
        font: Cpp_Misc_CommonFonts.uiFont
        icon.source: "qrc:/icons/buttons/apply.svg"
      }
    }
  }
}
