/*
 * Copyright (c) 2021-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
