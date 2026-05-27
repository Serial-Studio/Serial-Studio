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
  }

  //
  // Auto-scroll to the bottom on append
  //
  onModelChanged: positionAtEnd()
  onCountChanged: positionAtEnd()

  function positionAtEnd() {
    if (count > 0)
      positionViewAtIndex(count - 1, ListView.End)
  }

  //
  // Delegate
  //
  delegate: MessageBubble {
    width: root.width
    role: modelData.role
    text: modelData.text
    thinking: modelData.thinking || ""
    streaming: modelData.streaming === true
    toolCalls: modelData.toolCalls
    maxWidth: Math.min(root.width - 48, 640)
  }
}
