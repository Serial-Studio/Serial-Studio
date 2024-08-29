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
  Component.onCompleted: {
    Cpp_NativeWindow.addWindow(root)
    Cpp_Project_Model.openJsonFile(Cpp_JSON_Generator.jsonMapFilepath)
  }

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

    ColumnLayout {
      id: layout
      spacing: 0
      anchors.fill: parent

      //
      // Toolbar
      //
      Toolbar {
        z: 2
        Layout.fillWidth: true
      }

      //
      // Header (project properties)
      //
      Header {
        Layout.topMargin: -1
        Layout.fillWidth: true
      }

      //
      // Main Layout (project structure + group editor)
      //
      RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: Cpp_Project_Model.groupCount !== 0

        TreeView {
          Layout.fillHeight: true
          Layout.minimumWidth: 144
        }

        Widgets.Pane {
          Layout.fillWidth: true
          Layout.fillHeight: true
          title: qsTr("Group Editor")
          icon: "qrc:/rcc/icons/project-editor/windows/group.svg"

          GroupEditor {
            anchors.fill: parent
            anchors.topMargin: -6
          }
        }
      }

      //
      // Empty project text & icon
      //
      Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: Cpp_Project_Model.groupCount === 0
        color: Cpp_ThemeManager.colors["alternate_window"]

        Rectangle {
          height: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
          anchors {
            top: parent.top
            left: parent.left
            right: parent.right
          }
        }

        ColumnLayout {
          spacing: 8
          anchors.centerIn: parent

          Image {
            sourceSize: Qt.size(128, 128)
            Layout.alignment: Qt.AlignHCenter
            source: "qrc:/rcc/images/microcontroller.svg"
          }

          Label {
            font.bold: true
            font.pixelSize: 24
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Start Something Awesome")
            color: Cpp_ThemeManager.colors["text"]
          }

          Label {
            opacity: 0.8
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Click on the \"Add Group\" Button to Begin")
          }
        }
      }
    }
  }
}
