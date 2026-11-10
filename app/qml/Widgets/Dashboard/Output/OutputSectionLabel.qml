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

//
// Uppercase caption plus separator, the heading every output control wears. Extracted from the
// panel so the dashboard and the transmit-function preview cannot drift apart again.
//
ColumnLayout {
  id: root

  property real textScale: 1
  property string text: ""
  property color labelColor: Cpp_ThemeManager.colors["pane_section_label"]

  spacing: 2

  Label {
    text: root.text
    elide: Text.ElideRight
    color: root.labelColor
    Layout.fillWidth: true
    Component.onCompleted: font.capitalization = Font.AllUppercase
    font: Cpp_Misc_CommonFonts.customUiFont(0.75 * root.textScale, true)
  }

  Rectangle {
    implicitHeight: 1
    Layout.fillWidth: true
    color: Cpp_ThemeManager.colors["groupbox_border"]
  }
}
