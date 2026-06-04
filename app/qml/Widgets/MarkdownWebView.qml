/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
// Chromium-backed markdown viewer. Loaded only when Cpp_HasWebEngine is true,
// so the QtWebEngine import is never resolved on builds without WebEngine.
//
Item {
  id: root

  property bool ready: false
  property string markdown: ""
  property bool emitCopyToast: true
  property string copyToastText: qsTr("Copied to Clipboard")

  signal copyRequested(string text)
  signal navigationRejected(string url)

  function pushTheme() {
    if (!ready)
      return

    var json = Cpp_HelpCenter.themeColors
    view.runJavaScript("setTheme(" + json + ");")
  }

  function pushContent() {
    if (!ready)
      return

    var md = root.markdown
    if (!md || md === "") {
      view.runJavaScript("document.getElementById('content').innerHTML = '';")
      return
    }

    var escaped = md.replace(/\\/g, '\\\\')
                    .replace(/`/g, '\\`')
                    .replace(/\$/g, '\\$')
    view.runJavaScript("renderMarkdown(`" + escaped + "`);")
  }

  onMarkdownChanged: pushContent()

  Connections {
    target: Cpp_HelpCenter
    function onThemeColorsChanged() {
      root.pushTheme()
    }
  }

  //
  // Spinner shown until Chromium finishes loading the viewer page. The view fades via
  // opacity (never visible: false) so layout constraints hold and the page loads eagerly.
  //
  BusyIndicator {
    running: !root.ready
    visible: !root.ready
    anchors.centerIn: parent
  }

  WebEngineView {
    id: view

    anchors.fill: parent
    backgroundColor: "transparent"
    url: "qrc:/markdown-viewer.html"
    settings.localContentCanAccessRemoteUrls: true

    opacity: root.ready ? 1 : 0

    Behavior on opacity {
      NumberAnimation {
        duration: 150
      }
    }

    onLoadingChanged: function(loadRequest) {
      if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
        root.ready = true
        root.pushTheme()
        root.pushContent()
      }
    }

    //
    // Survive Chromium render-process death instead of crashing.
    //
    onRenderProcessTerminated: function(terminationStatus, exitCode) {
      console.warn("MarkdownWebView: render process terminated",
                   terminationStatus, "exit", exitCode)
      root.ready = false
    }

    onNavigationRequested: function(request) {
      var url = request.url.toString()

      if (url.startsWith("qrc:"))
        return

      if (url.startsWith("ext:")) {
        request.reject()
        Qt.openUrlExternally(url.substring(4))
        return
      }

      if (url.startsWith("copy:")) {
        request.reject()
        var text = decodeURIComponent(url.substring(5))
        Cpp_Misc_Utilities.copyText(text)
        if (root.emitCopyToast)
          root.copyRequested(text)

        return
      }

      if (url.startsWith("nav:")) {
        request.reject()
        root.navigationRejected(url.substring(4))
        return
      }

      request.reject()
      Qt.openUrlExternally(url)
    }
  }
}
