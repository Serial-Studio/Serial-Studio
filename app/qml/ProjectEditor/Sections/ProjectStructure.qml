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
  title: qsTr("Project Structure")
  icon: "qrc:/rcc/icons/project-editor/windows/project-structure.svg"


  TreeView {
    id: treeView

    clip: true
    model: Cpp_Project_Model.treeModel

    //
    // Always expand the model items
    //
    onModelChanged: {
      for (let i = 0; i < model.rowCount(); i++) {
        let index = model.index(i, 0)
        if (!treeView.isExpanded(index))
          treeView.expand(index)
      }
    }

    //
    // Set widget anchors
    //
    anchors {
      fill: parent
      topMargin: -16
      leftMargin: -9
      rightMargin: -9
      bottomMargin: -9
    }

    //
    // Set background item
    //
    delegate: Item {
      id: item
      implicitHeight: 18
      implicitWidth: treeView.width

      required property int row
      required property int depth
      required property int column
      required property bool current
      required property bool expanded
      required property bool isTreeNode
      required property int hasChildren
      required property TreeView treeView

      readonly property real padding: 4
      readonly property real indentation: 32

      function toggleExpanded() {
        let index = treeView.index(row, column)
        treeView.toggleExpanded(index)
      }

      function onLabelClicked() {
        if (isTreeNode) {
          let index = treeView.index(row, column)
          treeView.toggleExpanded(index)
        }

        else
          current = true
      }

      Rectangle {
        id: background
        anchors.fill: parent
        color: current ? Cpp_ThemeManager.colors["highlighted"] : "transparent"
      }

      RowLayout {
        spacing: 0
        anchors.fill: parent
        anchors.rightMargin: 8
        anchors.leftMargin: padding + (isTreeNode ? depth * indentation : 0)

        Image {
          id: indicator
          sourceSize: Qt.size(8, 8)
          rotation: expanded ? 0 : 270
          Layout.alignment: Qt.AlignVCenter
          visible: isTreeNode && hasChildren
          source: "qrc:/rcc/icons/project-editor/treeview/indicator.svg"

          TapHandler {
            onSingleTapped: item.toggleExpanded()
          }
        }

        Item {
          width: 6
          visible: indicator.visible
        }

        Image {
          id: icon
          source: model.decoration
          sourceSize: Qt.size(12, 12)
          Layout.alignment: Qt.AlignVCenter

          MouseArea {
            onDoubleClicked: item.onLabelClicked()
          }
        }

        Item {
          width: 4
        }

        Label {
          id: label
          text: model.display
          Layout.fillWidth: true
          elide: Label.ElideRight
          Layout.alignment: Qt.AlignVCenter
          color: Cpp_ThemeManager.colors["text"]
          font: depth === 0 ? Cpp_Misc_CommonFonts.boldUiFont :
                              Cpp_Misc_CommonFonts.uiFont

          MouseArea {
            onDoubleClicked: item.onLabelClicked()
          }
        }

        Label {
          opacity: 0.7
          id: frameIndex
          visible: model.whatsThis >= 1
          Layout.alignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.monoFont
          color: Cpp_ThemeManager.colors["text"]
          text: "[" + qsTr("IDX → %1").arg(model.whatsThis) + "]"

          MouseArea {
            onDoubleClicked: item.onLabelClicked()
          }
        }
      }
    }
  }
}
