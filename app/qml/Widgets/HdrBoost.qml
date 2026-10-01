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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

import SerialStudio

//
// Emissive boost primitive (spec 0089): re-renders a sibling item with its linear light
// multiplied by the HDR intensity; inert (nothing instantiated) on any non-HDR window.
//
Item {
  id: root

  required property Item target

  property bool active: true
  property real boost: Cpp_Misc_GraphicsBackend.hdrAutoIntensity

  //
  // Widens the capture rect past the target's bounds, for targets that paint outside
  // themselves (a MultiEffect's padded blur); the consumer sizes this item to match.
  //
  property real captureMargin: 0

  readonly property var hostWindow: root.Window.window
  readonly property real windowHeadroom: (root.hostWindow
                                          && root.hostWindow.hdrHeadroom !== undefined)
                                         ? root.hostWindow.hdrHeadroom : 1.0
  readonly property bool effectOn: root.active
                                   && root.hostWindow !== null
                                   && root.hostWindow.hdrActive === true
                                   && Math.min(root.boost, root.windowHeadroom) > 1.0

  //
  // A ShaderEffectSource captures content, not the source item's own opacity; mirror it here
  // so a translucent target (band stripes, arcs) keeps its translucency when boosted
  //
  opacity: root.target ? root.target.opacity : 1.0

  //
  // Geometry is the consumer's: anchors.fill the target when it is a sibling, or bind
  // x/y/width/height through the positioner offsets (the LEDPanel MultiEffect pattern).
  //
  // The boosted re-render; hideSource is load-bearing (a stacked overlay would double-blend
  // translucent sources), and deactivating the Loader restores the source item.
  //
  Loader {
    anchors.fill: parent
    active: root.effectOn

    sourceComponent: ShaderEffect {
      property var source: ShaderEffectSource {
        live: true
        hideSource: true
        sourceItem: root.target
        sourceRect: Qt.rect(-root.captureMargin,
                            -root.captureMargin,
                            root.target.width + 2 * root.captureMargin,
                            root.target.height + 2 * root.captureMargin)
      }
      property real boost: Math.min(root.boost, root.windowHeadroom)
      fragmentShader: "qrc:/serial-studio.com/shaders/hdr_boost.frag.qsb"
    }
  }
}
