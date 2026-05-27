/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/GroqProvider.h"

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

namespace detail {

/**
 * @brief Reply that fires errorOccurred on the next event-loop tick.
 */
class ImmediateErrorReplyGQ : public AI::Reply {
public:
  /**
   * @brief Schedules errorOccurred + finished on the next event-loop tick.
   */
  ImmediateErrorReplyGQ(const QString& message, QObject* parent = nullptr)
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

static const char* const kGroqEndpoint = "https://api.groq.com/openai/v1/chat/completions";

//--------------------------------------------------------------------------------------------------
// Construction and provider metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores QNAM ref and key getter.
 */
AI::GroqProvider::GroqProvider(QNetworkAccessManager& nam, std::function<QString()> keyGetter)
  : m_nam(nam), m_keyGetter(std::move(keyGetter))
{}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::GroqProvider::displayName() const
{
  return QStringLiteral("Groq");
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::GroqProvider::keyVendorUrl() const
{
  return QStringLiteral("https://console.groq.com/keys");
}

/**
 * @brief Returns the list of selectable Groq models, default first.
 */
QStringList AI::GroqProvider::availableModels() const
{
  return {
    QStringLiteral("llama-3.3-70b-versatile"),
    QStringLiteral("llama-3.1-8b-instant"),
    QStringLiteral("openai/gpt-oss-120b"),
    QStringLiteral("openai/gpt-oss-20b"),
  };
}

/**
 * @brief Returns the default Groq model for new sessions.
 */
QString AI::GroqProvider::defaultModel() const
{
  return QStringLiteral("llama-3.3-70b-versatile");
}

/**
 * @brief Returns a human-friendly label for a known Groq model id.
 */
QString AI::GroqProvider::modelDisplayName(const QString& modelId) const
{
  if (modelId == QStringLiteral("llama-3.3-70b-versatile"))
    return QStringLiteral("Llama 3.3 70B Versatile");

  if (modelId == QStringLiteral("llama-3.1-8b-instant"))
    return QStringLiteral("Llama 3.1 8B Instant");

  if (modelId == QStringLiteral("openai/gpt-oss-120b"))
    return QStringLiteral("GPT-OSS 120B");

  if (modelId == QStringLiteral("openai/gpt-oss-20b"))
    return QStringLiteral("GPT-OSS 20B");

  return modelId;
}

/**
 * @brief Returns capability hints tuned for Groq-hosted models.
 */
AI::ProviderCapabilities AI::GroqProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.needsSmallToolSurface = true;
  caps.toolResultByteBudget  = 4096;
  return caps;
}

//--------------------------------------------------------------------------------------------------
// sendMessage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Chat Completions request body and returns a streaming Reply.
 */
AI::Reply* AI::GroqProvider::sendMessage(const QJsonArray& history,
                                         const QJsonArray& tools,
                                         bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new detail::ImmediateErrorReplyGQ(
      QObject::tr("No Groq API key set. Open Manage Keys to add one."));

  // Flatten Anthropic-shaped system blocks into a single string
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
  body[QStringLiteral("model")]  = currentModel();
  body[QStringLiteral("stream")] = true;
  body[QStringLiteral("messages")] =
    OpenAIProvider::translateHistory(history, systemText, /*useDeveloperRole=*/false);
  if (!tools.isEmpty()) {
    body[QStringLiteral("tools")] = OpenAIProvider::translateTools(tools);
    body[QStringLiteral("tool_choice")] =
      forbidToolUse ? QStringLiteral("none") : QStringLiteral("auto");
  }

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "Groq request: tools=" << tools.size() << "history=" << history.size()
                          << "bytes=" << bytes.size();

  return new OpenAIReply(m_nam,
                         QString::fromUtf8(kGroqEndpoint),
                         QStringLiteral("Authorization"),
                         key,
                         bytes,
                         QStringLiteral("Groq"));
}
