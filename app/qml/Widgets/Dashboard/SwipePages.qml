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
import QtQuick.Controls

//
// Page dots and per-widget swipe-page persistence (spec 0075 G5); the "page" key under the widget's
// own id is unchanged, so a dashboard laid out by an older build reopens on the page it was left on.
//
Item {
  id: root

  required property var view
  required property string widgetId

  anchors.bottomMargin: 4
  anchors.bottom: parent.bottom
  implicitWidth: dots.implicitWidth
  implicitHeight: dots.implicitHeight
  anchors.horizontalCenter: parent.horizontalCenter

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
    const page = Number(s["page"])
    if (isFinite(page))
      root.view.currentIndex = Math.max(0, Math.min(root.view.count - 1, Math.floor(page)))

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

  PageIndicator {
    id: dots

    interactive: true
    count: root.view.count
    currentIndex: root.view.currentIndex

    delegate: Rectangle {
      required property int index
      implicitWidth: 8
      implicitHeight: 8
      radius: width / 2
      antialiasing: true
      color: Cpp_ThemeManager.colors["widget_text"]
      opacity: index === dots.currentIndex ? 0.95 : 0.40
      Behavior on opacity { NumberAnimation { duration: 120 } }
    }

    onCurrentIndexChanged: {
      if (root.view.currentIndex !== currentIndex)
        root.view.currentIndex = currentIndex
    }
  }
}
