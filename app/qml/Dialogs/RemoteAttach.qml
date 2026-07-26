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
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window properties
  //
  staysOnTop: true
  preferredWidth: 460
  preferredHeight: layout.implicitHeight
  title: qsTr("Attach to Remote Dashboard")

  //
  // Connection parameters
  //
  property int rate: 20
  property int port: 7777
  property string host: "127.0.0.1"

  //
  // Remote credential: held for this dialog only, never written to disk
  //
  property string token: ""

  //
  // Human-readable reason the Attach button is unavailable, empty when it is not
  //
  readonly property string blockReason: {
    if (Cpp_API_Mirror.attached)
      return ""

    if (!Cpp_API_Mirror.canAttach)
      return qsTr("Disconnect the local device or close the open recording first.")

    return ""
  }

  //
  // Live status line: the attach failure if there is one, otherwise the link state
  //
  readonly property string statusText: {
    if (Cpp_API_Mirror.lastError.length > 0)
      return Cpp_API_Mirror.lastError

    if (!Cpp_API_Mirror.attached)
      return qsTr("Not attached.")

    if (Cpp_API_Mirror.stale)
      return qsTr("Attached to %1 - no response, the link may be down.")
             .arg(Cpp_API_Mirror.endpoint)

    if (!Cpp_API_Mirror.live)
      return qsTr("Attached to %1 - connected, the remote is not producing data.")
             .arg(Cpp_API_Mirror.endpoint)

    return qsTr("Attached to %1 - live, %2 datasets at %3 Hz.")
           .arg(Cpp_API_Mirror.endpoint)
           .arg(Cpp_API_Mirror.datasetCount)
           .arg(Cpp_API_Mirror.hz)
  }

  //
  // Splits a remembered "host:port" entry back into the two fields
  //
  function applyEndpoint(text) {
    const cut = text.lastIndexOf(":")
    if (cut <= 0)
      return

    root.host = text.substring(0, cut)
    root.port = parseInt(text.substring(cut + 1)) || 7777
  }

  //
  // Starts the attach with the current field contents
  //
  function attachNow() {
    Cpp_API_Mirror.attach(root.host, root.port, root.token, root.rate)
  }

  //
  // Dialog contents
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12

    //
    // What this dialog does, and what it does not
    //
    Label {
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: 440
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Watch another Serial Studio instance's dashboard over the network. The view is "
               + "read-only: nothing you do here reaches the remote device.")
    }

    //
    // Connection parameters
    //
    GridLayout {
      columns: 2
      rowSpacing: 6
      columnSpacing: 8
      Layout.fillWidth: true

      Label {
        text: qsTr("Recent") + ":"
        visible: recentCombo.visible
        color: Cpp_ThemeManager.colors["text"]
      }

      Widgets.Combo {
        id: recentCombo

        Layout.fillWidth: true
        model: Cpp_API_Mirror.recentEndpoints
        visible: Cpp_API_Mirror.recentEndpoints.length > 0

        onCurrentIndexChanged: {
          if (count <= 0)
            return

          root.applyEndpoint(String(currentText))
        }
      }

      Label {
        text: qsTr("Host") + ":"
        color: Cpp_ThemeManager.colors["text"]
      }

      Widgets.LineField {
        text: root.host
        Layout.fillWidth: true
        enabled: !Cpp_API_Mirror.attached
        onTextEdited: root.host = text
        placeholderText: qsTr("Host name or IP address")
      }

      Label {
        text: qsTr("Port") + ":"
        color: Cpp_ThemeManager.colors["text"]
      }

      SpinBox {
        from: 1
        to: 65535
        editable: true
        value: root.port
        Layout.fillWidth: true
        enabled: !Cpp_API_Mirror.attached
        onValueModified: root.port = value
      }

      Label {
        text: qsTr("Token") + ":"
        color: Cpp_ThemeManager.colors["text"]
      }

      Widgets.LineField {
        text: root.token
        Layout.fillWidth: true
        enabled: !Cpp_API_Mirror.attached
        onTextEdited: root.token = text
        echoMode: TextInput.PasswordEchoOnEdit
        placeholderText: qsTr("Required only for connections from another machine")
      }

      Label {
        text: qsTr("Rate") + ":"
        color: Cpp_ThemeManager.colors["text"]
      }

      SpinBox {
        to: 60
        from: 1
        editable: true
        value: root.rate
        Layout.fillWidth: true
        enabled: !Cpp_API_Mirror.attached
        onValueModified: root.rate = value
        textFromValue: (value) => qsTr("%1 Hz").arg(value)
      }
    }

    //
    // Status and refusal reason
    //
    Rectangle {
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
      Layout.preferredHeight: statusLayout.implicitHeight + 16

      ColumnLayout {
        id: statusLayout

        spacing: 4
        anchors.margins: 8
        anchors.fill: parent

        Label {
          wrapMode: Text.WordWrap
          Layout.fillWidth: true
          text: root.statusText
          font: Cpp_Misc_CommonFonts.customUiFont(0.9)
          color: Cpp_ThemeManager.colors["text"]
        }

        Label {
          opacity: 0.8
          wrapMode: Text.WordWrap
          Layout.fillWidth: true
          text: root.blockReason
          visible: root.blockReason.length > 0
          font: Cpp_Misc_CommonFonts.customUiFont(0.9)
          color: Cpp_ThemeManager.colors["text"]
        }
      }
    }

    //
    // The v1 trust model, stated where the credential is typed
    //
    Label {
      opacity: 0.8
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: 440
      font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("The link is not encrypted. The token authorizes the connection but does not "
               + "protect what travels over it, so use this on a trusted network or through a "
               + "tunnel.")
    }

    //
    // Actions
    //
    RowLayout {
      spacing: 12
      Layout.topMargin: 4
      Layout.fillWidth: true

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("Close")
        horizontalPadding: 8
        onClicked: root.close()
        font: Cpp_Misc_CommonFonts.uiFont
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        horizontalPadding: 8
        text: qsTr("Detach")
        visible: Cpp_API_Mirror.attached
        font: Cpp_Misc_CommonFonts.uiFont
        onClicked: Cpp_API_Mirror.detach()
        icon.source: "qrc:/icons/buttons/cancel.svg"
      }

      Widgets.IconButton {
        highlighted: true
        horizontalPadding: 8
        text: qsTr("Attach")
        onClicked: root.attachNow()
        font: Cpp_Misc_CommonFonts.uiFont
        visible: !Cpp_API_Mirror.attached
        icon.source: "qrc:/icons/buttons/apply.svg"
        enabled: Cpp_API_Mirror.canAttach && root.host.length > 0
      }
    }
  }
}
