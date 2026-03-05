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
  // Font size shared by all status labels
  //
  readonly property real fontSize: Math.max(9, Math.min(11, Math.min(root.width, root.height) / 28))

  //
  // Blur background updates at ~4 Hz
  //
  property string blurUrl: ""* Cpp_Misc_CommonFonts.widgetFontScale
  Timer {
    repeat: true
    interval: 250
    id: blurRefresh
    running: model && model.frameCount > 0
    onTriggered: root.blurUrl = model ? model.imageUrl : ""
  }

  //
  // Hot-reload the image
  //
  Connections {
    target: model
    function onImageReady() {
      mainImage.source = model.imageUrl
      if (model.frameCount === 1)
        root.blurUrl = model.imageUrl
    }
  }

  // -------------------------------------------------------------------------
  // Blurred background — slow-refreshed, fills letterbox bars
  // -------------------------------------------------------------------------

  Rectangle {
    color: "#000"
    anchors.fill: parent
  }

  Image {
    id: blurSrc
    cache: false
    smooth: false
    mipmap: false
    visible: false
    asynchronous: true
    anchors.fill: parent
    source: root.blurUrl
    sourceSize: Qt.size(128, 128)
    fillMode: Image.PreserveAspectCrop
  }

  MultiEffect {
    blur: 1.0
    blurMax: 64
    source: blurSrc
    blurEnabled: true
    blurMultiplier: 10
    anchors.fill: blurSrc
    visible: model && model.frameCount > 0
  }

  // -------------------------------------------------------------------------
  // Main image — every frame gets a unique URL so QML never serves a stale
  // cached texture; cache: false ensures the old texture is released promptly
  // -------------------------------------------------------------------------

  Image {
    id: mainImage

    cache: false
    smooth: true
    asynchronous: false
    anchors.fill: parent
    fillMode: Image.PreserveAspectFit
    source: model && model.frameCount > 0 ? model.imageUrl : ""

    opacity: 0
    Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }

    onStatusChanged: {
      if (status === Image.Ready)
        opacity = 1
    }
  }

  // -------------------------------------------------------------------------
  // Vignette — follows the painted rect of the main image exactly
  // -------------------------------------------------------------------------

  Item {
    width:  mainImage.paintedWidth
    height: mainImage.paintedHeight
    visible: model && model.frameCount > 0
    x: (root.width  - mainImage.paintedWidth)  / 2
    y: (root.height - mainImage.paintedHeight) / 2

    Rectangle {
      anchors.fill: parent
      gradient: Gradient {
        orientation: Gradient.Vertical
        GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.35) }
        GradientStop { position: 0.3; color: Qt.rgba(0, 0, 0, 0.00) }
        GradientStop { position: 0.7; color: Qt.rgba(0, 0, 0, 0.00) }
        GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.55) }
      }
    }
  }

  // -------------------------------------------------------------------------
  // "Waiting for image…" placeholder
  // -------------------------------------------------------------------------

  ColumnLayout {
    anchors.centerIn: parent
    visible: !model || model.frameCount === 0
    spacing: 12

    Image {
      id: placeholderIcon
      sourceSize.width: 48
      sourceSize.height: 48
      Layout.alignment: Qt.AlignHCenter
      source: "qrc:/rcc/icons/dashboard-large/image.svg"

      opacity: 0
      layer.enabled: true
      layer.effect: MultiEffect { colorization: 1.0; colorizationColor: "white" }

      SequentialAnimation on opacity {
        running: true
        loops: Animation.Infinite
        NumberAnimation { to: 0.55; duration: 900; easing.type: Easing.InOutSine }
        NumberAnimation { to: 0.20; duration: 900; easing.type: Easing.InOutSine }
      }
    }

    Text {
      color: "white"
      opacity: 0.55
      font.pixelSize: root.fontSize
      text: qsTr("Waiting for image…")
      Layout.alignment: Qt.AlignHCenter
      font.family: Cpp_Misc_CommonFonts.widgetFontFamily
    }
  }

  // -------------------------------------------------------------------------
  // Bottom status line — plain text directly on the image (no background box)
  // Left: resolution + format    Right: frame counter
  // -------------------------------------------------------------------------

  RowLayout {
    visible: model && model.frameCount > 0
    spacing: 4

    anchors {
      leftMargin: 10
      rightMargin: 10
      bottomMargin: 7
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }

    Text {
      color: "white"
      opacity: 0.55
      style: Text.Raised
      styleColor: Qt.rgba(0, 0, 0, 0.8)
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.85))

      text: {
        if (!model) return ""
        var parts = []
        if (model.imageWidth > 0)
          parts.push(model.imageWidth + "×" + model.imageHeight)
        if (model.imageFormat.length > 0)
          parts.push(model.imageFormat)
        return parts.join("  ·  ")
      }
    }

    Item { Layout.fillWidth: true }

    Text {
      color: "white"
      opacity: 0.55
      style: Text.Raised
      styleColor: Qt.rgba(0, 0, 0, 0.8)
      text: model ? qsTr("Frame %1").arg(model.frameCount) : ""
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.85))
    }
  }
}
