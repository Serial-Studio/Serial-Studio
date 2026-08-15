/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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
  property bool shadowFollowsFocus: true
  readonly property int defaultRadius: 0
  property bool windowControlsVisible: true

  //
  // Progressive caption collapse: close button + drag strip survive down to 48 px
  //
  readonly property bool showMaximize: root.width >= 88
  readonly property bool showMinimize: root.width >= 120
  readonly property bool showTitleLabel: root.width >= 148
  readonly property bool showMenuButton: root.width >= 200

  //
  // Drag-region geometry consumed by the C++ window manager; hidden controls
  // must contribute zero width or the caption hit-test eats the drag strip
  //
  readonly property int menuControlWidth: menuBtMa.width
  readonly property int captionHeight: root.headerVisible ? 28 : 0
  readonly property int windowControlsWidth: root.windowControlsVisible
                                             ? (root.showMinimize ? minBtMa.width : 0)
                                               + (root.showMaximize ? maxBtMa.width : 0)
                                               + closeBtMa.width
                                             : 0

  //
  // Custom properties...to be set by the items that subclass this object
  //
  property bool hasToolbar: false
  property alias radius: _bg.radius

  //
  // Subclass hooks: gate state animations past the initial taskbar sync, and hold the
  // window visible until a hide (minimize/close) transition finishes
  //
  property bool animationsEnabled: true
  readonly property bool hideAnimationRunning: _minimizeTransition.running
                                               || _closeTransition.running

  //
  property int deviceIndex: 0

  //
  // Colors
  //
  property color backgroundColor: Cpp_ThemeManager.colors["widget_base"]
  property color captionTopColor: {
    const _t = Cpp_ThemeManager.theme
    const singleDevice = Cpp_JSON_ProjectModel.sourceCount <= 1
                         || Cpp_AppState.operationMode !== SerialStudio.ProjectFile
    if (singleDevice)
      return root.focused ? Cpp_ThemeManager.colors["window_caption_active_top"]
                          : Cpp_ThemeManager.colors["window_caption_inactive_top"]

    const top = SerialStudio.getDeviceTopColor(root.deviceIndex + 1)
    if (root.focused)
      return top

    return Qt.hsla(top.hslHue, top.hslSaturation * 0.45, top.hslLightness, 1.0)
  }
  property color captionBottomColor: {
    const _t = Cpp_ThemeManager.theme
    const singleDevice = Cpp_JSON_ProjectModel.sourceCount <= 1
                         || Cpp_AppState.operationMode !== SerialStudio.ProjectFile
    if (singleDevice)
      return root.focused ? Cpp_ThemeManager.colors["window_caption_active_bottom"]
                          : Cpp_ThemeManager.colors["window_caption_inactive_bottom"]

    const bot = SerialStudio.getDeviceBottomColor(root.deviceIndex + 1)
    if (root.focused)
      return bot

    return Qt.hsla(bot.hslHue, bot.hslSaturation * 0.45, bot.hslLightness, 1.0)
  }

  //
  // Window state properties
  //
  property bool focused: false
  property bool highlighted: false

  //
  // Caption button signals
  //
  signal menuClicked()
  signal closeClicked()
  signal restoreClicked()
  signal minimizeClicked()
  signal maximizeClicked()

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
  // Enable/disable window when state changes; scale animations anchor at the bottom
  // edge so minimize/restore read as moving toward the taskbar
  //
  state: "normal"
  transformOrigin: Item.Bottom
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
      id: _minimizeTransition

      from: "*"
      to: "minimized"
      enabled: root.animationsEnabled

      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
        properties: "x,y,scale,opacity"
      }
    },

    Transition {
      from: "*"
      to: "normal"
      enabled: root.animationsEnabled

      NumberAnimation {
        duration: 200
        properties: "scale,opacity"
        easing.type: Easing.InOutQuad
      }
    },

    Transition {
      from: "*"
      to: "maximized"
      enabled: root.animationsEnabled

      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
        properties: "x,y,width,height,scale,opacity"
      }
    },

    Transition {
      id: _closeTransition

      from: "*"
      to: "closed"
      enabled: root.animationsEnabled

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
    source: _shadowSrc
    anchors.fill: _shadowSrc

    //
    // Lifted only while focused, and only where focus is a live notion: a frozen dashboard
    // has no focus to follow, so its shadows must not flicker as the pointer moves
    //
    readonly property bool lifted: root.focused && root.shadowFollowsFocus

    //
    // Blur config
    //
    blurEnabled: true
    blur: lifted ? 0.6 : 0.3
    blurMax: lifted ? 24 : 12

    //
    // Shadow config
    //
    shadowEnabled: true
    shadowOpacity: lifted ? 0.07 : 0.035
    shadowColor: Cpp_ThemeManager.colors["shadow"]

    //
    // Only enabled when using RHI
    //
    enabled: root.shadowEnabled && Cpp_Misc_GraphicsBackend.effectsEnabled
    visible: root.shadowEnabled && Cpp_Misc_GraphicsBackend.effectsEnabled
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
      // Window menu button
      //
      Controls.ToolButton {
        id: menuBt

        flat: true
        icon.width: 12
        icon.height: 12
        background: Item {}
        icon.color: _title.color
        visible: root.showMenuButton
        width: root.showMenuButton ? implicitWidth : 0
        Layout.alignment: Qt.AlignVCenter
        onClicked: root.menuClicked()
        icon.source: "qrc:/icons/buttons/menu.svg"

        anchors {
          left: parent.left
          verticalCenter: parent.verticalCenter
        }

        MouseArea {
          id: menuBtMa

          anchors.fill: parent
          onClicked: root.menuClicked()
        }
      }

      //
      // Window controls
      //
      RowLayout {
        spacing: 0
        height: root.captionHeight
        anchors.left: menuBt.right
        anchors.right: parent.right

        Controls.Label {
          id: _title

          text: root.title
          elide: Qt.ElideRight
          Layout.fillWidth: true
          visible: root.showTitleLabel
          Layout.alignment: Qt.AlignVCenter
          horizontalAlignment: Qt.AlignLeft
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: root.focused ? Cpp_ThemeManager.colors["window_caption_active_text"] :
                                Cpp_ThemeManager.colors["window_caption_inactive_text"]
        }

        Item {
          Layout.fillWidth: true
          visible: !root.showTitleLabel
        }

        Controls.ToolButton {
          flat: true
          background: Item {}
          icon.color: _title.color
          visible: root.windowControlsVisible && root.showMinimize
          Layout.alignment: Qt.AlignVCenter
          onClicked: root.minimizeClicked()
          icon.width: root.captionHeight / 2
          icon.height: root.captionHeight / 2
          icon.source: Cpp_Misc_IconRegistry.icon("window", "minimize", 32)

          MouseArea {
            id: minBtMa

            anchors.fill: parent
            onClicked: root.minimizeClicked()
          }
        }

        Controls.ToolButton {
          flat: true
          onClicked: {
            if (root.state === "maximized")
              root.restoreClicked()

            else
              root.maximizeClicked()
          }
          background: Item {}
          visible: root.windowControlsVisible && root.showMaximize
          Layout.alignment: Qt.AlignVCenter
          icon.width: root.captionHeight / 2
          icon.height: root.captionHeight / 2

          icon.color: _title.color
          icon.source: root.state === "maximized"
                       ? Cpp_Misc_IconRegistry.icon("window", "restore", 32)
                       : Cpp_Misc_IconRegistry.icon("window", "maximize", 32)

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
          visible: root.windowControlsVisible
          icon.height: root.captionHeight / 2
          icon.source: Cpp_Misc_IconRegistry.icon("window", "close", 32)

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

  //
  // Highlight overlay: flashing border for search navigation
  //
  Rectangle {
    id: highlightBorder

    z: 999
    border.width: 3
    color: "transparent"
    radius: root.radius
    anchors.fill: parent
    visible: root.highlighted
    border.color: Cpp_ThemeManager.colors["highlight"]

    SequentialAnimation on opacity {
      id: flashAnim

      loops: 3
      running: root.highlighted

      NumberAnimation {
        from: 1
        to: 0.2
        duration: 400
        easing.type: Easing.InOutSine
      }

      NumberAnimation {
        to: 1
        from: 0.2
        duration: 400
        easing.type: Easing.InOutSine
      }

      onFinished: root.highlighted = false
    }
  }
}
