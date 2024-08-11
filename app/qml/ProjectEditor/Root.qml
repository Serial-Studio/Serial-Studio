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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "Sections"
import "../ProjectEditor"
import "../Widgets" as Widgets

Window {
  id: root

  //
  // Window options
  //
  minimumWidth: 910
  minimumHeight: 720
  title: qsTr("%1 - Project Editor").arg(Cpp_Project_Model.jsonFileName)

  //
  // Ensure that current JSON file is shown
  //
  Component.onCompleted: Cpp_Project_Model.openJsonFile(Cpp_JSON_Generator.jsonMapFilepath)

  //
  // Ask user to save changes before closing the window
  //
  onClosing: (close) => close.accepted = Cpp_Project_Model.askSave()

  //
  // Dummy string to increase width of buttons
  //
  readonly property string _btSpacer: "  "

  //
  // Save window size
  //
  Settings {
    category: "ProjectEditor"
    property alias windowX: root.x
    property alias windowY: root.y
    property alias windowWidth: root.width
    property alias windowHeight: root.height
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.windowText: Cpp_ThemeManager.colors["text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    //
    // Header (project properties)
    //
    Header {
      id: header
      anchors {
        margins: 0
        top: parent.top
        left: parent.left
        right: parent.right
      }
    }

    //
    // Footer background
    //
    Footer {
      id: footer
      radius: root.radius
      onCloseWindow: root.close()
      onScrollToBottom: groupEditor.selectLastGroup()

      anchors {
        margins: 0
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
    }

    //
    // Window controls
    //
    RowLayout {
      clip: true
      anchors.fill: parent
      spacing: 8
      anchors.topMargin: header.height
      anchors.bottomMargin: footer.height

      //
      // Horizontal spacer
      //
      Item {
        Layout.fillHeight: true
        Layout.minimumWidth: 8
        Layout.maximumWidth: 8
      }

      //
      // JSON structure tree
      //
      /*TreeView {
        id: jsonTree
        Layout.fillHeight: true
        Layout.minimumWidth: 240
        Layout.maximumWidth: 240
        Layout.topMargin: 8 * 2
        Layout.bottomMargin: 8 * 2
        visible: Cpp_Project_Model.groupCount !== 0
      }*/

      //
      // Group editor
      //
      GroupEditor {
        id: groupEditor
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: Cpp_Project_Model.groupCount !== 0
      }

      //
      // Empty project text & icon
      //
      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: Cpp_Project_Model.groupCount === 0

        ColumnLayout {
          spacing: 8
          anchors.centerIn: parent

          Image {
            sourceSize: Qt.size(128, 128)
            Layout.alignment: Qt.AlignHCenter
            source: "qrc:/images/microcontroller.svg"
          }

          Label {
            font.bold: true
            font.pixelSize: 24
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Start Something Awesome")
          }

          Label {
            opacity: 0.8
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Click on the \"Add Group\" Button to Begin")
          }
        }
      }

      //
      // Horizontal spacer
      //
      Item {
        Layout.fillHeight: true
        Layout.minimumWidth: 8
        Layout.maximumWidth: 8
      }
    }
  }
}
