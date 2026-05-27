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
#include <QTimer>

#include "AI/ContextBuilder.h"
#include "AI/Logging.h"
#include "AI/Providers/OpenAIProvider.h"
#include "AI/Providers/OpenAIReply.h"

namespace detail {

/**
 * @brief Reply that fires errorOccurred on the next event-loop tick.
 */
class ImmediateErrorReplyDS : public AI::Reply {
public:
  /**
   * @brief Schedules errorOccurred + finished on the next event-loop tick.
   */
  ImmediateErrorReplyDS(const QString& message, QObject* parent = nullptr)
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
    return new detail::ImmediateErrorReplyDS(
      QObject::tr("No DeepSeek API key set. Open Manage Keys to add one."));

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

  qCDebug(serialStudioAI) << "DeepSeek request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(m_nam,
                         QString::fromUtf8(kDeepSeekEndpoint),
                         QStringLiteral("Authorization"),
                         key,
                         bytes,
                         QStringLiteral("DeepSeek"));
}
