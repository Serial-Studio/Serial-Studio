/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/DeepSeekProvider.h"

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
// Constants
//--------------------------------------------------------------------------------------------------

static const char* const kDeepSeekEndpoint = "https://api.deepseek.com/v1/chat/completions";

//--------------------------------------------------------------------------------------------------
// Construction and provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores QNAM ref and key getter.
 */
AI::DeepSeekProvider::DeepSeekProvider(QNetworkAccessManager& nam,
                                       std::function<QString()> keyGetter)
  : m_nam(nam), m_keyGetter(std::move(keyGetter))
{}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::DeepSeekProvider::displayName() const
{
  return QStringLiteral("DeepSeek");
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::DeepSeekProvider::keyVendorUrl() const
{
  return QStringLiteral("https://platform.deepseek.com/api_keys");
}

/**
 * @brief Returns the list of selectable DeepSeek models, default first.
 */
QStringList AI::DeepSeekProvider::availableModels() const
{
  return {
    QStringLiteral("deepseek-chat"),
    QStringLiteral("deepseek-reasoner"),
  };
}

/**
 * @brief Returns the default DeepSeek model for new sessions.
 */
QString AI::DeepSeekProvider::defaultModel() const
{
  return QStringLiteral("deepseek-chat");
}

/**
 * @brief Returns a human-friendly label for a known DeepSeek model id.
 */
QString AI::DeepSeekProvider::modelDisplayName(const QString& modelId) const
{
  if (modelId == QStringLiteral("deepseek-chat"))
    return QStringLiteral("DeepSeek V3 (chat)");

  if (modelId == QStringLiteral("deepseek-reasoner"))
    return QStringLiteral("DeepSeek R1 (reasoner)");

  return modelId;
}

/**
 * @brief Returns capability hints tuned for the active DeepSeek model.
 */
AI::ProviderCapabilities AI::DeepSeekProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.thinking              = currentModel().contains(QStringLiteral("reasoner"));
  caps.needsSmallToolSurface = currentModel().contains(QStringLiteral("chat"));
  caps.toolResultByteBudget  = caps.needsSmallToolSurface ? 4096 : 8192;
  return caps;
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Chat Completions request body and returns a streaming Reply.
 */
AI::Reply* AI::DeepSeekProvider::sendMessage(const QJsonArray& history,
                                             const QJsonArray& tools,
                                             bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new AI::ImmediateErrorReply(
      QObject::tr("No DeepSeek API key set. Open Manage Keys to add one."));

  const auto systemBlocks = ContextBuilder::buildSystemArray(false);
  const auto systemText   = ProviderJson::flattenSystemBlocks(systemBlocks);

  const auto body =
    ProviderJson::chatCompletionsBody(currentModel(), history, systemText, tools, forbidToolUse);

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "DeepSeek request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(m_nam,
                         QString::fromUtf8(kDeepSeekEndpoint),
                         QStringLiteral("Authorization"),
                         key,
                         bytes,
                         QStringLiteral("DeepSeek"));
}
