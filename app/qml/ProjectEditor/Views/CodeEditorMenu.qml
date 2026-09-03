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

//
// The right-click menu every embedded code editor shows. Four hand-written copies of it drifted
// apart in ordering and in which items greyed out (spec 0075 G5); this is the union of them.
//
Menu {
  id: root

  required property var codeEditor
  property bool restoreFocus: true

  onClosed: {
    if (root.restoreFocus && root.codeEditor)
      root.codeEditor.forceActiveFocus()
  }

  MenuItem {
    text: qsTr("Undo")
    opacity: enabled ? 1 : 0.5
    enabled: root.codeEditor && root.codeEditor.undoAvailable
    onTriggered: root.codeEditor.undo()
  }

  MenuItem {
    text: qsTr("Redo")
    opacity: enabled ? 1 : 0.5
    enabled: root.codeEditor && root.codeEditor.redoAvailable
    onTriggered: root.codeEditor.redo()
  }

  MenuSeparator {}

  MenuItem {
    text: qsTr("Cut")
    onTriggered: root.codeEditor.cut()
  }

  MenuItem {
    text: qsTr("Copy")
    onTriggered: root.codeEditor.copy()
  }

  MenuItem {
    text: qsTr("Paste")
    onTriggered: root.codeEditor.paste()
  }

  MenuSeparator {}

  MenuItem {
    text: qsTr("Select All")
    opacity: enabled ? 1 : 0.5
    enabled: root.codeEditor && root.codeEditor.text.length > 0
    onTriggered: root.codeEditor.selectAll()
  }

  MenuSeparator {}

  MenuItem {
    opacity: enabled ? 1 : 0.5
    text: qsTr("Format Document")
    enabled: root.codeEditor && root.codeEditor.text.length > 0
    onTriggered: root.codeEditor.formatDocument()
  }

  MenuItem {
    opacity: enabled ? 1 : 0.5
    text: qsTr("Format Selection")
    enabled: root.codeEditor && root.codeEditor.text.length > 0
    onTriggered: root.codeEditor.formatSelection()
  }
}
