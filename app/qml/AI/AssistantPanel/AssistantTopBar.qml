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
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

//
// Assistant top bar: the chat title on the left, the provider and model pickers and the
// settings menu on the right.
//
RowLayout {
  id: root

  required property bool sidebarVisible

  signal sidebarToggled()
  signal memoryManagerRequested()

  //
  // Resolves the active chat's display title (falls back to "New chat")
  //
  function activeChatTitle() {
    const list = Cpp_AI_Assistant.chatList
    const id = Cpp_AI_Assistant.activeChatId
    for (var i = 0; i < list.length; ++i)
      if (list[i].id === id && list[i].title && list[i].title.length > 0)
        return list[i].title

    return qsTr("New chat")
  }

spacing: 8
Layout.fillWidth: true

ToolButton {
  display: AbstractButton.IconOnly
  checkable: true
  checked: root.sidebarVisible
  Layout.preferredWidth: 26
  Layout.preferredHeight: 26
  ToolTip.delay: 400
  ToolTip.visible: hovered
  ToolTip.text: qsTr("Toggle chat list")
  icon.width: 14
  icon.height: 14
  icon.color: Cpp_ThemeManager.colors["text"]
  icon.source: "qrc:/icons/buttons/sidebar.svg"
  onClicked: root.sidebarToggled()
}

//
// Active chat title (bold, elided)
//
Label {
  id: chatTitleLabel

  Layout.fillWidth: true
  elide: Label.ElideRight
  text: root.activeChatTitle()
  font: Cpp_Misc_CommonFonts.boldUiFont
  color: Cpp_ThemeManager.colors["text"]

  Connections {
    target: Cpp_AI_Assistant
    function onActiveChatChanged() { chatTitleLabel.text = root.activeChatTitle() }
    function onChatListChanged() { chatTitleLabel.text = root.activeChatTitle() }
  }
}

Widgets.Combo {
  id: providerCombo

  flat: true
  ToolTip.delay: 400
  ToolTip.visible: hovered
  Layout.preferredHeight: 26
  ToolTip.text: qsTr("Provider")
  model: Cpp_AI_Assistant.providerNames
  currentIndex: Cpp_AI_Assistant.currentProvider
  font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
  onActivated: {
    if (currentIndex === Cpp_AI_Assistant.currentProvider)
      return

    const target = currentIndex
    Cpp_AI_Assistant.requestProviderSwitch(target)
    currentIndex = Cpp_AI_Assistant.currentProvider
    modelCombo.refresh()
  }

  Connections {
    target: Cpp_AI_Assistant
    function onCurrentProviderChanged() {
      providerCombo.currentIndex = Cpp_AI_Assistant.currentProvider
    }
  }
}

Widgets.Combo {
  id: modelCombo

  property var modelIds: []

  flat: true
  valueRole: "id"
  textRole: "name"
  ToolTip.delay: 400
  ToolTip.visible: hovered
  Layout.preferredHeight: 26
  ToolTip.text: qsTr("Model selection")
  font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
  onActivated: {
    const id = modelIds[currentIndex]
    if (id)
      Cpp_AI_Assistant.setModel(Cpp_AI_Assistant.currentProvider, id)
  }

  function refresh() {
    const provider = Cpp_AI_Assistant.currentProvider
    const ids = Cpp_AI_Assistant.availableModels(provider)
    const items = []
    for (let i = 0; i < ids.length; ++i) {
      items.push({
        id: ids[i],
        name: Cpp_AI_Assistant.modelDisplayName(provider, ids[i])
      })
    }
    modelIds = ids
    model = items

    const cur = Cpp_AI_Assistant.currentModel(provider)
    const idx = ids.indexOf(cur)
    if (idx >= 0)
      currentIndex = idx
  }

  Component.onCompleted: refresh()

  Connections {
    target: Cpp_AI_Assistant
    function onCurrentProviderChanged() { modelCombo.refresh() }
  }
}

//
// Settings: assistant toggles + key manager
//
ToolButton {
  id: settingsButton

  display: AbstractButton.IconOnly
  Layout.preferredWidth: 26
  Layout.preferredHeight: 26
  ToolTip.delay: 400
  ToolTip.visible: hovered
  ToolTip.text: qsTr("Settings")
  icon.width: 14
  icon.height: 14
  icon.color: Cpp_ThemeManager.colors["text"]
  icon.source: "qrc:/icons/buttons/wrench.svg"
  onClicked: settingsMenu.popup(0, settingsButton.height)

  Menu {
    id: settingsMenu

    MenuItem {
      checkable: true
      text: qsTr("Auto-approve edits")
      checked: Cpp_AI_Assistant.autoApproveEdits
      onToggled: Cpp_AI_Assistant.autoApproveEdits = checked
      ToolTip.delay: 400
      ToolTip.visible: hovered
      ToolTip.text: qsTr("Reversible project edits run without asking. Edits are "
                       + "checkpointed to your backups folder; the project file on "
                       + "disk changes only when you save.")
    }

    MenuItem {
      id: deviceControlItem

      checkable: true
      text: qsTr("Allow device control")
      checked: Cpp_AI_Assistant.allowDeviceControl
      onToggled: Cpp_AI_Assistant.allowDeviceControl = checked

      Connections {
        target: Cpp_AI_Assistant
        function onAllowDeviceControlChanged() {
          deviceControlItem.checked = Cpp_AI_Assistant.allowDeviceControl
        }
      }
    }

    MenuSeparator {}

    MenuItem {
      id: probeToggleItem

      checkable: true
      text: qsTr("Context health check")
      checked: Cpp_AI_Assistant.contextProbeEnabled
      onToggled: Cpp_AI_Assistant.contextProbeEnabled = checked

      Connections {
        target: Cpp_AI_Assistant
        function onContextProbeEnabledChanged() {
          probeToggleItem.checked = Cpp_AI_Assistant.contextProbeEnabled
        }
      }
    }

    MenuItem {
      id: memoryToggleItem

      checkable: true
      text: qsTr("Assistant memory")
      checked: Cpp_AI_Assistant.memoryEnabled
      onToggled: Cpp_AI_Assistant.memoryEnabled = checked

      Connections {
        target: Cpp_AI_Assistant
        function onMemoryEnabledChanged() {
          memoryToggleItem.checked = Cpp_AI_Assistant.memoryEnabled
        }
      }
    }

    MenuItem {
      id: handoffToggleItem

      checkable: true
      text: qsTr("Carry context into new chats")
      checked: Cpp_AI_Assistant.handoffSeedingEnabled
      onToggled: Cpp_AI_Assistant.handoffSeedingEnabled = checked

      Connections {
        target: Cpp_AI_Assistant
        function onHandoffSeedingEnabledChanged() {
          handoffToggleItem.checked = Cpp_AI_Assistant.handoffSeedingEnabled
        }
      }
    }

    MenuItem {
      id: routingToggleItem

      checkable: true
      text: qsTr("Preload skills automatically")
      checked: Cpp_AI_Assistant.skillRoutingEnabled
      onToggled: Cpp_AI_Assistant.skillRoutingEnabled = checked

      Connections {
        target: Cpp_AI_Assistant
        function onSkillRoutingEnabledChanged() {
          routingToggleItem.checked = Cpp_AI_Assistant.skillRoutingEnabled
        }
      }
    }

    MenuItem {
      id: verifyToggleItem

      checkable: true
      text: qsTr("Verify edits automatically")
      checked: Cpp_AI_Assistant.autoVerifyEnabled
      onToggled: Cpp_AI_Assistant.autoVerifyEnabled = checked

      Connections {
        target: Cpp_AI_Assistant
        function onAutoVerifyEnabledChanged() {
          verifyToggleItem.checked = Cpp_AI_Assistant.autoVerifyEnabled
        }
      }
    }

    MenuSeparator {}

    MenuItem {
      text: qsTr("Manage memory…")
      onTriggered: root.memoryManagerRequested()
    }

    MenuItem {
      text: qsTr("Manage API keys…")
      onTriggered: Cpp_AI_Assistant.openKeyManager()
    }
  }
}
}
