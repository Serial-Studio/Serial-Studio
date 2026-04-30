/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  staysOnTop: true
  title: qsTr("Generating Report")

  //
  // Auto-open / auto-close follows the busy flag
  //
  Connections {
    target: Cpp_Sessions_Manager
    function onPdfExportBusyChanged() {
      if (Cpp_Sessions_Manager.pdfExportBusy) {
        root.show()
        root.raise()
      } else {
        root.close()
      }
    }
  }

  //
  // Body
  //
  dialogContent: ColumnLayout {
    spacing: 14
    implicitWidth: 380

    RowLayout {
      spacing: 12
      Layout.fillWidth: true

      BusyIndicator {
        implicitWidth: 28
        implicitHeight: 28
        running: Cpp_Sessions_Manager.pdfExportBusy
      }

      Label {
        Layout.fillWidth: true
        elide: Text.ElideRight
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["text"]
        text: Cpp_Sessions_Manager.pdfExportStatus.length > 0
                ? Cpp_Sessions_Manager.pdfExportStatus
                : qsTr("Working…")
      }
    }

    ProgressBar {
      to: 1.0
      from: 0.0
      Layout.fillWidth: true
      value: Cpp_Sessions_Manager.pdfExportProgress
    }

    Label {
      opacity: 0.6
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
      text: qsTr("This can take a few seconds for sessions with many "
               + "parameters. The window closes automatically when "
               + "the report is ready.")
    }
  }
}
