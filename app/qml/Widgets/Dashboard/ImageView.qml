/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

import SerialStudio

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property ImageViewModel model

  //
  // Toolbar visibility
  //
  property bool hasToolbar: true
  onWidthChanged:  hasToolbar = (root.width >= toolbar.implicitWidth) && (root.height >= 220)
  onHeightChanged: hasToolbar = (root.width >= toolbar.implicitWidth) && (root.height >= 220)

  //
  // Zoom / pan state
  //
  property real zoom: 1.0
  property real panX: 0.0
  property real panY: 0.0
  property bool panMode: false
  property bool showCrosshair: false
  readonly property real minZoom: 1.0
  readonly property real maxZoom: 5.0

  //
  // Image filter — index into filterModel
  //
  property int filterIndex: 0
  readonly property var currentFilter: filterModel[filterIndex]
  readonly property var filterModel: [
    { label: qsTr("Normal"),        sat:  0.0, bright: 0.0, cont: 0.0, colorize: false, color: "#000000" },
    { label: qsTr("Grayscale"),     sat: -1.0, bright: 0.0, cont: 0.0, colorize: false, color: "#000000" },
    { label: qsTr("High Contrast"), sat:  0.2, bright:-0.1, cont: 0.8, colorize: false, color: "#000000" },
    { label: qsTr("Vivid"),         sat:  0.8, bright: 0.1, cont: 0.3, colorize: false, color: "#000000" },
    { label: qsTr("Night Vision"),  sat:  0.0, bright: 0.2, cont: 0.4, colorize: true,  color: "#00ff44" },
    { label: qsTr("Infrared"),      sat:  0.0, bright: 0.1, cont: 0.3, colorize: true,  color: "#ff4400" },
    { label: qsTr("Deep Blue"),     sat:  0.0, bright: 0.0, cont: 0.2, colorize: true,  color: "#0088ff" },
    { label: qsTr("Amber"),         sat:  0.0, bright: 0.1, cont: 0.2, colorize: true,  color: "#ffaa00" },
  ]

  //
  // Pixel coordinate of the cursor inside the original image (–1 = outside)
  //
  property int cursorImgX: -1
  property int cursorImgY: -1

  //
  // Helpers: compute the unzoomed painted size analytically from the image
  // source dimensions and the area size, so clampPan() never reads stale
  // layout properties.
  //
  function basePaintedSize() {
    const pw = mainImage.paintedWidth
    const ph = mainImage.paintedHeight
    if (pw > 0 && ph > 0)
      return Qt.size(pw, ph)

    const iw = mainImage.implicitWidth
    const ih = mainImage.implicitHeight
    if (iw <= 0 || ih <= 0 || imageArea.width <= 0 || imageArea.height <= 0)
      return Qt.size(imageArea.width, imageArea.height)

    const imageAspect = iw / ih
    if (imageArea.width / imageArea.height > imageAspect)
      return Qt.size(imageArea.height * imageAspect, imageArea.height)

    return Qt.size(imageArea.width, imageArea.width / imageAspect)
  }

  //
  // Zoomed painted dimensions and absolute position of the image rect inside
  // the widget — used by the crosshair overlay and vignette positioning.
  //
  readonly property real paintedW: mainImage.paintedWidth  * zoom
  readonly property real paintedH: mainImage.paintedHeight * zoom
  readonly property real paintedX: imageArea.x + (imageArea.width  - paintedW) / 2 + panX
  readonly property real paintedY: imageArea.y + (imageArea.height - paintedH) / 2 + panY

  //
  // Constrain panX/panY so the image edge never retreats past the area edge.
  // Accepts an explicit zoom value to avoid reading the stale root.zoom binding
  // when called immediately after root.zoom is written.
  //
  function clampPan(z) {
    const z2   = (z !== undefined) ? z : zoom
    const base = basePaintedSize()
    const bw   = base.width
    const bh   = base.height
    const aw   = imageArea.width
    const ah   = imageArea.height
    const limitX = Math.max(0, (bw * z2 - aw) / 2)
    const limitY = Math.max(0, (bh * z2 - ah) / 2)

    panX = Math.max(-limitX, Math.min(limitX, panX))
    panY = Math.max(-limitY, Math.min(limitY, panY))
  }

  //
  // Return zoom and pan to their default (fit-to-area) state.
  //
  function resetView() {
    zoom = 1.0
    panX = 0.0
    panY = 0.0
  }

  //
  // Hot-reload the image
  //
  Connections {
    target: model
    function onImageReady() {
      mainImage.source = model.imageUrl
      blurSrc.source   = model.imageUrl
    }
  }

  //
  // Main layout
  //
  ColumnLayout {
    spacing: 0
    anchors.fill: parent

    //
    // Toolbar
    //
    RowLayout {
      id: toolbar

      spacing: 4
      Layout.fillWidth: true
      Layout.preferredHeight: 48
      Layout.leftMargin: 8
      visible: root.hasToolbar && model && model.frameCount > 0

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        checked: model && model.exportEnabled
        icon.source: "qrc:/rcc/icons/dashboard-buttons/archive.svg"
        onClicked: {
          if (model)
            model.exportEnabled = !model.exportEnabled
        }
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        icon.source: "qrc:/rcc/icons/dashboard-buttons/pictures-folder.svg"
        onClicked: {
          if (model)
            Cpp_Misc_Utilities.revealFile(
                  Cpp_Image_Export.imagesPath(model.groupTitle, Cpp_UI_Dashboard.title))
        }
      }

      Rectangle {
        implicitWidth: 1
        implicitHeight: 24
        color: Cpp_ThemeManager.colors["widget_border"]
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        icon.source: "qrc:/rcc/icons/dashboard-buttons/zoom-in.svg"
        onClicked: {
          const oldZoom = root.zoom
          const newZoom = Math.min(root.maxZoom, oldZoom * 1.5)
          root.panX = root.panX * newZoom / oldZoom
          root.panY = root.panY * newZoom / oldZoom
          root.zoom = newZoom
          root.clampPan(newZoom)
        }
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        icon.source: "qrc:/rcc/icons/dashboard-buttons/zoom-out.svg"
        onClicked: {
          const oldZoom = root.zoom
          const newZoom = Math.max(root.minZoom, oldZoom / 1.5)
          root.panX = root.panX * newZoom / oldZoom
          root.panY = root.panY * newZoom / oldZoom
          root.zoom = newZoom
          root.clampPan(newZoom)
        }
      }

      Rectangle {
        implicitWidth: 1
        implicitHeight: 24
        color: Cpp_ThemeManager.colors["widget_border"]
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        checked: root.showCrosshair
        icon.source: "qrc:/rcc/icons/dashboard-buttons/crosshair.svg"
        onClicked: {
          root.showCrosshair = !root.showCrosshair
          if (!root.showCrosshair) {
            root.cursorImgX = -1
            root.cursorImgY = -1
          }
        }
      }

      Rectangle {
        implicitWidth: 1
        implicitHeight: 24
        color: Cpp_ThemeManager.colors["widget_border"]
      }

      ToolButton {
        width: 24
        height: 24
        enabled: false
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        icon.source: "qrc:/rcc/icons/dashboard-buttons/filter.svg"
      }

      ComboBox {
        id: filterCombo

        implicitHeight: 24
        implicitWidth: 120
        currentIndex: root.filterIndex
        model: root.filterModel.map(f => f.label)
        onActivated: root.filterIndex = currentIndex
      }

      Item { Layout.fillWidth: true }
    }

    //
    // Image area
    //
    Rectangle {
      id: imageArea

      clip: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      color: model && model.frameCount > 0 ? model.primaryColor : "#000"

      Behavior on color { ColorAnimation { duration: 600; easing.type: Easing.OutCubic } }

      //
      // Blurred background
      //
      Image {
        id: blurSrc

        cache: false
        smooth: false
        mipmap: false
        visible: false
        asynchronous: false
        anchors.fill: parent
        sourceSize: Qt.size(128, 128)
        fillMode: Image.PreserveAspectCrop
      } MultiEffect {
        blur: 1.0
        blurMax: 64
        source: blurSrc
        blurEnabled: true
        blurMultiplier: 10
        anchors.fill: parent
        visible: model && model.frameCount > 0
        contrast:          root.currentFilter.cont
        saturation:        root.currentFilter.sat
        brightness:        root.currentFilter.bright
        colorization:      root.currentFilter.colorize ? 1.0 : 0.0
        colorizationColor: root.currentFilter.color
      }

      //
      // Main image
      //
      Item {
        id: imageContainer

        opacity: 0
        anchors.fill: parent

        Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }

        transform: [
          Scale {
            xScale: root.zoom
            yScale: root.zoom
            origin.x: imageArea.width  / 2
            origin.y: imageArea.height / 2
          },
          Translate { x: root.panX; y: root.panY
          }
        ]

        layer.enabled: true
        layer.effect: MultiEffect {
          contrast:          root.currentFilter.cont
          saturation:        root.currentFilter.sat
          brightness:        root.currentFilter.bright
          colorization:      root.currentFilter.colorize ? 1.0 : 0.0
          colorizationColor: root.currentFilter.color
        }

        Image {
          id: mainImage

          cache: false
          smooth: true
          asynchronous: false
          anchors.fill: parent
          fillMode: Image.PreserveAspectFit
          source: model && model.frameCount > 0 ? model.imageUrl : ""

          onStatusChanged: {
            if (status === Image.Ready)
              imageContainer.opacity = 1
          }
        }
      }

      //
      // Interaction handler
      //
      PinchArea {
        anchors.fill: parent
        enabled: model && model.frameCount > 0

        property real startZoom: 1.0
        property real startPanX: 0.0
        property real startPanY: 0.0

        onPinchStarted: {
          startZoom = root.zoom
          startPanX = root.panX
          startPanY = root.panY
        }

        onPinchUpdated: (pinch) => {
                          const newZoom = Math.max(root.minZoom, Math.min(root.maxZoom, startZoom * pinch.scale))
                          root.zoom = newZoom
                          root.panX = startPanX + pinch.center.x - pinch.startCenter.x
                          root.panY = startPanY + pinch.center.y - pinch.startCenter.y
                          root.clampPan(newZoom)
                        }

        MouseArea {
          anchors.fill: parent
          acceptedButtons: Qt.LeftButton
          hoverEnabled: root.showCrosshair
          cursorShape: pressed && (root.panMode || root.zoom > 1) ? Qt.ClosedHandCursor :
                                                                    Qt.CrossCursor

          property real dragStartX: 0
          property real dragStartY: 0
          property real dragStartPanX: 0
          property real dragStartPanY: 0
          property bool didDrag: false

          readonly property bool dragging: pressed && (root.panMode || root.zoom > 1)

          onWheel: (wheel) => {
            const zoomFactor = 1.15
            const delta      = -wheel.angleDelta.y / 120
            const factor     = Math.pow(zoomFactor, -delta)
            const newZoom    = Math.max(root.minZoom, Math.min(root.maxZoom, root.zoom * factor))
            const ratio      = newZoom / root.zoom
            root.panX = ratio * (root.panX + imageArea.width  / 2 - wheel.x) + wheel.x - imageArea.width  / 2
            root.panY = ratio * (root.panY + imageArea.height / 2 - wheel.y) + wheel.y - imageArea.height / 2
            root.zoom = newZoom
            root.clampPan(newZoom)
          }

          onPressed: (mouse) => {
                       dragStartX    = mouse.x
                       dragStartY    = mouse.y
                       dragStartPanX = root.panX
                       dragStartPanY = root.panY
                       didDrag = false
                     }

          onPositionChanged: (mouse) => {
                               const dx = mouse.x - dragStartX
                               const dy = mouse.y - dragStartY
                               if (Math.abs(dx) > 3 || Math.abs(dy) > 3)
                               didDrag = true

                               if (pressed && (root.panMode || root.zoom > 1)) {
                                 root.panX = dragStartPanX + dx
                                 root.panY = dragStartPanY + dy
                                 root.clampPan()
                               }

                               if (root.showCrosshair) {
                                 const imgW = model ? model.imageWidth  : 0
                                 const imgH = model ? model.imageHeight : 0
                                 if (imgW > 0 && imgH > 0) {
                                   const lpX = root.paintedX - imageArea.x
                                   const lpY = root.paintedY - imageArea.y
                                   const ix  = (mouse.x - lpX) / root.paintedW * imgW
                                   const iy  = (mouse.y - lpY) / root.paintedH * imgH
                                   root.cursorImgX = (ix >= 0 && ix < imgW) ? Math.floor(ix) : -1
                                   root.cursorImgY = (iy >= 0 && iy < imgH) ? Math.floor(iy) : -1
                                 }
                               }
                             }

          onExited: {
            root.cursorImgX = -1
            root.cursorImgY = -1
          }
        }
      }

      //
      // Crosshair overlay
      //
      Item {
        id: crosshairLayer

        anchors.fill: parent
        visible: root.showCrosshair && root.cursorImgX >= 0 && model && model.frameCount > 0

        readonly property real lpX: root.paintedX - imageArea.x
        readonly property real lpY: root.paintedY - imageArea.y
        readonly property real cx: lpX + (root.cursorImgX + 0.5) / (model ? model.imageWidth  : 1) * root.paintedW
        readonly property real cy: lpY + (root.cursorImgY + 0.5) / (model ? model.imageHeight : 1) * root.paintedH

        Rectangle {
          width: 1
          opacity: 0.7
          color: "white"
          y: crosshairLayer.lpY
          height: root.paintedH
          x: crosshairLayer.cx - width / 2
        }

        Rectangle {
          height: 1
          opacity: 0.7
          color: "white"
          x: crosshairLayer.lpX
          width: root.paintedW
          y: crosshairLayer.cy - height / 2
        }

        Rectangle {
          width: 7
          height: 7
          radius: 3.5
          border.width: 1.5
          color: "transparent"
          border.color: "white"
          x: crosshairLayer.cx - width / 2
          y: crosshairLayer.cy - height / 2
        }

        Rectangle {
          radius: 3
          x: crosshairLayer.cx + 8
          color: Qt.rgba(0, 0, 0, 0.65)
          y: crosshairLayer.cy - height / 2
          height: coordLabel.implicitHeight + 6
          width: coordLabel.implicitWidth + 10

          Text {
            id: coordLabel

            color: "white"
            anchors.centerIn: parent
            font.pixelSize: Math.max(9, Math.min(11, Math.min(root.width, root.height) / 28))
            text: root.cursorImgX + ", " + root.cursorImgY
            font.family: Cpp_Misc_CommonFonts.widgetFontFamily
          }
        }
      }

      //
      // "Waiting for image…" placeholder
      //
      ColumnLayout {
        spacing: 12
        anchors.centerIn: parent
        visible: !model || model.frameCount === 0

        Image {
          id: placeholderIcon

          opacity: 0
          layer.enabled: true
          sourceSize.width: 48
          sourceSize.height: 48
          Layout.alignment: Qt.AlignHCenter
          source: "qrc:/rcc/icons/dashboard-large/image.svg"
          layer.effect: MultiEffect { colorization: 1.0; colorizationColor: "white" }

          SequentialAnimation on opacity {
            running: true
            loops: Animation.Infinite
            NumberAnimation { to: 0.55; duration: 900; easing.type: Easing.InOutSine }
            NumberAnimation { to: 0.20; duration: 900; easing.type: Easing.InOutSine }
          }
        }

        Text {
          opacity: 0.55
          color: "white"
          text: qsTr("Waiting for image…")
          Layout.alignment: Qt.AlignHCenter
          font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
        }
      }

    }
  }
}
