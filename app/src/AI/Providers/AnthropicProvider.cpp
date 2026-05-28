/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/AnthropicProvider.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>

#include "AI/ContextBuilder.h"
#include "AI/Logging.h"
#include "AI/Providers/AnthropicReply.h"

namespace AI::detail {

/**
 * @brief Reply that fires errorOccurred on the next event-loop tick.
 *
 * Used when sendMessage cannot reach the network at all (e.g. no key).
 * Keeps the caller's signal-handling code uniform.
 */
class ImmediateErrorReply : public AI::Reply {
public:
  /**
   * @brief Schedules an error+finished pair via QTimer::singleShot(0, ...).
   */
  ImmediateErrorReply(const QString& message, QObject* parent = nullptr)
    : AI::Reply(parent), m_message(message)
  {
    QTimer::singleShot(0, this, [this]() {
      Q_EMIT errorOccurred(m_message);
      Q_EMIT finished();
    });
  }

  /**
   * @brief No-op: the timer-driven completion cannot be cancelled.
   */
  void abort() override {}

private:
  QString m_message;
};

}  // namespace AI::detail

/**
 * @brief Anthropic enforces tool names ^[a-zA-Z0-9_-]+; encode dots/colons.
 */
static QString anthropicToolName(const QString& original)
{
  QString out = original;
  out.replace(QChar('.'), QChar('_'));
  out.replace(QChar(':'), QChar('_'));
  return out;
}

/**
 * @brief Rewrites a single content block in-place for Anthropic's tool format.
 */
static QJsonObject sanitizeBlock(QJsonObject block)
{
  const auto type = block.value(QStringLiteral("type")).toString();
  if (type == QStringLiteral("tool_use")) {
    const auto name               = block.value(QStringLiteral("name")).toString();
    block[QStringLiteral("name")] = anthropicToolName(name);
  } else if (type == QStringLiteral("tool_result")) {
    // Anthropic strict-rejects extra keys on tool_result blocks
    block.remove(QStringLiteral("_tool_name"));
    block.remove(QStringLiteral("_gemini_response"));
  }
  return block;
}

/**
 * @brief Sanitizes tool_use names and strips non-Anthropic fields from tool_result blocks.
 */
static QJsonArray sanitizeHistoryToolNames(const QJsonArray& history)
{
  QJsonArray out;
  for (const auto& v : history) {
    auto msg                = v.toObject();
    const auto contentValue = msg.value(QStringLiteral("content"));
    if (!contentValue.isArray()) {
      out.append(msg);
      continue;
    }

    QJsonArray newContent;
    for (const auto& bv : contentValue.toArray())
      newContent.append(sanitizeBlock(bv.toObject()));

    msg[QStringLiteral("content")] = newContent;
    out.append(msg);
  }
  return out;
}

//--------------------------------------------------------------------------------------------------
// AnthropicProvider
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores QNAM ref and key getter. No state touched at construction.
 */
AI::AnthropicProvider::AnthropicProvider(QNetworkAccessManager& nam, KeyGetter keyGetter)
  : m_nam(nam), m_keyGetter(std::move(keyGetter))
{}

/**
 * @brief Returns the human-readable provider name.
 */
QString AI::AnthropicProvider::displayName() const
{
  return QStringLiteral("Anthropic");
}

/**
 * @brief Returns the vendor "Get a key" deep link.
 */
QString AI::AnthropicProvider::keyVendorUrl() const
{
  return QStringLiteral("https://console.anthropic.com/settings/keys");
}

/**
 * @brief Returns the list of selectable Anthropic models, default first.
 */
QStringList AI::AnthropicProvider::availableModels() const
{
  return {
    QStringLiteral("claude-haiku-4-5"),
    QStringLiteral("claude-sonnet-4-6"),
    QStringLiteral("claude-opus-4-7"),
    QStringLiteral("claude-opus-4-8"),
  };
}

/**
 * @brief Returns the default Anthropic model for new sessions.
 */
QString AI::AnthropicProvider::defaultModel() const
{
  return QStringLiteral("claude-haiku-4-5");
}

/**
 * @brief Returns a human-friendly label for a known Anthropic model id.
 */
QString AI::AnthropicProvider::modelDisplayName(const QString& modelId) const
{
  if (modelId == QStringLiteral("claude-haiku-4-5"))
    return QStringLiteral("Haiku 4.5");

  if (modelId == QStringLiteral("claude-sonnet-4-6"))
    return QStringLiteral("Sonnet 4.6");

  if (modelId == QStringLiteral("claude-opus-4-7"))
    return QStringLiteral("Opus 4.7");

  if (modelId == QStringLiteral("claude-opus-4-8"))
    return QStringLiteral("Opus 4.8");

  return modelId;
}

/**
 * @brief Returns Anthropic-specific assistant shaping hints.
 */
AI::ProviderCapabilities AI::AnthropicProvider::capabilities() const
{
  ProviderCapabilities caps;
  caps.structuredSystemBlocks = true;
  caps.promptCaching          = true;
  caps.thinking               = !currentModel().contains(QStringLiteral("haiku"));
  caps.structuredToolResults  = true;
  caps.needsSmallToolSurface  = currentModel().contains(QStringLiteral("haiku"));
  caps.toolResultByteBudget   = caps.needsSmallToolSurface ? 4096 : 8192;
  return caps;
}

/**
 * @brief Builds the Messages API request body and returns a streaming Reply.
 */
AI::Reply* AI::AnthropicProvider::sendMessage(const QJsonArray& history,
                                              const QJsonArray& tools,
                                              bool forbidToolUse)
{
  const auto key = m_keyGetter ? m_keyGetter() : QString();
  if (key.isEmpty())
    return new AI::detail::ImmediateErrorReply(
      QObject::tr("No Anthropic API key set. Open Manage Keys to add one."));

  const auto model = currentModel();
  const auto caps  = capabilities();

  QJsonObject body;
  body[QStringLiteral("model")]      = model;
  body[QStringLiteral("max_tokens")] = 4096;
  body[QStringLiteral("stream")]     = true;
  body[QStringLiteral("system")]     = ContextBuilder::buildSystemArray();

  // Adaptive thinking + effort:medium (Sonnet 4.6 / Opus 4.6+); Haiku 4.5 rejects both
  if (caps.thinking) {
    QJsonObject thinking;
    thinking[QStringLiteral("type")]    = QStringLiteral("adaptive");
    thinking[QStringLiteral("display")] = QStringLiteral("summarized");
    body[QStringLiteral("thinking")]    = thinking;

    QJsonObject outputConfig;
    outputConfig[QStringLiteral("effort")] = QStringLiteral("medium");
    body[QStringLiteral("output_config")]  = outputConfig;
  }

  // Sanitize tool names against Anthropic's ^[a-zA-Z0-9_-]+ constraint
  if (!tools.isEmpty()) {
    QJsonArray sanitizedTools;
    for (const auto& v : tools) {
      auto t                    = v.toObject();
      const auto name           = t.value(QStringLiteral("name")).toString();
      t[QStringLiteral("name")] = anthropicToolName(name);
      sanitizedTools.append(t);
    }
    body[QStringLiteral("tools")] = sanitizedTools;

    // Forced-summary round: keep the schema (history holds tool_use) but forbid new calls
    if (forbidToolUse) {
      QJsonObject toolChoice;
      toolChoice[QStringLiteral("type")]  = QStringLiteral("none");
      body[QStringLiteral("tool_choice")] = toolChoice;
    }
  }
  body[QStringLiteral("messages")] = sanitizeHistoryToolNames(history);

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "Anthropic request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new AnthropicReply(m_nam, key, bytes);
}
