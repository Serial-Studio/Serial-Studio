/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.impl

Button {
  id: root

  padding: 4
  icon.width: iconSize
  icon.height: iconSize
  opacity: enabled ? 1 : 0.5
  icon.color: Cpp_ThemeManager.colors["button_text"]

  property int iconSize: 18
  property bool mirrorIconInRtl: icon.source.toString() !== "qrc:/icons/buttons/apply.svg"

  readonly property bool hasText: text.length > 0
  readonly property bool flip: mirrorIconInRtl && Cpp_Misc_Translator.rtl
  readonly property bool hasIcon: icon.source.toString().length > 0 || icon.name.length > 0

  contentItem: Item {
    implicitWidth: _layout.implicitWidth
    implicitHeight: _layout.implicitHeight

    RowLayout {
      id: _layout

      anchors.centerIn: parent
      spacing: root.hasIcon && root.hasText ? 6 : 0

      IconImage {
        name: root.icon.name
        visible: root.hasIcon
        color: root.icon.color
        source: root.icon.source
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: root.icon.width
        Layout.preferredHeight: root.icon.height
        sourceSize: Qt.size(root.icon.width, root.icon.height)

        transform: Scale {
          xScale: root.flip ? -1 : 1
          origin.x: root.icon.width / 2
          origin.y: root.icon.height / 2
        }
      }

      Label {
        text: root.text
        font: root.font
        visible: root.hasText
        color: root.icon.color
        Layout.alignment: Qt.AlignVCenter
      }
    }
  }
}
