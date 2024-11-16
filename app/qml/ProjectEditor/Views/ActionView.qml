/*
 * Copyright (c) 2020-2024 Alex Spataru <https://github.com/alex-spataru>
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
  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_JSON_ProjectModel.selectedIcon
  title: Cpp_JSON_ProjectModel.selectedText

  //
  // User interface elements
  //
  Page {
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    anchors {
      fill: parent
      topMargin: -16
      leftMargin: -9
      rightMargin: -9
      bottomMargin: -10
    }

    //
    // Dataset actions panel
    //
    Rectangle {
      z: 2
      id: header
      height: layout.implicitHeight + 12
      color: Cpp_ThemeManager.colors["groupbox_background"]
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      //
      // Buttons
      //
      RowLayout {
        id: layout
        spacing: 4

        anchors {
          margins: 8
          left: parent.left
          right: parent.right
          verticalCenter: parent.verticalCenter
        }

        //
        // Change icon
        //
        Widgets.BigButton {
          iconSize: 24
          text: qsTr("Change Icon")
          Layout.alignment: Qt.AlignVCenter
          palette.buttonText: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/actions/" + Cpp_JSON_ProjectModel.actionIcon + ".svg"
          onClicked: {
            actionIconPicker.selectedIcon = Cpp_JSON_ProjectModel.actionIcon
            actionIconPicker.showNormal()
          }
        }

        //
        // Spacer
        //
        Item {
          Layout.fillWidth: true
        }

        //
        // Duplicate action
        //
        Widgets.BigButton {
          iconSize: 24
          text: qsTr("Duplicate")
          Layout.alignment: Qt.AlignVCenter
          onClicked: Cpp_JSON_ProjectModel.duplicateCurrentAction()
          palette.buttonText: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
        }

        //
        // Delete action
        //
        Widgets.BigButton {
          iconSize: 24
          text: qsTr("Delete")
          Layout.alignment: Qt.AlignVCenter
          onClicked: Cpp_JSON_ProjectModel.deleteCurrentAction()
          palette.buttonText: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/project-editor/actions/delete.svg"
        }
      }

      //
      // Bottom border
      //
      Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }
    }

    //
    // Dataset model editor
    //
    TableDelegate {
      id: delegate
      anchors.fill: parent
      anchors.topMargin: header.height
      modelPointer: Cpp_JSON_ProjectModel.actionModel

      footerItem: ColumnLayout {
        spacing: 0

        Image {
          sourceSize: Qt.size(128, 128)
          Layout.alignment: Qt.AlignHCenter
          source: "qrc:/rcc/images/action.svg"
        }

        Item {
          implicitHeight: 16
        }

        Label {
          Layout.alignment: Qt.AlignHCenter
          text: qsTr("Send commands with a button click")
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
          text: qsTr("Actions let you send custom commands to the connected device with a single button press.")
        }
      }
    }
  }
}
