/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/OpenAICompatibleProvider.h"

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
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Vendor table
//--------------------------------------------------------------------------------------------------

/**
 * @brief DeepSeek: the reasoner model streams thinking, the chat model wants the small surface.
 */
AI::OpenAICompatibleVendor AI::OpenAICompatibleProvider::deepSeek()
{
  OpenAICompatibleVendor vendor;
  vendor.name               = QStringLiteral("DeepSeek");
  vendor.endpoint           = QStringLiteral("https://api.deepseek.com/v1/chat/completions");
  vendor.keyUrl             = QStringLiteral("https://platform.deepseek.com/api_keys");
  vendor.thinkingMarker     = QStringLiteral("reasoner");
  vendor.smallSurfaceMarker = QStringLiteral("chat");
  vendor.modelIds           = {
    QStringLiteral("deepseek-chat"),
    QStringLiteral("deepseek-reasoner"),
  };
  vendor.modelLabels = {
    QStringLiteral("DeepSeek V3 (chat)"),
    QStringLiteral("DeepSeek R1 (reasoner)"),
  };

  return vendor;
}

/**
 * @brief Groq: every hosted model wants the small tool surface.
 */
AI::OpenAICompatibleVendor AI::OpenAICompatibleProvider::groq()
{
  OpenAICompatibleVendor vendor;
  vendor.name               = QStringLiteral("Groq");
  vendor.endpoint           = QStringLiteral("https://api.groq.com/openai/v1/chat/completions");
  vendor.keyUrl             = QStringLiteral("https://console.groq.com/keys");
  vendor.alwaysSmallSurface = true;
  vendor.modelIds           = {
    QStringLiteral("llama-3.3-70b-versatile"),
    QStringLiteral("llama-3.1-8b-instant"),
    QStringLiteral("openai/gpt-oss-120b"),
    QStringLiteral("openai/gpt-oss-20b"),
  };
  vendor.modelLabels = {
    QStringLiteral("Llama 3.3 70B Versatile"),
    QStringLiteral("Llama 3.1 8B Instant"),
    QStringLiteral("GPT-OSS 120B"),
    QStringLiteral("GPT-OSS 20B"),
  };

  return vendor;
}

/**
 * @brief Mistral: only the "small" tier wants the reduced tool surface.
 */
AI::OpenAICompatibleVendor AI::OpenAICompatibleProvider::mistral()
{
  OpenAICompatibleVendor vendor;
  vendor.name               = QStringLiteral("Mistral");
  vendor.endpoint           = QStringLiteral("https://api.mistral.ai/v1/chat/completions");
  vendor.keyUrl             = QStringLiteral("https://console.mistral.ai/api-keys");
  vendor.smallSurfaceMarker = QStringLiteral("small");
  vendor.modelIds           = {
    QStringLiteral("mistral-large-latest"),
    QStringLiteral("mistral-medium-latest"),
    QStringLiteral("mistral-small-latest"),
    QStringLiteral("ministral-8b-latest"),
    QStringLiteral("ministral-3b-latest"),
    QStringLiteral("codestral-latest"),
    QStringLiteral("pixtral-large-latest"),
    QStringLiteral("open-mistral-nemo"),
  };
  vendor.modelLabels = {
    QStringLiteral("Mistral Large"),
    QStringLiteral("Mistral Medium"),
    QStringLiteral("Mistral Small"),
    QStringLiteral("Ministral 8B"),
    QStringLiteral("Ministral 3B"),
    QStringLiteral("Codestral"),
    QStringLiteral("Pixtral Large"),
    QStringLiteral("Mistral Nemo"),
  };

  return vendor;
}

/**
 * @brief OpenRouter: a router in front of many vendors, so no per-model hint applies and the tool
 *        result budget sits between the small and the full one.
 */
