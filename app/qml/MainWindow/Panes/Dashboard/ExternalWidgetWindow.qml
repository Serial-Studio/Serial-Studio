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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Controls

import SerialStudio

import "../../../Widgets" as Widgets

Widgets.SmartWindow {
  id: window

  visible: true
  minimumWidth: 356
  minimumHeight: 320
  width: minimumWidth
  height: minimumHeight
  transientParent: null
  objectName: "ExternalWindow"
  category: "ExternalWidget_" + stableType + "_" + stableIndex

  //
  // Dashboard widget displayed by this window; owned outside the workspace
  // delegates so the window survives workspace switches
  //
  required property int widgetIndex

  //
  // Stable (widgetType, relativeIndex) identity captured at creation; geometry
  // and open-state persistence key off this pair since indices remap on reload
  //
  readonly property int stableType: Cpp_UI_Dashboard.widgetType(widgetIndex)
  readonly property int stableIndex: Cpp_UI_Dashboard.relativeIndex(widgetIndex)

  //
  // Emitted by embedded widgets (via windowRoot) to pop another dashboard widget
  // into an external window; handled by DashboardCanvas
  //
  signal externalWidgetRequested(int windowId)

  //
  // Widget metadata reported by the embedded DashboardWidget
  //
  property int deviceIndex: 0
  property bool hasToolbar: false
  readonly property bool focused: true

  //
  // Titlebar height reported by native window.
  //
  property int titlebarHeight: 0

  //
  // Caption color blends with toolbar or window background.
  //
  readonly property color captionColor: hasToolbar
                                        ? Cpp_ThemeManager.colors["window_toolbar_background"]
                                        : Cpp_ThemeManager.colors["window"]

  onVisibleChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(window, captionColor)
    else
      Cpp_NativeWindow.removeWindow(window)

    window.titlebarHeight = Cpp_NativeWindow.titlebarHeight(window)
  }

  onCaptionColorChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(window, captionColor)
  }

  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (window.visible)
        Cpp_NativeWindow.addWindow(window, window.captionColor)
    }
  }

  //
  // QML loader component
  //
  Component {
    id: widgetLoader

    Item {
      id: loader

      anchors.fill: parent
      property var windowRoot: null
      property var widgetInstance: null

      DashboardWidget {
        id: dashboardWidget

        widgetIndex: window.widgetIndex
        Component.onCompleted: {
          windowRoot.title       = widgetTitle
          windowRoot.deviceIndex = widgetSourceId
          if (windowRoot.icon !== undefined)
            windowRoot.icon = SerialStudio.dashboardWidgetIcon(widgetType)
        }
      }

      Component.onCompleted: {
        const component = Qt.createComponent(dashboardWidget.widgetQmlPath)
        if (component.status === Component.Ready) {
          if (widgetInstance) {
            if (widgetInstance.settings)
              widgetInstance.settings.sync()

            widgetInstance.destroy()
          }

          widgetInstance = component.createObject(loader, {
                                                    model: dashboardWidget.widgetModel,
                                                    windowRoot: loader.windowRoot,
                                                    color: dashboardWidget.widgetColor,
                                                    widgetId: dashboardWidget.widgetId
                                                  })

          if (!widgetInstance) {
            console.error("Failed to create widget from", dashboardWidget.widgetQmlPath)
            return
          }

          if (widgetInstance.hasToolbar !== undefined) {
            windowRoot.hasToolbar = widgetInstance.hasToolbar
            if (widgetInstance.hasToolbarChanged !== undefined) {
              widgetInstance.hasToolbarChanged.connect(function () {
                windowRoot.hasToolbar = widgetInstance.hasToolbar
              })
            }
          }

          widgetInstance.anchors.fill = loader
        }

        else if (component.status === Component.Error)
          console.error("Component load error:", component.errorString())

        if (window.width === window.minimumWidth && window.height === window.minimumHeight) {
          const gpsPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/GPS.qml"
          const barPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Bar.qml"
          const pltPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Plot.qml"
          const fftPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/FFTPlot.qml"
          const plot3DPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Plot3D.qml"
          const mplotPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/MultiPlot.qml"
          const webviewPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/WebView.qml"
          const painterPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Painter.qml"
          const datagridPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/DataGrid.qml"
          const waterfallPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Waterfall.qml"
          const imageViewPath = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/ImageView.qml"

          switch (dashboardWidget.widgetQmlPath) {
          case barPath:
            window.height = window.width * 1.4
            break
          case gpsPath:
          case fftPath:
          case pltPath:
          case mplotPath:
          case plot3DPath:
          case webviewPath:
          case painterPath:
          case datagridPath:
          case waterfallPath:
          case imageViewPath:
            window.width = 640
            window.height = 480
            break
          default:
            break
          }
        }
      }

      Connections {
        target: Cpp_ThemeManager

        function onThemeChanged() {
          if (widgetInstance !== null)
            widgetInstance.color = dashboardWidget.widgetColor
        }
      }
    }
  }

  //
  // Titlebar background + window drag handler.
  //
  Rectangle {
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    color: window.captionColor
    height: window.titlebarHeight

    DragHandler {
      target: null
      onActiveChanged: {
        if (active)
          window.startSystemMove()
      }
    }
  }

  //
  // Titlebar text
  //
  Label {
    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }

    text: window.title
    visible: window.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
  }

  Page {
    anchors.fill: parent
    anchors.topMargin: window.titlebarHeight
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

    Rectangle {
      id: toolbarBackground

      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      height: 48
      border.width: 0
      visible: window.hasToolbar
      color: Cpp_ThemeManager.colors["window_toolbar_background"]

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

    Item {
      anchors.fill: parent
      LayoutMirroring.enabled: false
      LayoutMirroring.childrenInherit: true
      Component.onCompleted: {
        window.displayWindow()
        widgetLoader.createObject(this, {windowRoot: window})
      }
    }
  }
}
