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

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  minimumWidth: 620
  preferredWidth: 620
  title: qsTr("Assistant Memory")
  preferredHeight: Math.min(column.implicitHeight, 520)

  //
  // Fact rows mirrored from C++; refreshed on every mutation
  //
  property var facts: Cpp_AI_Assistant.memoryList()

  Connections {
    target: Cpp_AI_Assistant
    function onMemoryChanged() { root.facts = Cpp_AI_Assistant.memoryList() }
  }

  //
  // Layout
  //
  dialogContent: ColumnLayout {
    id: column

    spacing: 14
    anchors.fill: parent

    Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Facts the assistant carries into every chat. Stored only on this "
              + "computer. Delete anything you no longer want it to know.")
    }

    //
    // Add-a-fact row
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      ComboBox {
        id: categoryBox

        Layout.preferredWidth: 130
        model: [qsTr("user"), qsTr("feedback"), qsTr("project"), qsTr("reference")]
        onActivated: Cpp_UI_Alarms.playEvent("toggle")
      }

      Widgets.LineField {
        id: factField

        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.uiFont
        placeholderText: qsTr("Something the assistant should remember…")
      }

      Widgets.PushButton {
        text: qsTr("Remember")
        enabled: factField.text.trim().length > 0
        onClicked: {
          const stored = Cpp_AI_Assistant.addMemory(
            ["user", "feedback", "project", "reference"][categoryBox.currentIndex],
            factField.text)
          if (stored)
            factField.clear()
        }
      }
    }

    //
    // Stored facts
    //
    ScrollView {
      clip: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      ScrollBar.vertical.policy: ScrollBar.AsNeeded
      ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

      ListView {
        spacing: 6
        model: root.facts

        delegate: Rectangle {
          id: factDelegate

          required property var modelData

          radius: 6
          border.width: 1
          width: ListView.view.width
          implicitHeight: rowLayout.implicitHeight + 12
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          RowLayout {
            id: rowLayout

            spacing: 8
            anchors.margins: 6
            anchors.rightMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            Label {
              text: factDelegate.modelData.category
              font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
              color: Cpp_ThemeManager.colors["highlight"]
              Layout.preferredWidth: 72
            }

            Label {
              Layout.fillWidth: true
              wrapMode: Text.WordWrap
              textFormat: Text.PlainText
              text: factDelegate.modelData.text
              font: Cpp_Misc_CommonFonts.uiFont
              color: Cpp_ThemeManager.colors["text"]
            }

            ToolButton {
              icon.width: 14
              icon.height: 14
              ToolTip.delay: 400
              ToolTip.visible: hovered
              ToolTip.text: qsTr("Forget this fact")
              icon.color: Cpp_ThemeManager.colors["alarm"]
              icon.source: "qrc:/icons/buttons/trash.svg"
              onClicked: Cpp_AI_Assistant.deleteMemory(factDelegate.modelData.id)
            }
          }
        }
      }
    }

    Label {
      visible: root.facts.length === 0
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Qt.darker(Cpp_ThemeManager.colors["text"], 1.5)
      text: qsTr("Nothing remembered yet. Add a fact above, or approve one when the "
              + "assistant proposes it in chat.")
    }
  }
}
