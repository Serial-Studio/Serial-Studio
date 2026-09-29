/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  minimumWidth: 480
  preferredWidth: column.implicitWidth
  title: qsTr("Assistant — Pro feature")
  preferredHeight: column.implicitHeight

  //
  // Layout
  //
  dialogContent: ColumnLayout {
    id: column

    spacing: 12
    anchors.centerIn: parent

    Label {
      Layout.fillWidth: true
      Layout.preferredWidth: 440
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("The Assistant is a Serial Studio Pro feature. "
                + "Activate your license to unlock it.")
    }

    DialogButtonBox {
      Layout.fillWidth: true

      Widgets.PushButton {
        text: qsTr("Activate")
        font: Cpp_Misc_CommonFonts.uiFont
        DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        onClicked: {
          if (typeof Cpp_CommercialBuild !== "undefined" && Cpp_CommercialBuild)
            app.showLicenseDialog()
          else
            Qt.openUrlExternally("https://serial-studio.com/")

          root.close()
        }
      }

      Widgets.PushButton {
        text: qsTr("Close")
        font: Cpp_Misc_CommonFonts.uiFont
        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        onClicked: root.close()
      }
    }
  }
}
