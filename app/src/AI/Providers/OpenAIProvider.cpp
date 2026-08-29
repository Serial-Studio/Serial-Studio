/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/OpenAIProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>

#include "AI/ContextBuilder.h"
#include "AI/Logging.h"
#include "AI/Providers/ImmediateErrorReply.h"
#include "AI/Providers/OpenAIReply.h"
#include "AI/Providers/ProviderJson.h"

//--------------------------------------------------------------------------------------------------
// Construction and provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores QNAM ref and key getter.
 */
AI::OpenAIProvider::OpenAIProvider(QNetworkAccessManager& nam, KeyGetter keyGetter)
  : m_nam(nam), m_keyGetter(std::move(keyGetter))
{}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::OpenAIProvider::displayName() const
{
  return QStringLiteral("OpenAI");
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::OpenAIProvider::keyVendorUrl() const
{
  return QStringLiteral("https://platform.openai.com/api-keys");
}

/**
 * @brief Returns the list of selectable OpenAI models, default first.
 */
QStringList AI::OpenAIProvider::availableModels() const
{
  return {
    QStringLiteral("gpt-5-mini"),
    QStringLiteral("gpt-5.2"),
    QStringLiteral("gpt-5.2-chat-latest"),
    QStringLiteral("gpt-4.1"),
    QStringLiteral("gpt-4.1-mini"),
    QStringLiteral("gpt-4o"),
    QStringLiteral("gpt-4o-mini"),
  };
}

/**
 * @brief Returns the default OpenAI model for new sessions.
 */
QString AI::OpenAIProvider::defaultModel() const
{
  return QStringLiteral("gpt-5-mini");
}

/**
 * @brief Returns a human-friendly label for a known OpenAI model id.
 */
QString AI::OpenAIProvider::modelDisplayName(const QString& modelId) const
{
  if (modelId == QStringLiteral("gpt-5-mini"))
    return QStringLiteral("GPT-5 mini");

  if (modelId == QStringLiteral("gpt-5.2"))
    return QStringLiteral("GPT-5.2");

  if (modelId == QStringLiteral("gpt-5.2-chat-latest"))
    return QStringLiteral("GPT-5.2 Chat");

  if (modelId == QStringLiteral("gpt-4.1-mini"))
    return QStringLiteral("GPT-4.1 mini");

  if (modelId == QStringLiteral("gpt-4.1"))
    return QStringLiteral("GPT-4.1");

  if (modelId == QStringLiteral("gpt-4o-mini"))
    return QStringLiteral("GPT-4o mini");

  if (modelId == QStringLiteral("gpt-4o"))
    return QStringLiteral("GPT-4o");

  return modelId;
}

/**
 * @brief Returns OpenAI-specific assistant shaping hints.
 */
AI::ProviderCapabilities AI::OpenAIProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.promptCaching         = true;
  caps.parallelToolCalls     = true;
  caps.developerRole         = prefersDeveloperRole(currentModel());
  caps.needsSmallToolSurface = currentModel().contains(QStringLiteral("mini"))
                            || currentModel().contains(QStringLiteral("4o-mini"));
  caps.toolResultByteBudget = caps.needsSmallToolSurface ? 4096 : 8192;
  return caps;
}

/** @brief Returns true when the model should receive top-level instructions as a developer message.
 */
bool AI::OpenAIProvider::prefersDeveloperRole(const QString& modelId)
{
  return modelId.startsWith(QStringLiteral("gpt-5")) || modelId.startsWith(QStringLiteral("o1"))
      || modelId.startsWith(QStringLiteral("o3")) || modelId.startsWith(QStringLiteral("o4"));
}

/**
 * @brief Returns true when the model supports reasoning effort controls. Chat-tuned
 *        variants (gpt-5.x-chat-*) reject the reasoning_effort parameter outright.
 */
bool AI::OpenAIProvider::isReasoningModel(const QString& modelId)
{
  if (modelId.contains(QStringLiteral("-chat")))
    return false;

  return modelId.startsWith(QStringLiteral("gpt-5.1"))
      || modelId.startsWith(QStringLiteral("gpt-5.2")) || modelId.startsWith(QStringLiteral("o1"))
      || modelId.startsWith(QStringLiteral("o3")) || modelId.startsWith(QStringLiteral("o4"));
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Chat Completions request body and returns a streaming Reply.
 */
AI::Reply* AI::OpenAIProvider::sendMessage(const QJsonArray& history,
                                           const QJsonArray& tools,
                                           bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new AI::ImmediateErrorReply(
      QObject::tr("No OpenAI API key set. Open Manage Keys to add one."));

  const auto systemBlocks = ContextBuilder::buildSystemArray(false);
  const auto systemText   = ProviderJson::flattenSystemBlocks(systemBlocks);

  const auto model = currentModel();
  const auto caps  = capabilities();

  auto body = ProviderJson::chatCompletionsBody(
    model, history, systemText, tools, forbidToolUse, caps.developerRole);
  body[QStringLiteral("store")]                  = false;
  body[QStringLiteral("parallel_tool_calls")]    = caps.parallelToolCalls;
  body[QStringLiteral("prompt_cache_key")]       = QStringLiteral("serial-studio-ai-assistant");
  body[QStringLiteral("prompt_cache_retention")] = QStringLiteral("24h");

  if (isReasoningModel(model))
    body[QStringLiteral("reasoning_effort")] = QStringLiteral("none");

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "OpenAI request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(m_nam, key, bytes);
}
