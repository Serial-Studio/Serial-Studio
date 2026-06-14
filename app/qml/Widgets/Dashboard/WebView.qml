/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
import QtQuick.Layouts
import QtQuick.Controls

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
  required property WebViewWidget model

  //
  // Browser surface: WebEngineSurface.qml only exists in WITH_WEBENGINE builds
  //
  Loader {
    id: surface

    anchors.fill: parent
    active: Cpp_HasWebEngine

    Component.onCompleted: {
      if (Cpp_HasWebEngine)
        surface.setSource(
          "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/WebEngineSurface.qml",
          { "url": Qt.binding(function() { return root.model ? root.model.url : "" }) })
    }
  }

  //
  // Placeholder shown when the build has no QtWebEngine support
  //
  ColumnLayout {
    spacing: 8
    anchors.centerIn: parent
    visible: !Cpp_HasWebEngine
    width: Math.min(parent.width - 32, 420)

    Label {
      font: Cpp_Misc_CommonFonts.boldUiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Web View Unavailable")
      Layout.alignment: Qt.AlignHCenter
      horizontalAlignment: Text.AlignHCenter
    }

    Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["placeholder_text"]
      font: Cpp_Misc_CommonFonts.customUiFont(1, false)
      horizontalAlignment: Text.AlignHCenter
      text: qsTr("This build of Serial Studio was compiled without Qt WebEngine, "
                 + "so web pages cannot be displayed.")
    }
  }
}
