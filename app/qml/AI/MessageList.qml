/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ListView {
  id: root

  //
  // Public properties
  //
  property var conversation: null

  //
  // Stick-to-bottom flag, updated only from user-driven scrolling (model resets can't corrupt it)
  //
  property bool stickToBottom: true

  //
  // Tool-call expansion store keyed by callId; hoisted above delegates to survive model resets
  //
  property var expandedCards: ({})

  function isCardExpanded(callId, fallback) {
    const value = expandedCards[callId]
    return value === undefined ? fallback : value
  }

  function setCardExpanded(callId, value) {
    var next = {}
    for (var key in expandedCards)
      next[key] = expandedCards[key]

    next[callId] = value
    expandedCards = next
  }

  onConversationChanged: expandedCards = ({})

  //
  // Behavior
  //
  clip: true
  spacing: 16
  cacheBuffer: 800
  reuseItems: false
  bottomMargin: 8
  topMargin: 8
  rightMargin: 12
  boundsBehavior: Flickable.StopAtBounds
  model: conversation ? conversation.messages : []

  //
  // Vertical scrollbar: pinned to the right edge, fades when idle.
  //
  ScrollBar.vertical: ScrollBar {
    id: vScroll

    minimumSize: 0.08
    policy: ScrollBar.AsNeeded
    active: hovered || pressed
    onPressedChanged: root.stickToBottom = pressed ? false : root.atYEnd
  }

  //
  // Auto-scroll to the bottom on append / model reset, but only while the
  // user has not scrolled away from the end.
  //
  onModelChanged: positionAtEnd()
  onCountChanged: positionAtEnd()

  onMovementEnded: stickToBottom = atYEnd
  onAtYEndChanged: {
    if (moving)
      stickToBottom = atYEnd
  }

  function positionAtEnd() {
    if (stickToBottom && count > 0)
      positionViewAtIndex(count - 1, ListView.End)
  }

  //
  // Delegate
  //
  delegate: MessageBubble {
    width: root.width
    role: modelData.role
    text: modelData.text
    expansionStore: root
    toolCalls: modelData.toolCalls
    thinking: modelData.thinking || ""
    streaming: modelData.streaming === true
    maxWidth: Math.min(root.width - 48, 640)
  }
}
