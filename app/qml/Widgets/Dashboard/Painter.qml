/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root

  required property color color
  required property var windowRoot
  required property string widgetId
  required property PainterWidget model

  onModelChanged: {
    if (model) {
      model.parent = container
      model.anchors.fill = container
    }
  }

  Item {
    id: container

    anchors.fill: parent
  }

  Rectangle {
    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.55)
    visible: model && !model.runtimeOk && model.lastError.length > 0

    ColumnLayout {
      spacing: 8
      anchors.centerIn: parent
      width: Math.min(parent.width - 32, 480)

      Label {
        text: qsTr("Canvas Widget Error")
        color: Cpp_ThemeManager.colors["error"]
        Layout.alignment: Qt.AlignHCenter
        font: Cpp_Misc_CommonFonts.boldUiFont
      }

      Label {
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        text: model ? model.lastError : ""
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.monoFont
        horizontalAlignment: Text.AlignHCenter
      }
    }
  }
}
