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
#include <QSet>
#include <QTimer>

#include "AI/ContextBuilder.h"
#include "AI/Logging.h"
#include "AI/Providers/OpenAIReply.h"

namespace detail {

/**
 * @brief Reply that fires errorOccurred on the next event-loop tick.
 */
class ImmediateErrorReply : public AI::Reply {
public:
  /**
   * @brief Schedules errorOccurred + finished on the next event-loop tick.
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
   * @brief No-op since the error fires immediately.
   */
  void abort() override {}

private:
  QString m_message;
};

}  // namespace detail

/**
 * @brief OpenAI enforces tool names ^[a-zA-Z0-9_-]+; encode dots/colons.
 */
static QString sanitizeName(const QString& original)
{
  QString out = original;
  out.replace(QChar('.'), QChar('_'));
  out.replace(QChar(':'), QChar('_'));
  return out;
}

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
// Translators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Inserts stub tool replies for any assistant tool_call that lacks a tool message.
 */
static QJsonArray backfillDanglingToolCalls(const QJsonArray& messages)
{
  QSet<QString> answered;
  for (const auto& value : messages) {
    const auto msg = value.toObject();
    if (msg.value(QStringLiteral("role")).toString() == QStringLiteral("tool"))
      answered.insert(msg.value(QStringLiteral("tool_call_id")).toString());
  }

  QJsonArray out;
  for (const auto& value : messages) {
    const auto msg = value.toObject();
    out.append(msg);

    const auto calls = msg.value(QStringLiteral("tool_calls")).toArray();
    for (const auto& callValue : calls) {
      const auto id = callValue.toObject().value(QStringLiteral("id")).toString();
      if (id.isEmpty() || answered.contains(id))
        continue;

      QJsonObject stub;
      stub[QStringLiteral("role")]         = QStringLiteral("tool");
      stub[QStringLiteral("tool_call_id")] = id;
      stub[QStringLiteral("content")] = QStringLiteral("{\"ok\":false,\"error\":\"no_result\"}");
      out.append(stub);
      answered.insert(id);
    }
  }

  return out;
}

/**
 * @brief Converts Anthropic-shaped history into the OpenAI Chat Completions shape.
 */
QJsonArray AI::OpenAIProvider::translateHistory(const QJsonArray& history,
                                                const QString& systemText,
                                                bool useDeveloperRole)
{
  QJsonArray out;

  if (!systemText.isEmpty()) {
    QJsonObject sys;
    sys[QStringLiteral("role")] =
      useDeveloperRole ? QStringLiteral("developer") : QStringLiteral("system");
    sys[QStringLiteral("content")] = systemText;
    out.append(sys);
  }

  for (const auto& v : history) {
    const auto msg          = v.toObject();
    const auto role         = msg.value(QStringLiteral("role")).toString();
    const auto contentValue = msg.value(QStringLiteral("content"));

    if (contentValue.isString()) {
      QJsonObject m;
      m[QStringLiteral("role")]    = role;
      m[QStringLiteral("content")] = contentValue.toString();
      out.append(m);
      continue;
    }

    if (!contentValue.isArray())
      continue;

    QString textAccumulator;
    QJsonArray toolCalls;
    QJsonArray toolResultMessages;
    translateBlocks(contentValue.toArray(), textAccumulator, toolCalls, toolResultMessages);

    if (!textAccumulator.isEmpty() || !toolCalls.isEmpty()) {
      QJsonObject m;
      m[QStringLiteral("role")] = role;
      if (!textAccumulator.isEmpty())
        m[QStringLiteral("content")] = textAccumulator;

      if (!toolCalls.isEmpty())
        m[QStringLiteral("tool_calls")] = toolCalls;

      out.append(m);
    }

    for (const auto& tr : toolResultMessages)
      out.append(tr);
  }

  return backfillDanglingToolCalls(out);
}

/**
 * @brief Splits Anthropic content blocks into OpenAI text / tool_calls / tool messages.
 */
