/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/RequestyProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>

#include "AI/ContextBuilder.h"
#include "AI/Logging.h"
#include "AI/Providers/OpenAIProvider.h"
#include "AI/Providers/OpenAIReply.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const char* const kRequestyEndpoint = "https://router.requesty.ai/v1/chat/completions";

namespace detail {

/**
 * @brief Reply that fires errorOccurred on the next event-loop tick.
 */
class ImmediateErrorReplyRQ : public AI::Reply {
public:
  /**
   * @brief Schedules errorOccurred + finished on the next event-loop tick.
   */
  ImmediateErrorReplyRQ(const QString& message, QObject* parent = nullptr)
    : AI::Reply(parent), m_message(message)
  {
    QTimer::singleShot(0, this, [this]() {
      Q_EMIT errorOccurred(m_message);
      Q_EMIT finished();
    });
  }

  /**
   * @brief No-op since the error fires immediately.
   */
  void abort() override {}

private:
  QString m_message;
};

}  // namespace detail

//--------------------------------------------------------------------------------------------------
// Construction and provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores QNAM ref and key getter.
 */
AI::RequestyProvider::RequestyProvider(QNetworkAccessManager& nam,
                                       std::function<QString()> keyGetter)
  : m_nam(nam), m_keyGetter(std::move(keyGetter))
{}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::RequestyProvider::displayName() const
{
  return QStringLiteral("Requesty");
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::RequestyProvider::keyVendorUrl() const
{
  return QStringLiteral("https://app.requesty.ai/api-keys");
}

/**
 * @brief Returns the list of selectable Requesty models, default first.
 */
QStringList AI::RequestyProvider::availableModels() const
{
  return {
    QStringLiteral("openai/gpt-4o-mini"),
    QStringLiteral("openai/gpt-4o"),
    QStringLiteral("anthropic/claude-haiku-4.5"),
    QStringLiteral("anthropic/claude-sonnet-4.6"),
    QStringLiteral("anthropic/claude-opus-4.8"),
    QStringLiteral("google/gemini-2.5-flash"),
    QStringLiteral("deepseek/deepseek-chat"),
    QStringLiteral("mistralai/mistral-large-2411"),
    QStringLiteral("meta-llama/llama-3.3-70b-instruct"),
  };
}

/**
 * @brief Returns the default Requesty model for new sessions.
 */
QString AI::RequestyProvider::defaultModel() const
{
  return QStringLiteral("openai/gpt-4o-mini");
}

/**
 * @brief Returns a human-friendly label for a known Requesty model id.
 */
QString AI::RequestyProvider::modelDisplayName(const QString& modelId) const
{
  if (modelId == QStringLiteral("openai/gpt-4o-mini"))
    return QStringLiteral("GPT-4o mini");

  if (modelId == QStringLiteral("openai/gpt-4o"))
    return QStringLiteral("GPT-4o");

  if (modelId == QStringLiteral("anthropic/claude-haiku-4.5"))
    return QStringLiteral("Claude Haiku 4.5");

  if (modelId == QStringLiteral("anthropic/claude-sonnet-4.6"))
    return QStringLiteral("Claude Sonnet 4.6");

  if (modelId == QStringLiteral("anthropic/claude-opus-4.8"))
    return QStringLiteral("Claude Opus 4.8");

  if (modelId == QStringLiteral("google/gemini-2.5-flash"))
    return QStringLiteral("Gemini 2.5 Flash");

  if (modelId == QStringLiteral("deepseek/deepseek-chat"))
    return QStringLiteral("DeepSeek V3");

  if (modelId == QStringLiteral("mistralai/mistral-large-2411"))
    return QStringLiteral("Mistral Large");

  if (modelId == QStringLiteral("meta-llama/llama-3.3-70b-instruct"))
    return QStringLiteral("Llama 3.3 70B");

  return modelId;
}

/**
 * @brief Returns capability hints tuned for Requesty-routed models.
 */
AI::ProviderCapabilities AI::RequestyProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.needsSmallToolSurface = false;
  caps.toolResultByteBudget  = 6144;
  return caps;
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Chat Completions request body and returns a streaming Reply.
 */
AI::Reply* AI::RequestyProvider::sendMessage(const QJsonArray& history,
                                             const QJsonArray& tools,
                                             bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new detail::ImmediateErrorReplyRQ(
      QObject::tr("No Requesty API key set. Open Manage Keys to add one."));

  const auto systemBlocks = ContextBuilder::buildSystemArray(false);
  QString systemText;
  for (const auto& v : systemBlocks) {
    const auto block = v.toObject();
    const auto t     = block.value(QStringLiteral("text")).toString();
    if (!t.isEmpty()) {
      if (!systemText.isEmpty())
        systemText.append(QStringLiteral("\n\n"));

      systemText.append(t);
    }
  }

  QJsonObject body;
  body[QStringLiteral("model")]    = currentModel();
  body[QStringLiteral("stream")]   = true;
  body[QStringLiteral("messages")] = OpenAIProvider::translateHistory(history, systemText, false);
  if (!tools.isEmpty()) {
    body[QStringLiteral("tools")] = OpenAIProvider::translateTools(tools);
    body[QStringLiteral("tool_choice")] =
      forbidToolUse ? QStringLiteral("none") : QStringLiteral("auto");
  }

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "Requesty request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(m_nam,
                         QString::fromUtf8(kRequestyEndpoint),
                         QStringLiteral("Authorization"),
                         key,
                         bytes,
                         QStringLiteral("Requesty"));
}
