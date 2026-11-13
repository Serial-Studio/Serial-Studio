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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

//
// Assistant composer: one rounded surface with the prompt field on the left and the Attach
// and Send/Cancel buttons on the right.
//
Rectangle {
  id: root

  //
  // Empties the prompt field; the conversation area calls this after sending a suggestion.
  //
  function clear() {
    composer.clear()
  }

  //
  // Sends the prompt. A message the pending attachments block stays in the field: the
  // assistant refuses it and the panel shows why.
  //
  function send() {
    if (Cpp_AI_Assistant.busy || composer.text.length === 0)
      return

    var blocked = Cpp_AI_Assistant.attachmentProblem.length > 0
    Cpp_AI_Assistant.sendMessage(composer.text)
    if (!blocked)
      composer.clear()
  }

radius: 22
border.width: 1
Layout.fillWidth: true
Layout.preferredHeight: 44
color: Cpp_ThemeManager.colors["window"]
border.color: composer.activeFocus
             ? Cpp_ThemeManager.colors["highlight"]
             : Cpp_ThemeManager.colors["groupbox_border"]

Behavior on border.color { ColorAnimation { duration: 120 } }

//
// File picker for the Attach button: each pick joins the same read allowlist as a drop
//
FileDialog {
  id: attachDialog

  title: qsTr("Attach Files")
  fileMode: FileDialog.OpenFiles
  onAccepted: {
    for (var i = 0; i < selectedFiles.length; ++i) {
      var path = selectedFiles[i].toString()
      if (Qt.platform.os !== "windows")
        path = path.replace(/^(file:\/{2})/, "")
      else
        path = path.replace(/^(file:\/{3})/, "")

      Cpp_AI_Assistant.addDroppedPath(decodeURIComponent(path))
    }
  }
}

RowLayout {
  spacing: 4
  anchors.fill: parent
  anchors.leftMargin: 16
  anchors.rightMargin: 6

  Widgets.LineField {
    id: composer

    Layout.fillWidth: true
    Layout.fillHeight: true
    background: Item {}
    verticalAlignment: TextInput.AlignVCenter
    enabled: !Cpp_AI_Assistant.busy
    font: Cpp_Misc_CommonFonts.uiFont
    color: Cpp_ThemeManager.colors["text"]
    placeholderText: qsTr("Ask Serial Studio anything…")
    placeholderTextColor: Qt.darker(Cpp_ThemeManager.colors["text"], 1.5)
    onAccepted: root.send()
  }

  //
  // Trailing Attach (icon-only): the visible door to the drag-and-drop file grant
  //
  ToolButton {
    id: attachButton

    display: AbstractButton.IconOnly
    Layout.preferredWidth: 32
    Layout.preferredHeight: 32
    Layout.alignment: Qt.AlignVCenter
    ToolTip.text: qsTr("Attach files (or drag files and folders into the chat)")
    ToolTip.visible: hovered
    ToolTip.delay: 400
    opacity: 0.85
    icon.width: 16
    icon.height: 16
    icon.color: Cpp_ThemeManager.colors["text"]
    icon.source: "qrc:/icons/buttons/attach.svg"
    onClicked: attachDialog.open()
  }

  //
  // Send / Cancel button
  //
  Widgets.IconButton {
    id: sendButton

    padding: 0
    iconSize: 14
    Layout.preferredWidth: 32
    Layout.preferredHeight: 32
    display: AbstractButton.IconOnly
    Layout.alignment: Qt.AlignVCenter

    readonly property bool canActivate:
      Cpp_AI_Assistant.busy
      || (!Cpp_AI_Assistant.busy && composer.text.length > 0)

    hoverEnabled: true
    enabled: canActivate

    icon.source: Cpp_AI_Assistant.busy
                 ? "qrc:/icons/buttons/cancel.svg"
                 : "qrc:/icons/buttons/send.svg"
    icon.color: canActivate
                ? Cpp_ThemeManager.colors["highlighted_text"]
                : Cpp_ThemeManager.colors["button_text"]

    ToolTip.text: Cpp_AI_Assistant.busy
                  ? qsTr("Stop generating")
                  : qsTr("Send message (Enter)")
    ToolTip.visible: hovered
    ToolTip.delay: 400

    background: Rectangle {
      anchors.fill: parent
      radius: width / 2
      color: sendButton.canActivate
             ? Cpp_ThemeManager.colors["highlight"]
             : Cpp_ThemeManager.colors["mid"]
      opacity: sendButton.canActivate ? 1.0 : 0.4

      Behavior on color { ColorAnimation { duration: 150 } }
      Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    onClicked: {
      if (Cpp_AI_Assistant.busy)
        Cpp_AI_Assistant.cancel()
      else
        root.send()
    }
  }
}
}
