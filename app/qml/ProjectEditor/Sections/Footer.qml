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

Rectangle {
  id: root
  color: Cpp_ThemeManager.toolbarGradient2
  height: footer.implicitHeight + 32

  //
  // Signals
  //
  signal closeWindow()
  signal scrollToBottom()

  //
  // Radius compensator
  //
  Rectangle {
    color: root.color
    height: root.radius
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  //
  // Top border
  //
  Rectangle {
    height: 1
    color: Cpp_ThemeManager.toolbarGradient1
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  //
  // Dialog buttons
  //
  RowLayout {
    id: footer
    spacing: 8

    anchors {
      left: parent.left
      right: parent.right
      margins: 8 * 2
      verticalCenter: parent.verticalCenter
    }

    Button {
      icon.width: 24
      icon.height: 24
      onClicked: root.closeWindow()
      text: qsTr("Close") + _btSpacer
      icon.source: "qrc:/icons/close.svg"
      icon.color: Cpp_ThemeManager.menubarText
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
    }

    Item {
      Layout.fillWidth: true
    }

    Button {
      id: addGrp
      icon.width: 24
      icon.height: 24
      highlighted: true
      Layout.fillWidth: true
      text: qsTr("Add group")
      icon.source: "qrc:/icons/add.svg"
      icon.color: Cpp_ThemeManager.menubarText
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
      onClicked: {
        Cpp_Project_Model.addGroup()
        root.scrollToBottom()
      }
    }

    Button {
      icon.width: 24
      icon.height: 24
      Layout.fillWidth: true
      icon.source: "qrc:/icons/code.svg"
      text: qsTr("Customize frame parser")
      icon.color: Cpp_ThemeManager.menubarText
      onClicked: Cpp_Project_CodeEditor.displayWindow()
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
    }

    Item {
      Layout.fillWidth: true
    }

    Button {
      icon.width: 24
      icon.height: 24
      icon.source: "qrc:/icons/open.svg"
      icon.color: Cpp_ThemeManager.menubarText
      onClicked: Cpp_Project_Model.openJsonFile()
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
      text: qsTr("Open existing project...") + _btSpacer
    }

    Button {
      icon.width: 24
      icon.height: 24
      icon.source: "qrc:/icons/new.svg"
      icon.color: Cpp_ThemeManager.menubarText
      onClicked: Cpp_Project_Model.newJsonFile()
      text: qsTr("Create new project") + _btSpacer
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
    }

    Button {
      icon.width: 24
      icon.height: 24
      opacity: enabled ? 1: 0.5
      enabled: Cpp_Project_Model.modified
      icon.source: "qrc:/icons/apply.svg"
      icon.color: Cpp_ThemeManager.menubarText
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
      text: (Cpp_Project_Model.jsonFilePath.length > 0 ? qsTr("Apply") : qsTr("Save")) + _btSpacer

      onClicked: {
        if (Cpp_Project_Model.saveJsonFile())
          root.closeWindow()
      }
    }
  }
}
