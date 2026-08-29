/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/MistralProvider.h"

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

static const char* const kMistralEndpoint = "https://api.mistral.ai/v1/chat/completions";

//--------------------------------------------------------------------------------------------------
// Construction and provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores QNAM ref and key getter.
 */
AI::MistralProvider::MistralProvider(QNetworkAccessManager& nam, std::function<QString()> keyGetter)
  : m_nam(nam), m_keyGetter(std::move(keyGetter))
{}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::MistralProvider::displayName() const
{
  return QStringLiteral("Mistral");
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::MistralProvider::keyVendorUrl() const
{
  return QStringLiteral("https://console.mistral.ai/api-keys");
}

/**
 * @brief Returns the list of selectable Mistral models, default first.
 */
QStringList AI::MistralProvider::availableModels() const
{
  return {
    QStringLiteral("mistral-large-latest"),
    QStringLiteral("mistral-medium-latest"),
    QStringLiteral("mistral-small-latest"),
    QStringLiteral("ministral-8b-latest"),
    QStringLiteral("ministral-3b-latest"),
    QStringLiteral("codestral-latest"),
    QStringLiteral("pixtral-large-latest"),
    QStringLiteral("open-mistral-nemo"),
  };
}

/**
 * @brief Returns the default Mistral model for new sessions.
 */
QString AI::MistralProvider::defaultModel() const
{
  return QStringLiteral("mistral-large-latest");
}

/**
 * @brief Returns a human-friendly label for a known Mistral model id.
 */
QString AI::MistralProvider::modelDisplayName(const QString& modelId) const
{
  if (modelId == QStringLiteral("mistral-large-latest"))
    return QStringLiteral("Mistral Large");

  if (modelId == QStringLiteral("mistral-medium-latest"))
    return QStringLiteral("Mistral Medium");

  if (modelId == QStringLiteral("mistral-small-latest"))
    return QStringLiteral("Mistral Small");

  if (modelId == QStringLiteral("ministral-8b-latest"))
    return QStringLiteral("Ministral 8B");

  if (modelId == QStringLiteral("ministral-3b-latest"))
    return QStringLiteral("Ministral 3B");

  if (modelId == QStringLiteral("codestral-latest"))
    return QStringLiteral("Codestral");

  if (modelId == QStringLiteral("pixtral-large-latest"))
    return QStringLiteral("Pixtral Large");

  if (modelId == QStringLiteral("open-mistral-nemo"))
    return QStringLiteral("Mistral Nemo");

  return modelId;
}

/**
 * @brief Returns capability hints tuned for the active Mistral model.
 */
AI::ProviderCapabilities AI::MistralProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.needsSmallToolSurface = currentModel().contains(QStringLiteral("small"));
  caps.toolResultByteBudget  = caps.needsSmallToolSurface ? 4096 : 8192;
  return caps;
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Chat Completions request body and returns a streaming Reply.
 */
AI::Reply* AI::MistralProvider::sendMessage(const QJsonArray& history,
                                            const QJsonArray& tools,
                                            bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new AI::ImmediateErrorReply(
      QObject::tr("No Mistral API key set. Open Manage Keys to add one."));

  const auto systemBlocks = ContextBuilder::buildSystemArray(false);
  const auto systemText   = ProviderJson::flattenSystemBlocks(systemBlocks);

  const auto body =
    ProviderJson::chatCompletionsBody(currentModel(), history, systemText, tools, forbidToolUse);

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "Mistral request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(m_nam,
                         QString::fromUtf8(kMistralEndpoint),
                         QStringLiteral("Authorization"),
                         key,
                         bytes,
                         QStringLiteral("Mistral"));
}
