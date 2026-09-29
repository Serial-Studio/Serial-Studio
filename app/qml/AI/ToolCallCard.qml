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

import "../Widgets" as Widgets

Rectangle {
  id: root

  //
  // Public properties
  //
  property string callId: ""
  property int statusValue: 0
  property string toolName: ""
  property string argsJson: ""
  property string resultJson: ""
  property var verification: null

  readonly property bool hasVerification: verification !== null
                                          && verification !== undefined
                                          && verification.ok !== undefined
  readonly property bool verifiedOk: hasVerification && verification.ok === true

  //
  // True when the parent groups this card under a batched-confirm header
  //
  property bool groupedConfirm: false

  //
  // Expansion state lives in expansionStore (keyed by callId) so it survives model resets
  //
  property var expansionStore: null
  property bool localExpanded: false
  readonly property bool expanded: expansionStore
                                   ? expansionStore.isCardExpanded(callId, localExpanded)
                                   : localExpanded

  function toggleExpanded() {
    if (expansionStore)
      expansionStore.setCardExpanded(callId, !expanded)
    else
      localExpanded = !localExpanded
  }

  //
  // Status enum (matches Conversation::CallStatus)
  //
  readonly property int kDone: 2
  readonly property int kError: 3
  readonly property int kDenied: 4
  readonly property int kRunning: 0
  readonly property int kBlocked: 5
  readonly property int kAwaitingConfirm: 1

  //
  // Status pill styling
  //
  readonly property string statusLabel: {
    switch (root.statusValue) {
      case root.kAwaitingConfirm: return qsTr("Awaiting approval")
      case root.kDone:            return qsTr("Done")
      case root.kError:           return qsTr("Error")
      case root.kDenied:          return qsTr("Denied")
      case root.kBlocked:         return qsTr("Blocked")
      default:                    return qsTr("Running")
    }
  }

  readonly property color statusColor: {
    switch (root.statusValue) {
      case root.kDone:            return Cpp_ThemeManager.colors["highlight"]
      case root.kError:           return Cpp_ThemeManager.colors["alarm"]
      case root.kBlocked:         return Cpp_ThemeManager.colors["alarm"]
      case root.kDenied:          return Cpp_ThemeManager.colors["mid"]
      case root.kAwaitingConfirm: return Cpp_ThemeManager.colors["accent"]
      default:                    return Cpp_ThemeManager.colors["mid"]
    }
  }

  //
  // Geometry / appearance
  //
  radius: 4
  border.width: 1
  implicitWidth: column.implicitWidth + 16
  implicitHeight: column.implicitHeight + 16
  color: Cpp_ThemeManager.colors["groupbox_background"]
  border.color: Cpp_ThemeManager.colors["groupbox_border"]

  ColumnLayout {
    id: column

    spacing: 6
    anchors.margins: 8
    anchors.fill: parent

    //
    // Header (clickable to toggle expand)
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Image {
        sourceSize.width: 12
        sourceSize.height: 12
        Layout.preferredWidth: 12
        Layout.preferredHeight: 12
        Layout.alignment: Qt.AlignVCenter
        fillMode: Image.PreserveAspectFit
        rotation: root.expanded ? 0 : 270
        source: Cpp_Misc_IconRegistry.icon("editor", "indicator", 16)

        Behavior on rotation { NumberAnimation { duration: 120 } }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor
          onClicked: root.toggleExpanded()
        }
      }

      Label {
        Layout.fillWidth: true
        text: root.toolName
        elide: Text.ElideMiddle
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["text"]

        MouseArea {
          anchors.fill: parent
          onClicked: root.toggleExpanded()
        }
      }

      Rectangle {
        radius: 2
        border.width: 1
        visible: root.hasVerification
        implicitWidth: verifyPill.implicitWidth + 12
        implicitHeight: verifyPill.implicitHeight + 4
        border.color: root.verifiedOk
                      ? Cpp_ThemeManager.colors["highlight"]
                      : Cpp_ThemeManager.colors["alarm"]
        color: Qt.darker(root.verifiedOk
                         ? Cpp_ThemeManager.colors["highlight"]
                         : Cpp_ThemeManager.colors["alarm"], 1.4)

        Label {
          id: verifyPill

          anchors.centerIn: parent
          text: root.verifiedOk ? qsTr("Verified") : qsTr("Verify failed")
          font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
          color: Cpp_ThemeManager.colors["highlighted_text"]
        }
      }

      Rectangle {
        radius: 2
        border.width: 1
        border.color: root.statusColor
        implicitWidth: pill.implicitWidth + 12
        implicitHeight: pill.implicitHeight + 4
        color: Qt.darker(root.statusColor, 1.4)

        Label {
          id: pill

          anchors.centerIn: parent
          text: root.statusLabel
          font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
          color: Cpp_ThemeManager.colors["highlighted_text"]
        }
      }
    }

    //
    // Failed-verification detail + recovery (restore stays behind the approval card)
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      visible: root.hasVerification && !root.verifiedOk

      Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["alarm"]
        text: root.verification && root.verification.detail
              ? qsTr("Verification failed: %1").arg(root.verification.detail)
              : qsTr("The applied change failed its verification check.")
      }

      //
      // The instruction is deliberately untranslated: the tool names are literal and the
      // recovery flow must not vary by UI locale.
      //
      Widgets.PushButton {
        enabled: !Cpp_AI_Assistant.busy
        text: qsTr("Restore checkpoint…")
        font: Cpp_Misc_CommonFonts.uiFont
        onClicked: Cpp_AI_Assistant.sendMessage(
          "The last edit failed verification. Call assistant.listCheckpoints, then "
          + "restore the most recent checkpoint with assistant.restore and wait for "
          + "my approval.")
      }
    }

    //
    // Confirmation buttons
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      visible: root.statusValue === root.kAwaitingConfirm
               && !root.groupedConfirm

      Widgets.PushButton {
        text: qsTr("Approve")
        font: Cpp_Misc_CommonFonts.uiFont
        onClicked: Cpp_AI_Assistant.approveToolCall(root.callId)
      }

      Widgets.PushButton {
        text: qsTr("Deny")
        font: Cpp_Misc_CommonFonts.uiFont
        onClicked: Cpp_AI_Assistant.denyToolCall(root.callId)
      }

      Item { Layout.fillWidth: true }
    }

    //
    // Expanded body: args + result
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: root.expanded

      Label {
        text: qsTr("Arguments")
        font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      Rectangle {
        radius: 2
        Layout.fillWidth: true
        implicitHeight: argsView.implicitHeight + 8
        color: Cpp_ThemeManager.colors["base"]
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        TextEdit {
          id: argsView

          readOnly: true
          selectByMouse: true
          wrapMode: TextEdit.Wrap
          anchors.margins: 4
          anchors.fill: parent
          text: root.argsJson
          font: Cpp_Misc_CommonFonts.monoFont
          color: Cpp_ThemeManager.colors["text"]

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.IBeamCursor
            acceptedButtons: Qt.RightButton
            onClicked: function(mouse) { argsMenu.popup(mouse.x, mouse.y) }
          }

          Menu {
            id: argsMenu

            MenuItem {
              text: qsTr("Copy")
              enabled: argsView.selectedText.length > 0
              onTriggered: argsView.copy()
            }
            MenuItem {
              text: qsTr("Copy All")
              onTriggered: {
                argsView.selectAll()
                argsView.copy()
                argsView.deselect()
              }
            }
            MenuItem {
              text: qsTr("Select All")
              onTriggered: argsView.selectAll()
            }
          }
        }
      }

      Label {
        text: qsTr("Result")
        font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        visible: root.resultJson.length > 0
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      Rectangle {
        radius: 2
        Layout.fillWidth: true
        implicitHeight: resultView.implicitHeight + 8
        visible: root.resultJson.length > 0
        color: Cpp_ThemeManager.colors["base"]
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        TextEdit {
          id: resultView

          readOnly: true
          selectByMouse: true
          wrapMode: TextEdit.Wrap
          anchors.margins: 4
          anchors.fill: parent
          text: root.resultJson
          font: Cpp_Misc_CommonFonts.monoFont
          color: Cpp_ThemeManager.colors["text"]

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.IBeamCursor
            acceptedButtons: Qt.RightButton
            onClicked: function(mouse) { resultMenu.popup(mouse.x, mouse.y) }
          }

          Menu {
            id: resultMenu

            MenuItem {
              text: qsTr("Copy")
              enabled: resultView.selectedText.length > 0
              onTriggered: resultView.copy()
            }
            MenuItem {
              text: qsTr("Copy All")
              onTriggered: {
                resultView.selectAll()
                resultView.copy()
                resultView.deselect()
              }
            }
            MenuItem {
              text: qsTr("Select All")
              onTriggered: resultView.selectAll()
            }
          }
        }
      }
    }
  }
}
