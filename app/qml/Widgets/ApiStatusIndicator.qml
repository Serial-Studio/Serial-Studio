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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Effects
import QtQuick.Controls

// Pane caption label for the API server and remote attach state; it glows while clients are served.
Item {
  id: indicator

  readonly property bool mirrored: Cpp_API_Mirror.attached
  readonly property bool serving: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0
  readonly property bool pulsing: indicator.mirrored ? Cpp_API_Mirror.live : indicator.serving

  opacity: indicator.mirrored ? 1.0 :
             (Cpp_API_Server.enabled ? (indicator.serving ? 1 : 0.5) : 0.0)
  implicitWidth: label.implicitWidth
  implicitHeight: label.implicitHeight
  Behavior on opacity { NumberAnimation { duration: 200 } }

  MultiEffect {
    id: glow

    source: label
    shadowBlur: 2.0
    shadowEnabled: true
    anchors.fill: label
    shadowVerticalOffset: 0
    shadowHorizontalOffset: 0
    enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
    visible: Cpp_Misc_GraphicsBackend.effectsEnabled && indicator.pulsing
    shadowColor: (indicator.mirrored || Cpp_API_Server.enabled) ?
                   Cpp_ThemeManager.colors["highlight"] :
                   Cpp_ThemeManager.colors["pane_caption_border"]

    SequentialAnimation on opacity {
      loops: Animation.Infinite
      running: indicator.pulsing

      NumberAnimation {
        to: 1.00
        from: 0.4
        duration: 800
        easing.type: Easing.InOutSine
      }
      NumberAnimation {
        to: 0.4
        from: 1.00
        duration: 800
        easing.type: Easing.InOutSine
      }
    }

    SequentialAnimation on brightness {
      loops: Animation.Infinite
      running: indicator.pulsing

      NumberAnimation {
        to: 0.6
        from: 0.15
        duration: 800
        easing.type: Easing.InOutSine
      }
      NumberAnimation {
        to: 0.15
        from: 0.6
        duration: 800
        easing.type: Easing.InOutSine
      }
    }
  }

  Label {
    id: label

    visible: opacity > 0
    text: {
      if (indicator.mirrored) {
        if (Cpp_API_Mirror.stale)
          return qsTr("Remote %1 - Stale").arg(Cpp_API_Mirror.endpoint)

        if (Cpp_API_Mirror.live)
          return qsTr("Remote %1 - Live").arg(Cpp_API_Mirror.endpoint)

        return qsTr("Remote %1 - No Data").arg(Cpp_API_Mirror.endpoint)
      }

      if (!Cpp_API_Server.enabled)
        return qsTr("API Server Off")

      if (Cpp_API_Server.clientCount > 0)
        return qsTr("API Server Active (%1)").arg(Cpp_API_Server.clientCount)

      return qsTr("API Server Ready")
    }
    font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
    color: Cpp_ThemeManager.colors["pane_caption_foreground"]
  }
}
