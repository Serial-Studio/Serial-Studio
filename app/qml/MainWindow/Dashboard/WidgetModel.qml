/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick

Repeater {
  id: root

  property real cellWidth: 0
  property real cellHeight: 0

  delegate: Loader {
    id: loader
    asynchronous: true
    width: root.cellWidth
    height: root.cellHeight
    readonly property bool widgetInViewPort: opacity > 0

    // Uncomment to verify that lazy widget rendering is working
    //Behavior on opacity {NumberAnimation{}}

    sourceComponent: WidgetDelegate {
      widgetIndex: index
      active: loader.visible && loader.widgetInViewPort
    }

    Connections {
      target: Cpp_UI_Dashboard

      function onWidgetVisibilityChanged() {
        const widget = Cpp_UI_Dashboard.widgetType(index)
        const idx = Cpp_UI_Dashboard.relativeIndex(index)
        loader.visible = Cpp_UI_Dashboard.widgetVisible(widget, idx)
      }
    }
  }
}
