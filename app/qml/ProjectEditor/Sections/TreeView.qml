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

import "../Widgets" as Widgets

Widgets.Window {
  id: root

  //
  // Window properties
  //
  gradient: true
  headerDoubleClickEnabled: false
  title: qsTr("JSON Project Tree")
  icon.source: "qrc:/icons/json.svg"

  //
  // Connections with JSON editor model
  //
  Connections {
    target: Cpp_Project_Model

    function onGroupCountChanged() {
      view.model = 0
      view.model = Cpp_Project_Model.groupCount
    }

    function onGroupOrderChanged() {
      view.model = 0
      view.model = Cpp_Project_Model.groupCount
    }

    function onDatasetChanged() {
      view.model = 0
      view.model = Cpp_Project_Model.groupCount
    }
  }

  //
  // List view
  //
  ListView {
    id: view
    anchors.fill: parent
    spacing: 8 * 2
    anchors.margins: 8
    model: Cpp_Project_Model.groupCount

    delegate: Loader {
      width: view.width - 8 * 2
      height: Cpp_Project_Model.datasetCount(index) * 24 + 24

      sourceComponent: ColumnLayout {
        id: groupDelegate
        spacing: 8

        readonly property var groupId: index

        RowLayout {
          spacing: 8
          Layout.fillWidth: true

          Widgets.Icon {
            width: 18
            height: 18
            color: Cpp_ThemeManager.text
            source: "qrc:/icons/group.svg"
            Layout.alignment: Qt.AlignVCenter
          }

          Label {
            font.bold: true
            Layout.fillWidth: true
            elide: Label.ElideRight
            Layout.alignment: Qt.AlignVCenter
            text: Cpp_Project_Model.groupTitle(groupDelegate.groupId)
          }

          Label {
            opacity: 0.5
            visible: text !== "[]"
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.monoFont
            text: "[" + Cpp_Project_Model.groupWidget(groupDelegate.groupId) + "]"
          }
        }

        Repeater {
          model: Cpp_Project_Model.datasetCount(groupDelegate.groupId)

          delegate: Loader {
            Layout.fillWidth: true
            sourceComponent: RowLayout {
              spacing: 8

              Item {
                width: 2 * 8
              }

              Widgets.Icon {
                width: 18
                height: 18
                color: Cpp_ThemeManager.text
                source: "qrc:/icons/dataset.svg"
                Layout.alignment: Qt.AlignVCenter
              }

              Label {
                id: label
                Layout.fillWidth: true
                elide: Label.ElideRight
                Layout.alignment: Qt.AlignVCenter
                text: Cpp_Project_Model.datasetTitle(groupDelegate.groupId, index)

                MouseArea {
                  anchors.fill: parent
                  cursorShape: Qt.PointingHandCursor
                }
              }

              Label {
                opacity: 0.5
                visible: text !== "[]"
                Layout.alignment: Qt.AlignVCenter
                font: Cpp_Misc_CommonFonts.monoFont
                text: "[" + Cpp_Project_Model.datasetWidget(groupDelegate.groupId, index) + "]"
              }
            }
          }
        }
      }
    }
  }
}
