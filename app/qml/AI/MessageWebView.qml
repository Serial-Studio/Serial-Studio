/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtWebEngine
import QtWebChannel

//
// Chromium-backed transcript; HTML-to-QML IPC via QWebChannel.
//
Item {
  id: root

  property bool ready: false
  signal chipClicked(string text)

  function pushTheme() {
    if (!ready)
      return

    var base = Cpp_HelpCenter.themeColors
    if (!base || base.length === 0)
      return

    var extras = {
      alarm:   Cpp_ThemeManager.colors["alarm"]              || "",
      mid:     Cpp_ThemeManager.colors["mid"]                || "",
      section: Cpp_ThemeManager.colors["pane_section_label"] || ""
    }

    view.runJavaScript(
      "setTheme(Object.assign(" + base + ", " + JSON.stringify(extras) + "));")
    pushMessages()
  }

  function pushDirection() {
    if (!ready)
      return

    view.runJavaScript(
      "setDirection(" + (Cpp_Misc_Translator.rtl ? "true" : "false") + ");")
    pushMessages()
  }

  function pushL10N() {
    if (!ready)
      return

    var src = {
      you:           qsTr("You"),
      assistant:     qsTr("Assistant"),
      error:         qsTr("Error"),
      discovery:     qsTr("Discovery"),
      execution:     qsTr("Execution"),
      running:       qsTr("Running"),
      awaiting:      qsTr("Awaiting approval"),
      done:          qsTr("Done"),
      errStatus:     qsTr("Error"),
      denied:        qsTr("Denied"),
      blocked:       qsTr("Blocked"),
      approve:       qsTr("Approve"),
      deny:          qsTr("Deny"),
      approveAll:    qsTr("Approve all"),
      denyAll:       qsTr("Deny all"),
      arguments:     qsTr("Arguments"),
      result:        qsTr("Result"),
      groupTitle:    qsTr("Approve {n} actions in {family}?"),
      groupSubtitle: qsTr("These calls will run together. Expand each card to inspect arguments."),
      copy:          qsTr("Copy")
    }
    view.runJavaScript("setL10N(" + JSON.stringify(src) + ");")
  }

  function pushMessages() {
    if (!ready)
      return

    var conv = Cpp_AI_Assistant.conversation
    var msgs = conv ? conv.messages : []
    var json = JSON.stringify(msgs || [])
    view.runJavaScript("setMessages(" + json + ");")
  }

  Timer {
    id: pushTimer

    interval: 50
    repeat: false
    onTriggered: root.pushMessages()
  }

  Connections {
    target: Cpp_AI_Assistant.conversation
    function onMessagesChanged() { pushTimer.restart() }
  }

  Connections {
    target: Cpp_HelpCenter
    function onThemeColorsChanged() { root.pushTheme() }
  }

  Connections {
    target: Cpp_Misc_Translator
    function onLanguageChanged() {
      root.pushDirection()
      root.pushL10N()
    }
  }

  //
  // IPC bridge exposed to the page as `window.ai`.
  //
  QtObject {
    id: aiBridge

    WebChannel.id: "ai"

    function approve(callId)         { Cpp_AI_Assistant.approveToolCall(callId) }
    function deny(callId)            { Cpp_AI_Assistant.denyToolCall(callId) }
    function approveAll(family)      { Cpp_AI_Assistant.approveToolCallGroup(family) }
    function denyAll(family)         { Cpp_AI_Assistant.denyToolCallGroup(family) }
    function copy(text)              { Cpp_Misc_Utilities.copyText(text) }
    function chip(text)              { root.chipClicked(text) }
    function ext(url)                { Qt.openUrlExternally(url) }
  }

  WebChannel {
    id: aiChannel

    registeredObjects: [aiBridge]
  }

  WebEngineView {
    id: view

    anchors.fill: parent
    webChannel: aiChannel
    url: "qrc:/chat-viewer.html"
    backgroundColor: "transparent"
    settings.localContentCanAccessRemoteUrls: true

    onLoadingChanged: function(loadRequest) {
      if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
        root.ready = true
        root.pushTheme()
        root.pushDirection()
        root.pushL10N()
        root.pushMessages()
      }
    }

    onRenderProcessTerminated: function(terminationStatus, exitCode) {
      console.warn("MessageWebView: render process terminated",
                   terminationStatus, "exit", exitCode)
      root.ready = false
    }

    onNavigationRequested: function(request) {
      var url = request.url.toString()

      if (url.startsWith("qrc:"))
        return

      request.reject()

      if (url.startsWith("http://") || url.startsWith("https://") ||
          url.startsWith("mailto:"))
        Qt.openUrlExternally(url)
    }
  }
}
