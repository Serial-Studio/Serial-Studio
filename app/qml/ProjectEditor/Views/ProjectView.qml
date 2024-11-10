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
  icon: Cpp_JSON_ProjectModel.selectedIcon
  title: Cpp_JSON_ProjectModel.selectedText
  Component.onCompleted: Cpp_JSON_ProjectModel.buildProjectModel()

  TableDelegate {
    id: delegate
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -10
    anchors.rightMargin: -10
    anchors.bottomMargin: -9
    modelPointer: Cpp_JSON_ProjectModel.projectModel

    footerItem: ColumnLayout {
      spacing: 0

      Image {
        sourceSize: Qt.size(128, 128)
        Layout.alignment: Qt.AlignHCenter
        source: "qrc:/rcc/images/soldering-iron.svg"
      }

      Item {
        implicitHeight: 16
      }

      Label {
        Layout.alignment: Qt.AlignHCenter
        text: qsTr("Start Building Now!")
        horizontalAlignment: Label.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(2, true)
      }

      Item {
        implicitHeight: 8
      }

      Label {
        opacity: 0.8
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Label.AlignHCenter
        Layout.maximumWidth: delegate.width * 0.9
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        font: Cpp_Misc_CommonFonts.customUiFont(1.5, false)
        text: qsTr("Get started by adding a group with the toolbar buttons above.")
      }
    }
  }
}
