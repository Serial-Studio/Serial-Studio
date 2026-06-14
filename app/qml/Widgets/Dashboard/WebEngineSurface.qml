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
import QtWebEngine
import QtQuick.Controls

//
// Chromium-backed surface for the Web View widget; only in WITH_WEBENGINE builds
//
Item {
  id: root

  property string url: ""
  property bool ready: false

  onUrlChanged: {
    root.ready = false
    if (url && url.length > 0)
      view.url = url
  }

  //
  // Spinner shown until Chromium finishes loading the page
  //
  BusyIndicator {
    anchors.centerIn: parent
    visible: !root.ready && root.url.length > 0
    running: !root.ready && root.url.length > 0
  }

  WebEngineView {
    id: view

    url: root.url
    anchors.fill: parent
    backgroundColor: "transparent"
    settings.localContentCanAccessRemoteUrls: true

    onLoadingChanged: function(loadRequest) {
      if (loadRequest.status === WebEngineView.LoadSucceededStatus)
        root.ready = true
    }

    //
    // Survive Chromium render-process death instead of crashing
    //
    onRenderProcessTerminated: function(terminationStatus, exitCode) {
      console.warn("WebView: render process terminated", terminationStatus, "exit", exitCode)
      root.ready = false
    }
  }
}
