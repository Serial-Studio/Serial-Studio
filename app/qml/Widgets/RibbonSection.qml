/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

/**
 * @brief A section within a RibbonToolbar.
 *
 * Groups related buttons with a right-side separator.
 * Supports collapsing to a single icon+dropdown when the toolbar is tight.
 */
RowLayout {
  id: root

  spacing: 4
  Layout.alignment: Qt.AlignVCenter

  //
  // Collapse configuration
  //
  property bool collapsible: false
  property bool collapsed: false
  property string collapsedIcon: ""
  property string collapsedText: ""

  //
  // Priority for collapse ordering (lower = collapses first)
  //
  property int collapsePriority: 0

  //
  // Hide the right separator (for the last section)
  //
  property bool showSeparator: true

  //
  // Default content
  //
  default property alias content: contentRow.data

  //
  // Width when expanded vs collapsed (for RibbonToolbar math)
  //
  readonly property real expandedWidth: contentRow.implicitWidth + 4 + (showSeparator ? 1 : 0)
  readonly property real collapsedWidth: collapsedBtn.implicitWidth + 4 + (showSeparator ? 1 : 0)

  //
  // Content row — visible when NOT collapsed
  //
  RowLayout {
    id: contentRow

    spacing: 4
    visible: !root.collapsed
    Layout.alignment: Qt.AlignVCenter
  }

  //
  // Collapsed button
  //
  AbstractButton {
    id: collapsedBtn

    visible: root.collapsed && root.collapsible
    Layout.alignment: Qt.AlignVCenter
    implicitWidth: collapsedCol.implicitWidth + 16
    implicitHeight: collapsedCol.implicitHeight + 8
    onClicked: sectionPopup.open()

    opacity: 1

    background: Rectangle {
      radius: 4
      color: collapsedBtn.hovered
             ? Qt.rgba(Cpp_ThemeManager.colors["highlight"].r,
                       Cpp_ThemeManager.colors["highlight"].g,
                       Cpp_ThemeManager.colors["highlight"].b, 0.15)
             : "transparent"
    }

    contentItem: ColumnLayout {
      id: collapsedCol
      spacing: 4

      RowLayout {
        spacing: 4
        Layout.alignment: Qt.AlignHCenter

        Image {
          opacity: 1
          source: root.collapsedIcon
          sourceSize: Qt.size(32, 32)
          Layout.alignment: Qt.AlignVCenter
        }

        Button {
          enabled: false
          icon.width: 12
          icon.height: 12
          background: Item {}
          icon.source: "qrc:/rcc/icons/buttons/dropdown.svg"
          icon.color: Cpp_ThemeManager.colors["toolbar_text"]
        }
      }

      Label {
        id: collapsedLabel
        text: root.collapsedText
        Layout.alignment: Qt.AlignHCenter
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["toolbar_text"]
      }
    }
  }

  //
  // Popup showing the full section content when collapsed
  //
  Popup {
    id: sectionPopup

    x: collapsedBtn.x
    y: root.height
    padding: 8
    modal: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Item {
      Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: 6
        opacity: 0.95
        color: Cpp_ThemeManager.colors["toolbar_bottom"]
        border.width: 1
        border.color: Cpp_ThemeManager.colors["toolbar_border"]

        layer.enabled: true
        layer.effect: MultiEffect {
          shadowEnabled: true
          shadowColor: Qt.rgba(0, 0, 0, 0.35)
          shadowBlur: 0.5
          shadowVerticalOffset: 4
          shadowHorizontalOffset: 1
        }
      }

      Canvas {
        id: arrow

        width: 16
        height: 8
        opacity: 0.95
        x: Math.max(4, Math.min(collapsedBtn.width / 2 - 8,
                                parent.width - 20))
        y: -7

        Connections {
          target: Cpp_ThemeManager
          function onThemeChanged() { arrow.requestPaint() }
        }

        onPaint: {
          var ctx = getContext("2d")
          ctx.clearRect(0, 0, width, height)

          var bg = Cpp_ThemeManager.colors["toolbar_bottom"].toString()
          var border = Cpp_ThemeManager.colors["toolbar_border"].toString()

          // Outer border triangle
          ctx.beginPath()
          ctx.moveTo(0, height + 1)
          ctx.lineTo(width / 2, 0)
          ctx.lineTo(width, height + 1)
          ctx.closePath()
          ctx.fillStyle = border
          ctx.fill()

          // Inner fill triangle (covers border at bottom to merge with rect)
          ctx.beginPath()
          ctx.moveTo(1.5, height + 1)
          ctx.lineTo(width / 2, 1.5)
          ctx.lineTo(width - 1.5, height + 1)
          ctx.closePath()
          ctx.fillStyle = bg
          ctx.fill()
        }
      }
    }

    contentItem: RowLayout {
      id: popupContent
      spacing: 4
    }

    function connectClicks(item) {
      if (item.clicked !== undefined)
        item.clicked.connect(sectionPopup.close)

      if (item.children) {
        for (var i = 0; i < item.children.length; ++i)
          connectClicks(item.children[i])
      }
    }

    function disconnectClicks(item) {
      if (item.clicked !== undefined)
        item.clicked.disconnect(sectionPopup.close)

      if (item.children) {
        for (var i = 0; i < item.children.length; ++i)
          disconnectClicks(item.children[i])
      }
    }

    onAboutToShow: {
      while (contentRow.children.length > 0)
        contentRow.children[0].parent = popupContent

      for (var i = 0; i < popupContent.children.length; ++i)
        connectClicks(popupContent.children[i])
    }

    onClosed: {
      for (var i = 0; i < popupContent.children.length; ++i)
        disconnectClicks(popupContent.children[i])

      while (popupContent.children.length > 0)
        popupContent.children[0].parent = contentRow
    }
  }

  //
  // Right separator (can be hidden for the last section)
  //
  Rectangle {
    width: 1
    visible: root.showSeparator
    Layout.fillHeight: true
    Layout.maximumHeight: 64
    Layout.alignment: Qt.AlignVCenter
    color: Cpp_ThemeManager.colors["toolbar_separator"]
  }
}
