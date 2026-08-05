/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Controls

ComboBox {
  id: control

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  LayoutMirroring.enabled: false
  LayoutMirroring.childrenInherit: true

  contentItem: Text {
    font: control.font
    elide: Text.ElideRight
    text: control.displayText
    color: control.palette.buttonText
    verticalAlignment: Text.AlignVCenter
    leftPadding: control.rtl ? control.indicator.width + 4 : 6
    rightPadding: control.rtl ? 6 : control.indicator.width + 4
    horizontalAlignment: control.rtl ? Text.AlignRight : Text.AlignLeft
  }

  //
  // Show the full item text on hover (long device names elide in the popup)
  //
  delegate: ItemDelegate {
    required property var model
    required property int index

    readonly property bool isCurrent: control.currentIndex === index
    readonly property string label: control.textRole === ""
                                    ? String(model.modelData ?? "")
                                    : String(model[control.textRole] ?? "")

    text: label
    hoverEnabled: true
    highlighted: control.highlightedIndex === index
    width: ListView.view ? ListView.view.width : control.width
    font: isCurrent ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont

    ToolTip.delay: 400
    ToolTip.text: label
    ToolTip.visible: hovered
  }
}
