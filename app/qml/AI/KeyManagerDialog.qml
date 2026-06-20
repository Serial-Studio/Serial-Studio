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
  minimumWidth: 680
  preferredWidth: 680
  title: qsTr("API Keys")
  preferredHeight: Math.min(column.implicitHeight, 560)

  //
  // Provider currently shown in the dialog: tracks the active assistant provider
  //
  readonly property int providerIdx: Cpp_AI_Assistant.currentProvider

  //
  // Mirrored Q_INVOKABLE hasKey(); refreshed via keysChanged
  //
  property bool hasKey: Cpp_AI_Assistant.hasKey(providerIdx)

  Connections {
    target: Cpp_AI_Assistant
    function onKeysChanged() { root.hasKey = Cpp_AI_Assistant.hasKey(root.providerIdx) }
    function onCurrentProviderChanged() { root.hasKey = Cpp_AI_Assistant.hasKey(root.providerIdx) }
  }

  //
  // Per-provider tagline (visible under the provider name)
  //
  function providerTagline(idx) {
    if (idx === 0) {
      return qsTr("Anthropic Claude. The default is Claude Haiku 4.5 "
              + "($1 input / $5 output per million tokens). Sonnet 4.6 "
              + "and Opus 4.7 are also available. Supports streaming, "
              + "tool use, extended thinking, and prompt caching.")
    } else if (idx === 1) {
      return qsTr("OpenAI Chat Completions. The default is GPT-5 mini "
              + "for fast, cost-conscious agentic work. GPT-5.2 is the "
              + "stronger general-purpose option, and GPT-5.2 Chat "
              + "tracks the model currently used in ChatGPT.")
    } else if (idx === 2) {
      return qsTr("Google Gemini. The default is Gemini 2.0 Flash, which "
              + "has a generous free tier (subject to rate limits). "
              + "Gemini 1.5 Pro and Gemini 1.5 Flash are also available.")
    } else if (idx === 3) {
      return qsTr("DeepSeek. OpenAI-compatible API. The default is "
              + "deepseek-chat (V3). deepseek-reasoner (R1) is also "
              + "available. Often the cheapest cloud option for tool use.")
    } else if (idx === 4) {
      return qsTr("OpenRouter. One key, ~200 models from Anthropic, OpenAI, "
              + "Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier "
              + "models (suffixed :free) are rate-limited but require no "
              + "additional billing. Pay-as-you-go for the rest.")
    } else if (idx === 5) {
      return qsTr("Groq. Hardware-accelerated inference (LPUs) for very "
              + "fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen "
              + "models. Generous free tier with daily token limits.")
    } else if (idx === 6) {
      return qsTr("Mistral. The default is Mistral Large. Codestral targets "
              + "code completion, Pixtral handles vision, and the Ministral "
              + "models are tuned for edge / low-latency use.")
    } else if (idx === 7) {
      return qsTr("Local model server. Works with any OpenAI-compatible "
              + "endpoint -- Ollama, llama.cpp's llama-server, LM Studio, "
              + "or vLLM. Nothing leaves your machine. The model list is "
              + "queried live from the server.")
    } else if (idx === 8) {
      return qsTr("Requesty. One key, OpenAI-compatible router that fans out "
              + "to OpenAI, Anthropic, Google, Meta, Mistral, DeepSeek, and "
              + "others using provider/model ids (e.g. openai/gpt-4o-mini). "
              + "Pay-as-you-go.")
    }
    return ""
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
      text: qsTr("Bring your own API keys. They are encrypted at rest with "
              + "a per-machine key and never leave your computer except to "
              + "communicate with the provider you select.")
    }

    //
    // Scrollable card area: shows only the active provider's settings
    //
    ScrollView {
      id: cardScroll

      clip: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      ScrollBar.vertical.policy: ScrollBar.AsNeeded
      ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

      Rectangle {
        id: providerCard

        radius: 4
        border.width: 1
        width: cardScroll.availableWidth
        implicitHeight: providerColumn.implicitHeight + 24
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        property bool revealed: false

        ColumnLayout {
          id: providerColumn

          spacing: 8
          anchors.margins: 12
          anchors.fill: parent

          //
          // Header row: provider name + status pill
          //
          RowLayout {
            spacing: 10
            Layout.fillWidth: true

            Label {
              text: Cpp_AI_Assistant.providerNames[root.providerIdx] || ""
              font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
              color: Cpp_ThemeManager.colors["text"]
            }

            Rectangle {
              radius: 9
              implicitWidth: statusPill.implicitWidth + 14
              implicitHeight: statusPill.implicitHeight + 4
              color: root.hasKey
                     ? Qt.darker(Cpp_ThemeManager.colors["highlight"], 1.1)
                     : Qt.darker(Cpp_ThemeManager.colors["mid"], 1.05)
              border.width: 1
              border.color: root.hasKey
                           ? Cpp_ThemeManager.colors["highlight"]
                           : Cpp_ThemeManager.colors["mid"]

              Label {
                id: statusPill

                anchors.centerIn: parent
                text: root.hasKey
                      ? qsTr("Key set")
                      : qsTr("No key")
                font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
                color: Cpp_ThemeManager.colors["highlighted_text"]
                Component.onCompleted: font.capitalization = Font.AllUppercase
              }
            }

            Item { Layout.fillWidth: true }
          }

          //
          // Provider tagline / model line
          //
          Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            opacity: 0.75
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
            color: Cpp_ThemeManager.colors["text"]
            text: root.providerTagline(root.providerIdx)
          }

          //
          // Key field row: shown for cloud providers that need an API key
          //
          RowLayout {
            spacing: 6
            Layout.fillWidth: true
            visible: Cpp_AI_Assistant.requiresApiKey(root.providerIdx)

            Widgets.LineField {
              id: keyField

              Layout.fillWidth: true
              font: Cpp_Misc_CommonFonts.monoFont
              echoMode: providerCard.revealed ? TextInput.Normal : TextInput.Password
              placeholderText: root.hasKey
                               ? qsTr("A key is on file -- paste a new one to replace it")
                               : qsTr("Paste your API key here")
            }

            ToolButton {
              text: ""
              display: AbstractButton.IconOnly
              ToolTip.text: providerCard.revealed
                            ? qsTr("Hide key")
                            : qsTr("Show key while typing")
              ToolTip.visible: hovered
              ToolTip.delay: 400
              icon.color: Cpp_ThemeManager.colors["text"]
              icon.source: providerCard.revealed
                           ? "qrc:/icons/buttons/invisible.svg"
                           : "qrc:/icons/buttons/visible.svg"
              onClicked: providerCard.revealed = !providerCard.revealed
            }

            ToolButton {
              text: qsTr("Get key")
              ToolTip.text: qsTr("Open the provider's console to create a new key")
              ToolTip.visible: hovered
              ToolTip.delay: 400
              display: AbstractButton.TextBesideIcon
              font: Cpp_Misc_CommonFonts.uiFont
              icon.color: Cpp_ThemeManager.colors["text"]
              icon.source: "qrc:/icons/buttons/website.svg"
              onClicked: Qt.openUrlExternally(Cpp_AI_Assistant.keyVendorUrl(root.providerIdx))
            }

            ToolButton {
              text: qsTr("Save")
              display: AbstractButton.TextBesideIcon
              enabled: keyField.text.length > 0
              font: Cpp_Misc_CommonFonts.uiFont
              icon.color: Cpp_ThemeManager.colors["text"]
              icon.source: "qrc:/icons/buttons/save.svg"
              onClicked: {
                Cpp_AI_Assistant.setKey(root.providerIdx, keyField.text)
                keyField.clear()
                providerCard.revealed = false
                root.close()
              }
            }

            ToolButton {
              text: ""
              display: AbstractButton.IconOnly
              enabled: root.hasKey
              ToolTip.text: qsTr("Remove the stored key for %1").arg(
                              Cpp_AI_Assistant.providerNames[root.providerIdx] || "")
              ToolTip.visible: hovered
              ToolTip.delay: 400
              icon.color: Cpp_ThemeManager.colors["alarm"]
              icon.source: "qrc:/icons/buttons/trash.svg"
              onClicked: Cpp_AI_Assistant.clearKey(root.providerIdx)
            }
          }

          //
          // Local-server URL row: shown only for the Local provider
          //
          RowLayout {
            spacing: 6
            Layout.fillWidth: true
            visible: !Cpp_AI_Assistant.requiresApiKey(root.providerIdx)

            Widgets.LineField {
              id: urlField

              Layout.fillWidth: true
              font: Cpp_Misc_CommonFonts.monoFont
              text: Cpp_AI_Assistant.localBaseUrl()
              placeholderText: qsTr("http://localhost:11434/v1")
            }

            ToolButton {
              text: qsTr("Install Ollama")
              ToolTip.text: qsTr("Open the Ollama download page")
              ToolTip.visible: hovered
              ToolTip.delay: 400
              display: AbstractButton.TextBesideIcon
              font: Cpp_Misc_CommonFonts.uiFont
              icon.color: Cpp_ThemeManager.colors["text"]
              icon.source: "qrc:/icons/buttons/website.svg"
              onClicked: Qt.openUrlExternally(Cpp_AI_Assistant.keyVendorUrl(root.providerIdx))
            }

            ToolButton {
              text: qsTr("Apply")
              display: AbstractButton.TextBesideIcon
              enabled: urlField.text.length > 0
              font: Cpp_Misc_CommonFonts.uiFont
              icon.color: Cpp_ThemeManager.colors["text"]
              icon.source: "qrc:/icons/buttons/save.svg"
              onClicked: {
                Cpp_AI_Assistant.setLocalBaseUrl(urlField.text)
                root.close()
              }
            }

            ToolButton {
              text: ""
              display: AbstractButton.IconOnly
              ToolTip.text: qsTr("Re-query the model list")
              ToolTip.visible: hovered
              ToolTip.delay: 400
              icon.color: Cpp_ThemeManager.colors["text"]
              icon.source: "qrc:/icons/buttons/refresh.svg"
              onClicked: Cpp_AI_Assistant.refreshLocalModels()
            }
          }
        }
      }
    }

    //
    // Footer with summary status
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 4
      Layout.fillWidth: true

      Label {
        id: footerSummary

        Layout.fillWidth: true
        opacity: 0.6
        wrapMode: Text.WordWrap
        font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
        color: Cpp_ThemeManager.colors["text"]

        property int keysReady: 0

        function refresh() {
          let count = 0
          for (let i = 0; i < Cpp_AI_Assistant.providerNames.length; ++i)
            if (Cpp_AI_Assistant.hasKey(i)) ++count

          keysReady = count
        }

        Component.onCompleted: refresh()

        Connections {
          target: Cpp_AI_Assistant
          function onKeysChanged() { footerSummary.refresh() }
        }

        text: {
          if (keysReady === 0)
            return qsTr("No API keys configured yet. Add a key to get started.")

          if (keysReady === 1)
            return qsTr("One provider is ready.")

          return qsTr("%1 providers are ready.").arg(keysReady)
        }
      }

      DialogButtonBox {
        standardButtons: DialogButtonBox.Close
        onRejected: root.close()
      }
    }
  }
}
