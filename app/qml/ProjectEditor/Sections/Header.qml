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
  title: qsTr("Project Setup")
  Layout.minimumHeight: layout.implicitHeight + 16
  icon: "qrc:/rcc/icons/project-editor/windows/project-setup.svg"

  //
  // Main layout
  //
  GroupBox {
    id: layout
    anchors.fill: parent
    anchors.topMargin: -6

    background: Rectangle {
      radius: 2
      border.width: 1
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    GridLayout {
      columns: 2
      rowSpacing: 8
      columnSpacing: 8
      anchors.fill: parent

      //
      // Project title
      //
      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Image {
          sourceSize: Qt.size(18, 18)
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/rcc/icons/project-editor/project-title.svg"
        }

        TextField {
          Layout.fillWidth: true
          Layout.minimumWidth: 320
          Layout.maximumHeight: 24
          Layout.minimumHeight: 24
          text: Cpp_Project_Model.title
          onTextChanged: Cpp_Project_Model.setTitle(text)
          placeholderText: qsTr("Title (required)")
        }
      }

      //
      // Separator character
      //
      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Image {
          sourceSize: Qt.size(18, 18)
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/rcc/icons/project-editor/separator.svg"
        }

        TextField {
          Layout.fillWidth: true
          Layout.minimumWidth: 420
          Layout.maximumHeight: 24
          Layout.minimumHeight: 24
          text: Cpp_Project_Model.separator
          onTextChanged: Cpp_Project_Model.setSeparator(text)
          placeholderText: qsTr("Data separator (default is ',')")
        }
      }

      //
      // Start sequence
      //
      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Image {
          sourceSize: Qt.size(18, 18)
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/rcc/icons/project-editor/frame-start.svg"
        }

        TextField {
          Layout.fillWidth: true
          Layout.minimumWidth: 256
          Layout.maximumHeight: 24
          Layout.minimumHeight: 24
          text: Cpp_Project_Model.frameStartSequence
          onTextChanged: Cpp_Project_Model.setFrameStartSequence(text)
          placeholderText: qsTr("Frame start sequence (default is '/*')")
        }
      }

      //
      // End sequence
      //
      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Image {
          sourceSize: Qt.size(18, 18)
          Layout.alignment: Qt.AlignVCenter
          source: "qrc:/rcc/icons/project-editor/frame-end.svg"
        }

        TextField {
          Layout.fillWidth: true
          Layout.minimumWidth: 256
          Layout.maximumHeight: 24
          Layout.minimumHeight: 24
          text: Cpp_Project_Model.frameEndSequence
          onTextChanged: Cpp_Project_Model.setFrameEndSequence(text)
          placeholderText: qsTr("Frame end sequence (default is '*/')")
        }
      }
    }
  }
}
