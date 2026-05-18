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

TextField {
  id: control

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  LayoutMirroring.enabled: false
  LayoutMirroring.childrenInherit: true

  horizontalAlignment: control.rtl ? TextInput.AlignRight : TextInput.AlignLeft
}
