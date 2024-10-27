/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
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
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  //
  // Widget data inputs
  //
  property color color: "transparent"
  property LEDPanelModel model: LEDPanelModel {}

  //
  // Create scrollable grid view with LED states
  //
  Flickable {
    contentWidth: width
    anchors.fill: parent
    anchors.topMargin: 8
    anchors.leftMargin: 8
    anchors.bottomMargin: 8
    contentHeight: Math.max(grid.implicitHeight, height)

    ScrollBar.vertical: ScrollBar {
      id: scroll
    }

    GridLayout {
      id: grid
      columns: 2
      rowSpacing: 4
      columnSpacing: 4
      width: parent.width - 8
      anchors.centerIn: parent
      anchors.horizontalCenterOffset: -4

      Repeater {
        model: root.model.count
        delegate: Rectangle {
          border.width: 1
          Layout.fillWidth: true
          Layout.minimumHeight: 32
          Layout.maximumHeight: 32
          color: Cpp_ThemeManager.colors["widget_base"]
          border.color: Cpp_ThemeManager.colors["widget_border"]

          RowLayout {
            id: layout
            spacing: 0
            anchors.margins: 4
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            Item {
              Layout.fillWidth: true
            }

            Rectangle {
              id: led
              readonly property color ledColor: root.model.alarms[index] ? Cpp_ThemeManager.colors["alarm"] :
                                                                           root.model.colors[index]

              border.width: 1
              implicitWidth: 18
              implicitHeight: 18
              radius: implicitWidth / 2
              Layout.alignment: Qt.AlignVCenter
              border.color: Cpp_ThemeManager.colors["widget_border"]
              color: root.model.states[index] ? ledColor : Qt.darker(ledColor)

              Behavior on color {ColorAnimation{}}
            }

            Item {
              Layout.fillWidth: true
              Layout.minimumWidth: 4
            }

            Label {
              elide: Qt.ElideRight
              text: root.model.titles[index]
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              horizontalAlignment: Label.AlignLeft
              Layout.maximumWidth: layout.width - 4 - 24
              color: root.model.alarms[index] ? Cpp_ThemeManager.colors["alarm"] :
                                                Cpp_ThemeManager.colors["widget_text"]

              Behavior on color {ColorAnimation{}}
            }

            Item {
              Layout.fillWidth: true
              Layout.minimumWidth: 4
            }
          }

          MultiEffect {
            source: led
            width: led.width
            height: led.height
            x: layout.x + led.x
            y: layout.y + led.y

            blurEnabled: true
            blur: root.model.states[index] ? 1 : 0
            blurMax: root.model.states[index] ? 64 : 0
            brightness: root.model.states[index] ? 0.4 : 0
            saturation: root.model.states[index] ? 0.2 : 0
          }

          MultiEffect {
            source: led
            width: led.width
            height: led.height
            x: layout.x + led.x
            y: layout.y + led.y

            blurEnabled: true
            blur: root.model.states[index] ? 0.3 : 0
            blurMax: root.model.states[index] ? 64 : 0
            brightness: root.model.states[index] ? 0.4 : 0
            saturation: root.model.states[index] ? 0.2 : 0
          }
        }
      }
    }
  }
}
