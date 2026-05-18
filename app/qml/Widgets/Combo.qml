/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
}
