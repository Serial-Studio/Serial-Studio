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

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  title: qsTr("Project Structure")
  icon: "qrc:/rcc/icons/project-editor/windows/project-structure.svg"

  signal groupClicked(var title)
  signal datasetClicked(var title)

  //
  // Put tree view inside a scroll view
  //
  ScrollView {
    id: scrollView
    contentWidth: -1
    anchors {
      fill: parent
      topMargin: -16
      leftMargin: -9
      rightMargin: -9
      bottomMargin: -9
    }

    TreeView {
      id: treeView
      focus: true
      reuseItems: false
      model: Cpp_JSON_ProjectModel.treeModel
      boundsBehavior: TreeView.StopAtBounds
      selectionModel: Cpp_JSON_ProjectModel.selectionModel

      //
      // Keyboard navigation
      //
      Keys.onPressed: (event) => {
                        // Move down to the next sibling (or parent if collapsed)
                        if (event.key === Qt.Key_Down) {
                          let nextIndex = treeView.index(treeView.currentRow + 1, treeView.currentColumn)
                          if (nextIndex.isValid)
                          treeView.selectionModel.setCurrentIndex(nextIndex, ItemSelectionModel.ClearAndSelect)
                        }

                        // Move up to the previous sibling (or parent)
                        else if (event.key === Qt.Key_Up) {
                          let prevIndex = treeView.index(treeView.currentRow - 1, treeView.currentColumn)
                          if (prevIndex.isValid)
                          treeView.selectionModel.setCurrentIndex(prevIndex, ItemSelectionModel.ClearAndSelect)
                        }

                        // Delete current item
                        else if (event.key === Qt.Key_Delete) {
                          if (Cpp_JSON_ProjectModel.currentView === ProjectModel.DatasetView)
                          Cpp_JSON_ProjectModel.deleteCurrentDataset()
                          else if (Cpp_JSON_ProjectModel.currentView === ProjectModel.GroupView)
                          Cpp_JSON_ProjectModel.deleteCurrentGroup()
                        }
                      }

      //
      // Set background item
      //
      delegate: Item {
        id: item
        implicitWidth: treeView.width
        implicitHeight: depth === 0 ? 30 : 18
        Component.onCompleted: syncExpandedState()

        required property int row
        required property int depth
        required property int column
        required property bool current
        required property bool expanded
        required property bool isTreeNode
        required property bool hasChildren
        required property TreeView treeView

        readonly property real padding: 4
        readonly property real indentation: 16

        //
        // Restore expanded state from C++ model
        //
        function syncExpandedState() {
          if (model.treeViewExpanded === true)
            treeView.expand(row)
          else
            treeView.collapse(row)
        }

        //
        // Show/hide the children of the current item.
        //
        function toggleExpanded() {
          if (hasChildren) {
            treeView.toggleExpanded(row)
            model.treeViewExpanded = expanded
            return true
          }

          return false
        }

        //
        // Select the item and open the associated view automatically.
        //
        function onLabelClicked() {
          treeView.forceActiveFocus()
          let index = treeView.index(row, column)
          treeView.selectionModel.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect)
        }

        //
        // If item has children, expand on double click.
        // Otherwise, select the item and open the associated view.
        //
        function onLabelDoubleClicked() {
          treeView.forceActiveFocus()
          if (!toggleExpanded()) {
            onLabelClicked()
          }
        }

        //
        // Item background
        //
        Rectangle {
          id: background
          anchors.fill: parent
          color: current ? Cpp_ThemeManager.colors["highlight"] : "transparent"

          MouseArea {
            anchors.fill: parent
            onClicked: onLabelClicked()
            onDoubleClicked: onLabelDoubleClicked()
          }
        }

        //
        // Item controls
        //
        RowLayout {
          spacing: 0
          anchors.fill: parent
          anchors.rightMargin: 16
          anchors.leftMargin: padding + (isTreeNode ? depth * indentation : 0)

          //
          // Expanded indicator
          //
          Image {
            id: indicator
            enabled: hasChildren
            sourceSize: Qt.size(8, 8)
            opacity: hasChildren ? 1 : 0
            Layout.alignment: Qt.AlignVCenter
            rotation: model.treeViewExpanded ? 0 : 270
            source: "qrc:/rcc/icons/project-editor/treeview/indicator.svg"

            MouseArea {
              anchors.fill: parent
              onClicked: toggleExpanded()
            }
          }

          //
          // Spacer
          //
          Item {
            width: 6
          }

          //
          // Item icon
          //
          Image {
            id: icon
            source: model.treeViewIcon
            sourceSize: Qt.size(12, 12)
            Layout.alignment: Qt.AlignVCenter
          }

          //
          // Spacer
          //
          Item {
            width: 4
          }

          //
          // Item text
          //
          Label {
            id: label
            Layout.fillWidth: true
            elide: Label.ElideRight
            text: model.treeViewText
            Layout.alignment: Qt.AlignVCenter
            font: depth === 0 ? Cpp_Misc_CommonFonts.boldUiFont :
                                Cpp_Misc_CommonFonts.uiFont
            color: current ? Cpp_ThemeManager.colors["highlighted_text"] :
                             Cpp_ThemeManager.colors["text"]
          }

          //
          // Frame index indicator (only for datasets)
          //
          Label {
            opacity: 0.7
            id: frameIndex
            visible: depth > 1
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.monoFont
            text: "[" + qsTr("IDX %1").arg(model.treeViewFrameIndex) + "]"
            color: current ? Cpp_ThemeManager.colors["highlighted_text"] :
                             Cpp_ThemeManager.colors["text"]
          }
        }
      }
    }
  }
}
