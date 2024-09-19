/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_Project_Model.selectedIcon
  title: Cpp_Project_Model.selectedText
  Component.onCompleted: Cpp_Project_Model.buildProjectModel()

  TableDelegate {
    id: delegate
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -10
    anchors.rightMargin: -10
    anchors.bottomMargin: -9
    modelPointer: Cpp_Project_Model.projectModel

    footerItem: Image {
      readonly property real aspectRatio: 696 / 280
      readonly property real idealWidth: delegate.width * 0.8
      readonly property real idealHeight: delegate.height - delegate.tableHeight
      readonly property real maxWidth: idealHeight * aspectRatio
      readonly property real maxHeight: idealWidth / aspectRatio
      readonly property real actualWidth: Math.min(idealWidth, maxWidth)
      readonly property real actualHeight: Math.min(idealHeight, maxHeight)

      smooth: true
      antialiasing: true
      source: "qrc:/rcc/diagrams/project-dfd.svg"
      sourceSize: Qt.size(actualWidth, actualHeight)
    }
  }
}
