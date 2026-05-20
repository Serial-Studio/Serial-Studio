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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Window

/**
 * @brief Drop into any top-level Window to propagate RTL/LTR mirroring from
 *        the active language. Targets the host window's contentItem because
 *        Window itself is not an Item and cannot host attached LayoutMirroring
 *        properties.
 */
Item {
  id: root

  width: 0
  height: 0
  visible: false

  function _apply() {
    const w = Window.window
    if (!w || !w.contentItem)
      return

    if (typeof Cpp_Misc_Translator === "undefined")
      return

    w.contentItem.LayoutMirroring.enabled = Cpp_Misc_Translator.rtl
    w.contentItem.LayoutMirroring.childrenInherit = true
  }

  Component.onCompleted: Qt.callLater(_apply)
  onParentChanged: Qt.callLater(_apply)

  Connections {
    target: (typeof Cpp_Misc_Translator !== "undefined") ? Cpp_Misc_Translator
                                                         : null
    function onLanguageChanged() { root._apply() }
  }
}
