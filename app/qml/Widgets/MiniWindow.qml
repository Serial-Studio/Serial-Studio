/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
  property bool shadowEnabled: true
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
  // Enable/disable window when state changes
  //
  state: "normal"
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
    enabled: !Cpp_Misc_ModuleManager.softwareRendering && root.shadowEnabled
    visible: !Cpp_Misc_ModuleManager.softwareRendering && root.shadowEnabled
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
      // Expand button
      //
      Controls.ToolButton {
        id: expandBt
        flat: true
        icon.width: 18
        icon.height: 18
        background: Item {}
        icon.color: _title.color
        Layout.alignment: Qt.AlignVCenter
        onClicked: root.externalWindowClicked()
        icon.source: "qrc:/rcc/icons/miniwindow/external.svg"

        anchors {
          left: parent.left
          verticalCenter: parent.verticalCenter
        }

        MouseArea {
          id: externalWinBt
          anchors.fill: parent
          onClicked: root.externalWindowClicked()
        }
      }

      //
      // Window controls
      //
      RowLayout {
        spacing: 0
        height: root.captionHeight
        anchors.right: parent.right
        anchors.left: expandBt.right

        Controls.Label {
          id: _title
          text: root.title
          elide: Qt.ElideRight
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignVCenter
          horizontalAlignment: Qt.AlignLeft
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: root.focused ? Cpp_ThemeManager.colors["window_caption_active_text"] :
                                Cpp_ThemeManager.colors["window_caption_inactive_text"]
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