AI::OpenAICompatibleVendor AI::OpenAICompatibleProvider::openRouter()
{
  OpenAICompatibleVendor vendor;
  vendor.name            = QStringLiteral("OpenRouter");
  vendor.endpoint        = QStringLiteral("https://openrouter.ai/api/v1/chat/completions");
  vendor.keyUrl          = QStringLiteral("https://openrouter.ai/keys");
  vendor.toolResultBytes = 6144;
  vendor.modelIds        = {
    QStringLiteral("anthropic/claude-haiku-4.5"),
    QStringLiteral("anthropic/claude-sonnet-4.6"),
    QStringLiteral("anthropic/claude-opus-4.8"),
    QStringLiteral("openai/gpt-5-mini"),
    QStringLiteral("openai/gpt-5.2"),
    QStringLiteral("google/gemini-2.5-flash"),
    QStringLiteral("meta-llama/llama-3.3-70b-instruct"),
    QStringLiteral("meta-llama/llama-3.3-70b-instruct:free"),
    QStringLiteral("deepseek/deepseek-chat"),
    QStringLiteral("mistralai/mistral-large-2411"),
    QStringLiteral("qwen/qwen-2.5-72b-instruct"),
  };
  vendor.modelLabels = {
    QStringLiteral("Claude Haiku 4.5"),
    QStringLiteral("Claude Sonnet 4.6"),
    QStringLiteral("Claude Opus 4.8"),
    QStringLiteral("GPT-5 mini"),
    QStringLiteral("GPT-5.2"),
    QStringLiteral("Gemini 2.5 Flash"),
    QStringLiteral("Llama 3.3 70B"),
    QStringLiteral("Llama 3.3 70B (free)"),
    QStringLiteral("DeepSeek V3"),
    QStringLiteral("Mistral Large"),
    QStringLiteral("Qwen 2.5 72B"),
  };

  return vendor;
}

//--------------------------------------------------------------------------------------------------
// Construction and provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the QNAM reference, the key getter and the vendor description.
 */
AI::OpenAICompatibleProvider::OpenAICompatibleProvider(QNetworkAccessManager& nam,
                                                       std::function<QString()> keyGetter,
                                                       OpenAICompatibleVendor vendor)
  : m_nam(nam), m_keyGetter(std::move(keyGetter)), m_vendor(std::move(vendor))
{
  SS_ASSERT_LOG(!m_vendor.modelIds.isEmpty());
  SS_ASSERT_LOG(m_vendor.modelIds.size() == m_vendor.modelLabels.size());
}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::OpenAICompatibleProvider::displayName() const
{
  return m_vendor.name;
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::OpenAICompatibleProvider::keyVendorUrl() const
{
  return m_vendor.keyUrl;
}

/**
 * @brief Returns the selectable model ids, default first.
 */
QStringList AI::OpenAICompatibleProvider::availableModels() const
{
  return m_vendor.modelIds;
}

/**
 * @brief Returns the default model for new sessions, which is the first row of the roster.
 */
QString AI::OpenAICompatibleProvider::defaultModel() const
{
  SS_ASSERT(!m_vendor.modelIds.isEmpty(), return {});
  return m_vendor.modelIds.first();
}

/**
 * @brief Returns the label at the model's row, or the id itself for a model the roster does not
 *        carry (a project or setting written by a newer build).
 */
QString AI::OpenAICompatibleProvider::modelDisplayName(const QString& modelId) const
{
  const auto row = m_vendor.modelIds.indexOf(modelId);
  if (row < 0 || row >= m_vendor.modelLabels.size())
    return modelId;

  return m_vendor.modelLabels.at(row);
}

/**
 * @brief Returns capability hints tuned for the active model: the two vendor markers decide
 *        whether it streams thinking and whether it needs the reduced tool surface.
 */
AI::ProviderCapabilities AI::OpenAICompatibleProvider::capabilities() const
{
  const auto model = currentModel();

  ProviderCapabilities caps;
  caps.thinking = !m_vendor.thinkingMarker.isEmpty() && model.contains(m_vendor.thinkingMarker);
  caps.needsSmallToolSurface =
    m_vendor.alwaysSmallSurface
    || (!m_vendor.smallSurfaceMarker.isEmpty() && model.contains(m_vendor.smallSurfaceMarker));
  caps.toolResultByteBudget =
    caps.needsSmallToolSurface ? m_vendor.smallToolResultBytes : m_vendor.toolResultBytes;

  return caps;
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Chat Completions request body and returns a streaming Reply.
 */
AI::Reply* AI::OpenAICompatibleProvider::sendMessage(const QJsonArray& history,
                                                     const QJsonArray& tools,
                                                     bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new AI::ImmediateErrorReply(
      QObject::tr("No %1 API key set. Open Manage Keys to add one.").arg(m_vendor.name));

  const auto systemBlocks = ContextBuilder::buildSystemArray(false);
  const auto systemText   = ProviderJson::flattenSystemBlocks(systemBlocks);

  const auto body =
    ProviderJson::chatCompletionsBody(currentModel(), history, systemText, tools, forbidToolUse);

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << m_vendor.name << "request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(
    m_nam, m_vendor.endpoint, QStringLiteral("Authorization"), key, bytes, m_vendor.name);
}
