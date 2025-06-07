/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls as Controls

import SerialStudio

Item {
  id: root

  //
  // Window caption properties
  //
  property string icon: ""
  property string title: ""
  property bool headerVisible: true
  readonly property int defaultRadius: 0

  //
  // Properties used by the C++ window manager to detect where to drag window
  //
  readonly property int captionHeight: root.headerVisible ? 28 : 0
  readonly property int externControlWidth: externalWinBt.width
  readonly property int windowControlsWidth: minBtMa.width + maxBtMa.width + closeBtMa.width

  //
  // Custom properties...to be set by the items that subclass this object
  //
  property bool hasToolbar: false
  property alias radius: _bg.radius

  //
  // Colors
  //
  property color backgroundColor: Cpp_ThemeManager.colors["widget_base"]
  property color captionTopColor: root.focused ? Cpp_ThemeManager.colors["window_caption_active_top"] :
                                                 Cpp_ThemeManager.colors["window_caption_inactive_top"]
  property color captionBottomColor: root.focused ? Cpp_ThemeManager.colors["window_caption_active_bottom"] :
                                                    Cpp_ThemeManager.colors["window_caption_inactive_bottom"]

  //
  // Window state properties
  //
  property bool focused: false

  //
  // Caption button signals
  //
  signal closeClicked()
  signal restoreClicked()
  signal minimizeClicked()
  signal maximizeClicked()
  signal externalWindowClicked()

  //
  // Internal properties for saving/restoring window geometry
  //
  property int prevX: x
  property int prevY: y
  property int prevWidth: width
  property int prevHeight: height

  //
  // Minimize button behavior
  //
  onMinimizeClicked: {
    prevX = x
    prevY = y
    root.state = "minimized"
  }

  //
  // Maximize button behavior
  //
  onMaximizeClicked: {
    prevX = x
    prevY = y
    root.radius = 0
    prevWidth = width
    prevHeight = height
    root.state = "maximized"
  }

  //
  // Restore button behavior
  //
  onRestoreClicked: {
    root.x = prevX
    root.y = prevY
    root.state = "normal"
    root.width = prevWidth
    root.height = prevHeight
    root.radius = defaultRadius
  }

  //
  // Close button behavior
  //
  onCloseClicked: {
    prevX = x
    prevY = y
    root.state = "closed"
  }

  //
  // Disable the window when opacity falls below 1
  //
  enabled: root.state === "normal" || root.state === "maximized"

  //
  // Window states
  //
  states: [
    State {
      name: "minimized"
      PropertyChanges {
        x: prevX
        y: prevY
        scale: 0.0
        opacity: 0.0
        target: root
        width: prevWidth
        height: prevHeight
      }
    },

    State {
      name: "maximized"
      PropertyChanges {
        x: 0
        y: 0
        radius: 0
        scale: 1.0
        opacity: 1.0
        target: root
        width: parent ? parent.width : prevWidth
        height: parent ? parent.height : prevHeight
      }
    },

    State {
      name: "normal"
      PropertyChanges {
        x: prevX
        y: prevY
        scale: 1.0
        opacity: 1.0
        target: root
        width: prevWidth
        height: prevHeight
        radius: defaultRadius
      }
    },
    State {
      name: "closed"
      PropertyChanges {
        x: prevX
        y: prevY
        scale: 0.0
        opacity: 0.0
        target: root
        width: prevWidth
        height: prevHeight
      }
    }
  ]

  //
  // State transitions/animations
  //
  transitions: [
    Transition {
      from: "*"
      to: "minimized"

      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
        properties: "x,y,scale,opacity"
      }
    },

    Transition {
      from: "*"
      to: "normal"

      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
        properties: "scale,opacity"
      }
    },

    Transition {
      from: "*"
      to: "maximized"

      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
        properties: "x,y,width,height,scale,opacity"
      }
    },

    Transition {
      from: "*"
      to: "closed"

      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
        properties: "x,y,scale,opacity"
      }
    }
  ]

  //
  // Window shadow source
  //
  Rectangle {
    id: _shadowSrc
    visible: false
    radius: root.radius
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["shadow"]
  }

  //
  // Window shadow effect
  //
  MultiEffect {
    anchors.fill: _shadowSrc
    source: _shadowSrc

    // Blur config
    blurEnabled: true
    blur: root.focused ? 0.6 : 0.3
    blurMax: root.focused ? 24 : 12

    // Shadow config
    shadowEnabled: true
    shadowOpacity: root.focused ? 0.07 : 0.035
    shadowColor: Cpp_ThemeManager.colors["shadow"]

    // Only enabled when using RHI
    enabled: !Cpp_Misc_ModuleManager.softwareRendering
    visible: !Cpp_Misc_ModuleManager.softwareRendering
  }

  //
  // Window background
  //
  Rectangle {
    id: _bg
    clip: true
    anchors.fill: parent
    radius: defaultRadius

    //
    // Window caption background
    //
    Rectangle {
      radius: root.radius
      visible: root.headerVisible
      height: visible ? root.captionHeight : 0

      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      Rectangle {
        width: 1
        color: Cpp_ThemeManager.colors["window_border"]

        anchors {
          top: parent.top
          left: parent.left
          bottom: parent.bottom
        }
      }

      Rectangle {
        width: 1
        color: Cpp_ThemeManager.colors["window_border"]

        anchors {
          top: parent.top
          right: parent.right
          bottom: parent.bottom
        }
      }

      Rectangle {
        height: 1
        color: Cpp_ThemeManager.colors["window_border"]

        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
        }
      }

      Rectangle {
        color: Cpp_ThemeManager.colors["window_border"]
        height: root.hasToolbar && root.captionBottomColor === _toolbar.color ? 0 : 1

        anchors {
          left: parent.left
          right: parent.right
          bottom: parent.bottom
        }
      }

      gradient: Gradient {
        GradientStop {
          position: 0
          color: root.captionTopColor
        }

        GradientStop {
          position: 1
          color: root.captionBottomColor
        }
      }
    }

    //
    // Window caption
    //
    Item {
      id: _caption
      visible: root.headerVisible
      height: visible ? root.captionHeight : 0

      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      //
      // Window caption
      //
      RowLayout {
        anchors.centerIn: parent
        height: root.captionHeight
        spacing: root.captionHeight / 8

        Image {
          source: root.icon
          Layout.alignment: Qt.AlignVCenter
          sourceSize: Qt.size(root.captionHeight / 2, root.captionHeight / 2)
        }

        Controls.Label {
          id: _title
          text: root.title
          elide: Qt.ElideRight
          Layout.alignment: Qt.AlignVCenter
          horizontalAlignment: Qt.AlignLeft
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: root.focused ? Cpp_ThemeManager.colors["window_caption_active_text"] :
                                Cpp_ThemeManager.colors["window_caption_inactive_text"]
        }
      }

      //
      // Window controls
      //
      RowLayout {
        spacing: 0
        height: root.captionHeight
        anchors.left: parent.left
        anchors.right: parent.right

        Controls.ToolButton {
          flat: true
          background: Item {}
          icon.color: _title.color
          Layout.alignment: Qt.AlignVCenter
          icon.width: root.captionHeight / 2
          icon.height: root.captionHeight / 2
          onClicked: root.externalWindowClicked()
          icon.source: "qrc:/rcc/icons/miniwindow/external.svg"

          MouseArea {
            id: externalWinBt
            anchors.fill: parent
            onClicked: root.externalWindowClicked()
          }
        }

        Item {
          Layout.fillWidth: true
        }

        Controls.ToolButton {
          flat: true
          background: Item {}
          icon.color: _title.color
          Layout.alignment: Qt.AlignVCenter
          onClicked: root.minimizeClicked()
          icon.width: root.captionHeight / 2
          icon.height: root.captionHeight / 2
          icon.source: "qrc:/rcc/icons/miniwindow/minimize.svg"

          MouseArea {
            id: minBtMa
            anchors.fill: parent
            onClicked: root.minimizeClicked()
          }
        }

        Controls.ToolButton {
          flat: true
          background: Item {}
          Layout.alignment: Qt.AlignVCenter
          icon.width: root.captionHeight / 2
          icon.height: root.captionHeight / 2
          onClicked: {
            if (root.state === "maximized")
              root.restoreClicked()

            else
              root.maximizeClicked()
          }

          icon.color: _title.color
          icon.source: root.state === "maximized" ? "qrc:/rcc/icons/miniwindow/restore.svg" :
                                                    "qrc:/rcc/icons/miniwindow/maximize.svg"

          MouseArea {
            id: maxBtMa
            anchors.fill: parent
            onClicked: {
              if (root.state === "maximized")
                root.restoreClicked()

              else
                root.maximizeClicked()
            }
          }
        }

        Controls.ToolButton {
          flat: true
          background: Item {}
          icon.color: _title.color
          onClicked: root.closeClicked()
          Layout.alignment: Qt.AlignVCenter
          icon.width: root.captionHeight / 2
          icon.height: root.captionHeight / 2
          icon.source: "qrc:/rcc/icons/miniwindow/close.svg"

          MouseArea {
            id: closeBtMa
            anchors.fill: parent
            onClicked: root.closeClicked()
          }
        }

        Item {
          implicitWidth: 4
        }
      }
    }

    //
    // Toolbar background
    //
    Rectangle {
      id: _toolbar
      visible: root.hasToolbar
      height: visible ? 48 : 0
      color: Cpp_ThemeManager.colors["window_toolbar_background"]

      anchors {
        left: parent.left
        right: parent.right
        top: _caption.bottom
      }

      Rectangle {
        width: 1
        color: Cpp_ThemeManager.colors["window_border"]

        anchors {
          top: parent.top
          left: parent.left
          bottom: parent.bottom
        }
      }

      Rectangle {
        width: 1
        color: Cpp_ThemeManager.colors["window_border"]

        anchors {
          top: parent.top
          right: parent.right
          bottom: parent.bottom
        }
      }

      Rectangle {
        height: 1
        color: Cpp_ThemeManager.colors["window_border"]

        anchors {
          left: parent.left
          right: parent.right
          bottom: parent.bottom
        }
      }
    }
  }
}
