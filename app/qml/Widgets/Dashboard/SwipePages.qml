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

//
// Per-widget swipe-page persistence (spec 0075 G5); the "page" key under the widget's own id is
// unchanged, so a dashboard laid out by an older build reopens on the page it was left on.
//
Item {
  id: root

  required property var view
  required property string widgetId

  visible: false

  //
  // Suppresses the page auto-save while restore assigns the persisted index
  //
  property bool restoring: false

  //
  // Restore per-widget page from project settings, then persist on change.
  //
  Component.onCompleted: {
    root.restoring = true
    const s = Cpp_JSON_ProjectModel.widgetSettings(root.widgetId)
    if (s["page"] !== undefined)
      root.view.currentIndex = parseInt(s["page"])

    root.restoring = false
  }

  Connections {
    target: root.view
    function onCurrentIndexChanged() {
      if (root.restoring)
        return

      Cpp_JSON_ProjectModel.saveWidgetSetting(
            root.widgetId, "page", root.view.currentIndex)
    }
  }
}