void AI::OpenAIProvider::translateBlocks(const QJsonArray& blocks,
                                         QString& textAccumulator,
                                         QJsonArray& toolCalls,
                                         QJsonArray& toolResultMessages)
{
  for (const auto& bv : blocks) {
    const auto block = bv.toObject();
    const auto type  = block.value(QStringLiteral("type")).toString();

    if (type == QStringLiteral("text")) {
      const auto t = block.value(QStringLiteral("text")).toString();
      if (t.isEmpty())
        continue;

      if (!textAccumulator.isEmpty())
        textAccumulator.append(QChar('\n'));

      textAccumulator.append(t);
      continue;
    }

    if (type == QStringLiteral("tool_use")) {
      QJsonObject fn;
      fn[QStringLiteral("name")] = sanitizeName(block.value(QStringLiteral("name")).toString());
      fn[QStringLiteral("arguments")] =
        QString::fromUtf8(QJsonDocument(block.value(QStringLiteral("input")).toObject())
                            .toJson(QJsonDocument::Compact));

      QJsonObject tc;
      tc[QStringLiteral("id")]       = block.value(QStringLiteral("id")).toString();
      tc[QStringLiteral("type")]     = QStringLiteral("function");
      tc[QStringLiteral("function")] = fn;
      toolCalls.append(tc);
      continue;
    }

    if (type != QStringLiteral("tool_result"))
      continue;

    QJsonObject toolMsg;
    toolMsg[QStringLiteral("role")]         = QStringLiteral("tool");
    toolMsg[QStringLiteral("tool_call_id")] = block.value(QStringLiteral("tool_use_id")).toString();
    toolMsg[QStringLiteral("content")]      = block.value(QStringLiteral("content")).toString();
    toolResultMessages.append(toolMsg);
  }
}

/**
 * @brief Converts AI-tool definitions into the OpenAI tool-choice schema.
 */
QJsonArray AI::OpenAIProvider::translateTools(const QJsonArray& tools)
{
  QJsonArray out;
  for (const auto& v : tools) {
    const auto t = v.toObject();
    QJsonObject fn;
    fn[QStringLiteral("name")]        = sanitizeName(t.value(QStringLiteral("name")).toString());
    fn[QStringLiteral("description")] = t.value(QStringLiteral("description"));
    fn[QStringLiteral("parameters")]  = t.value(QStringLiteral("input_schema"));

    QJsonObject tool;
    tool[QStringLiteral("type")]     = QStringLiteral("function");
    tool[QStringLiteral("function")] = fn;
    out.append(tool);
  }
  return out;
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
    return new detail::ImmediateErrorReply(
      QObject::tr("No OpenAI API key set. Open Manage Keys to add one."));

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

  const auto model = currentModel();
  const auto caps  = capabilities();

  QJsonObject body;
  body[QStringLiteral("model")]                  = model;
  body[QStringLiteral("stream")]                 = true;
  body[QStringLiteral("store")]                  = false;
  body[QStringLiteral("parallel_tool_calls")]    = caps.parallelToolCalls;
  body[QStringLiteral("prompt_cache_key")]       = QStringLiteral("serial-studio-ai-assistant");
  body[QStringLiteral("prompt_cache_retention")] = QStringLiteral("24h");
  body[QStringLiteral("messages")] = translateHistory(history, systemText, caps.developerRole);

  if (isReasoningModel(model))
    body[QStringLiteral("reasoning_effort")] = QStringLiteral("none");

  if (!tools.isEmpty()) {
    body[QStringLiteral("tools")] = translateTools(tools);
    body[QStringLiteral("tool_choice")] =
      forbidToolUse ? QStringLiteral("none") : QStringLiteral("auto");
  }

  const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

  qCDebug(serialStudioAI) << "OpenAI request: tools=" << tools.size()
                          << "history=" << history.size() << "bytes=" << bytes.size();

  return new OpenAIReply(m_nam, key, bytes);
}
