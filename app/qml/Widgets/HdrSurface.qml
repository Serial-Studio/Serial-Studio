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
import QtQuick.Controls

import SerialStudio

//
// Per-window HDR output stage (spec 0089): requests the FP16 swapchain and, once granted,
// re-homes the window's content into a full-size wrapper carrying the RGBA16F transform layer.
//
Item {
  id: root

  required property Window hostWindow

  property Item contentWrapper: null

  readonly property bool active: _output.status === Loader.Ready && _output.item.active
  readonly property real headroom: _output.status === Loader.Ready ? _output.item.headroom : 1.0

  visible: false
  onActiveChanged: root.syncLayer()

  //
  // Builds or tears down the transform stage: activation creates the wrapper (which forces
  // QQuickOverlay to exist first), moves every contentItem child into it, enables the layer.
  //
  function syncLayer() {
    const content = root.hostWindow ? root.hostWindow.contentItem : null
    if (!content)
      return

    if (root.active) {
      if (!root.contentWrapper)
        root.contentWrapper = _wrapper.createObject(content)

      const wrapper = root.contentWrapper
      const moved = []
      for (let i = 0; i < content.children.length; ++i) {
        if (content.children[i] !== wrapper)
          moved.push(content.children[i])
      }

      for (let j = 0; j < moved.length; ++j)
        moved[j].parent = wrapper

      wrapper.layer.format = ShaderEffectSource.RGBA16F
      wrapper.layer.effect = _transform
      wrapper.layer.enabled = true
    } else if (root.contentWrapper) {
      const wrapper = root.contentWrapper
      wrapper.layer.enabled = false
      wrapper.layer.effect = null

      const restored = []
      for (let i = 0; i < wrapper.children.length; ++i)
        restored.push(wrapper.children[i])

      for (let j = 0; j < restored.length; ++j)
        restored[j].parent = content
    }
  }

  //
  // Swapchain request + runtime truth, gated on the launch-latched request (CONSTANT): the
  // live hdrEnabled setting is restart-applied and must never re-plumb a running window.
  //
  Loader {
    id: _output

    active: Cpp_Misc_GraphicsBackend.hdrRequested
    sourceComponent: HdrOutput {
      window: root.hostWindow
    }
  }

  //
  // Full-window host for the transformed content; reading Overlay.overlay at creation forces
  // the popup overlay into existence while the child sweep can still capture it
  //
  Component {
    id: _wrapper

    Item {
      readonly property Item popupOverlay: Overlay.overlay

      anchors.fill: parent
    }
  }

  //
  // The fullscreen output transform (app/shaders/hdr_output.frag): eotfExt per channel plus
  // the scene-referred SDR-white multiply, premultiplied-alpha aware
  //
  Component {
    id: _transform

    ShaderEffect {
      property variant source
      property real sdrWhiteScale: _output.status === Loader.Ready ? _output.item.sdrWhiteScale
                                                                   : 1.0
      fragmentShader: "qrc:/serial-studio.com/shaders/hdr_output.frag.qsb"
    }
  }
}
