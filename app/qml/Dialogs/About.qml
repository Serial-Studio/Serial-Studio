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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  readonly property int year: new Date().getFullYear()

  //
  // Window options
  //
  title: qsTr("About")
  width: minimumWidth
  height: minimumHeight
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + root.titlebarHeight + 32
  maximumHeight: column.implicitHeight + root.titlebarHeight + 32

  //
  // Make window stay on top
  //
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Native window registration
  //
  property real titlebarHeight: 0
  onVisibleChanged: {
    if (visible) {
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["base"])
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
    }

    else {
      root.titlebarHeight = 0
      Cpp_NativeWindow.removeWindow(root)
    }
  }

  //
  // Background + window title on macOS
  //
  Rectangle {
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["window"]

    //
    // Drag the window anywhere
    //
    DragHandler {
      target: null
      onActiveChanged: {
        if (active)
          root.startSystemMove()
      }
    }

    //
    // Titlebar text
    //
    Label {
      text: root.title
      visible: root.titlebarHeight > 0
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(13, true)

      anchors {
        topMargin: 6
        top: parent.top
        horizontalCenter: parent.horizontalCenter
      }
    }
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
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

    //
    // Window controls
    //
    ColumnLayout {
      id: column
      spacing: 4
      anchors.centerIn: parent

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Image {
          Layout.minimumWidth: 128
          Layout.maximumWidth: 128
          Layout.minimumHeight: 128
          Layout.maximumHeight: 128
          Layout.alignment: Qt.AlignVCenter
          sourceSize: Qt.size(128, 128)
          source: {
            if (Screen.pixelDensity >= 2)
              return "qrc:/rcc/images/icon@2x.png"

            return "qrc:/rcc/images/icon@1x.png"
          }
        }

        ColumnLayout {
          spacing: 0
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter

          Label {
            text: Cpp_AppName
            font: Cpp_Misc_CommonFonts.customUiFont(28, true)
          }

          Item {
            implicitHeight: 8
          }

          Label {
            opacity: 0.5
            text: qsTr("Version %1").arg(Cpp_AppVersion)
            font: Cpp_Misc_CommonFonts.customUiFont(16, false)
          }
        }
      }

      Label {
        opacity: 0.8
        Layout.fillWidth: true
        Layout.maximumWidth: 320
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        text: qsTr("Copyright © 2020-%1 %2, released under the MIT License.").arg(root.year).arg(Cpp_AppOrganization)
      }

      Label {
        opacity: 0.8
        font.pixelSize: 12
        Layout.fillWidth: true
        Layout.maximumWidth: 320
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        text: qsTr("The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.")
      }

      Item {
        implicitHeight: 8
      }

      Button {
        Layout.fillWidth: true
        text: qsTr("Website")
        onClicked: Qt.openUrlExternally("https://serial-studio.github.io/")
      }

      Button {
        Layout.fillWidth: true
        text: qsTr("Check for Updates")
        onClicked: app.checkForUpdates()
      }

      Button {
        Layout.fillWidth: true
        text: qsTr("Make a Donation")
        onClicked: donateDialog.show()
      }

      Button {
        Layout.fillWidth: true
        text: qsTr("Report Bug")
        onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/issues")
      }

      Button {
        Layout.fillWidth: true
        text: qsTr("Documentation")
        onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
      }

      Button {
        Layout.fillWidth: true
        text: qsTr("Acknowledgements")
        onClicked: app.showAcknowledgements()
      }

      Item {
        implicitHeight: 8
      }

      Button {
        text: qsTr("Close")
        Layout.fillWidth: true
        onClicked: root.close()
      }
    }
  }
}
