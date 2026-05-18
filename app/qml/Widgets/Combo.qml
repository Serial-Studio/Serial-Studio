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

  readonly property bool _rtl: Cpp_Misc_Translator.rtl
  readonly property int _hAlign: _rtl ? Text.AlignRight : Text.AlignLeft

  Binding {
    target: control.contentItem
    property: "horizontalAlignment"
    value: control._hAlign
    restoreMode: Binding.RestoreNone
    when: control.contentItem !== null
          && control.contentItem.hasOwnProperty("horizontalAlignment")
  }
}
