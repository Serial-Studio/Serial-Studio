/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/AnthropicReply.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "AI/KeyVault.h"
#include "AI/Logging.h"
#include "AI/SseEventReader.h"
#include "API/CommandRegistry.h"
#include "Misc/JsonValidator.h"

static constexpr int kInitialResponseTimeoutMs = 30 * 1000;
static const char* kEndpoint                   = "https://api.anthropic.com/v1/messages";

/**
 * @brief Resolves an Anthropic-sanitized tool name back to its dotted form.
 */
static QString resolveCanonicalToolName(const QString& sanitized)
{
  const auto& commands = API::CommandRegistry::instance().commands();

  if (commands.contains(sanitized))
    return sanitized;

  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    QString candidate = it.key();
    candidate.replace(QChar('.'), QChar('_'));
    candidate.replace(QChar(':'), QChar('_'));
    if (candidate == sanitized)
      return it.key();
  }

  if (sanitized.startsWith(QStringLiteral("meta_"))) {
    QString restored = sanitized;
    restored.replace(0, 5, QStringLiteral("meta."));
    return restored;
  }

  return sanitized;
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Issues the POST and wires SSE / network slots.
 */
AI::AnthropicReply::AnthropicReply(QNetworkAccessManager& nam,
                                   const QString& apiKey,
                                   const QByteArray& requestBody,
                                   QObject* parent)
  : Reply(parent)
  , m_nam(nam)
  , m_apiKey(apiKey)
  , m_requestBody(requestBody)
  , m_reply(nullptr)
  , m_sse(new SseEventReader(this))
  , m_finished(false)
{
  connect(m_sse, &SseEventReader::frameReceived, this, &AnthropicReply::onSseEvent);
  connect(m_sse, &SseEventReader::parseError, this, &AnthropicReply::onSseError);

  QNetworkRequest req((QUrl(QString::fromUtf8(kEndpoint))));
  req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  req.setRawHeader("x-api-key", apiKey.toUtf8());
  req.setRawHeader("anthropic-version", "2023-06-01");
  req.setRawHeader("accept", "text/event-stream");
  req.setTransferTimeout(kInitialResponseTimeoutMs);

  qCDebug(serialStudioAI) << "POST anthropic key=" << KeyVault::redact(apiKey)
                          << "body_bytes=" << requestBody.size();

  m_reply = m_nam.post(req, requestBody);
  m_reply->setParent(this);

  connect(m_reply, &QNetworkReply::readyRead, this, &AnthropicReply::onReplyReadyRead);
  connect(m_reply, &QNetworkReply::finished, this, &AnthropicReply::onReplyFinished);
  connect(m_reply, &QNetworkReply::errorOccurred, this, &AnthropicReply::onReplyError);
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cancels the in-flight network reply if any.
 */
void AI::AnthropicReply::abort()
{
  if (m_reply && m_reply->isRunning())
    m_reply->abort();
}

/**
 * @brief Returns the Anthropic stop_reason from message_delta if seen.
 */
QString AI::AnthropicReply::stopReason() const noexcept
{
  return m_stopReason;
}

//--------------------------------------------------------------------------------------------------
// SSE event handler
//--------------------------------------------------------------------------------------------------

/**
 * @brief Routes a single SSE event to the right handler branch.
 */
void AI::AnthropicReply::onSseEvent(const QString& name, const QJsonObject& data)
{
  if (m_finished)
    return;

  if (name == QStringLiteral("message_start")) {
    handleMessageStart(data);
    return;
  }

  if (name == QStringLiteral("content_block_start")) {
    handleContentBlockStart(data);
    return;
  }

  if (name == QStringLiteral("content_block_delta")) {
    handleContentBlockDelta(data);
    return;
  }

  if (name == QStringLiteral("content_block_stop")) {
    handleContentBlockStop(data);
    return;
  }

  if (name == QStringLiteral("message_delta")) {
    const auto delta = data.value(QStringLiteral("delta")).toObject();
    const auto sr    = delta.value(QStringLiteral("stop_reason")).toString();
    if (!sr.isEmpty())
      m_stopReason = sr;

    return;
  }

  if (name == QStringLiteral("message_stop")) {
    finishOk();
    return;
  }

  if (name == QStringLiteral("error")) {
    const auto err = data.value(QStringLiteral("error")).toObject();
    const auto msg = err.value(QStringLiteral("message")).toString();
    finishWithError(msg.isEmpty() ? tr("Anthropic error") : msg);
  }
}

/**
 * @brief Pulls cache-token stats from the message_start envelope.
 */
void AI::AnthropicReply::handleMessageStart(const QJsonObject& data)
{
  const auto message = data.value(QStringLiteral("message")).toObject();
  const auto usage   = message.value(QStringLiteral("usage")).toObject();
  const int read     = usage.value(QStringLiteral("cache_read_input_tokens")).toInt();
  const int created  = usage.value(QStringLiteral("cache_creation_input_tokens")).toInt();
  if (read > 0 || created > 0)
    Q_EMIT cacheStatsAvailable(read, created);
}

/**
 * @brief Records the type and (for tool_use) id/name of a new content block.
 */
void AI::AnthropicReply::handleContentBlockStart(const QJsonObject& data)
{
  const int idx    = data.value(QStringLiteral("index")).toInt(-1);
  const auto block = data.value(QStringLiteral("content_block")).toObject();
  BlockState bs;
  bs.type = block.value(QStringLiteral("type")).toString();
  if (bs.type == QStringLiteral("tool_use")) {
    bs.toolUseId   = block.value(QStringLiteral("id")).toString();
    bs.toolUseName = block.value(QStringLiteral("name")).toString();
  }
  if (idx >= 0)
    m_blocks.insert(idx, bs);
}

/**
 * @brief Forwards text/thinking deltas and accumulates partial tool-input JSON.
 */
void AI::AnthropicReply::handleContentBlockDelta(const QJsonObject& data)
{
  const int idx        = data.value(QStringLiteral("index")).toInt(-1);
  const auto delta     = data.value(QStringLiteral("delta")).toObject();
  const auto deltaType = delta.value(QStringLiteral("type")).toString();

  if (deltaType == QStringLiteral("text_delta")) {
    const auto chunk = delta.value(QStringLiteral("text")).toString();
    if (!chunk.isEmpty())
      Q_EMIT partialText(chunk);

    return;
  }

  if (deltaType == QStringLiteral("thinking_delta")) {
    const auto chunk = delta.value(QStringLiteral("thinking")).toString();
    if (!chunk.isEmpty())
      Q_EMIT partialThinking(chunk);

    return;
  }

  if (deltaType == QStringLiteral("input_json_delta")) {
    auto it = m_blocks.find(idx);
    if (it != m_blocks.end())
      it->toolInputJson.append(delta.value(QStringLiteral("partial_json")).toString().toUtf8());
  }
}

/**
 * @brief On block close, validates accumulated tool-use JSON and emits the tool call.
 */
void AI::AnthropicReply::handleContentBlockStop(const QJsonObject& data)
{
  const int idx = data.value(QStringLiteral("index")).toInt(-1);
  auto it       = m_blocks.find(idx);
  if (it == m_blocks.end())
    return;

  if (it->type == QStringLiteral("tool_use"))
    emitToolUseFromBlock(*it);

  m_blocks.remove(idx);
}

/**
 * @brief Validates the buffered tool input JSON and fires toolCallRequested.
 */
void AI::AnthropicReply::emitToolUseFromBlock(const BlockState& bs)
{
  Misc::JsonValidator::Limits limits;
  limits.maxFileSize  = 1 * 1024 * 1024;
  limits.maxDepth     = 32;
  limits.maxArraySize = 1000;

  QByteArray payload = bs.toolInputJson;
  if (payload.isEmpty())
    payload = "{}";

  const auto result = Misc::JsonValidator::parseAndValidate(payload, limits);
  if (!result.valid || !result.document.isObject()) {
    qCWarning(serialStudioAI) << "Tool-use input JSON invalid for" << bs.toolUseName << ":"
                              << result.errorMessage;
    return;
  }

  Q_EMIT toolCallRequested(
    bs.toolUseId, resolveCanonicalToolName(bs.toolUseName), result.document.object());
}

/**
 * @brief Logs and ends the stream when the SSE parser rejects a frame.
 */
void AI::AnthropicReply::onSseError(const QString& reason)
{
  qCWarning(serialStudioAI) << "SSE parse error:" << reason;
  finishWithError(tr("Stream parse error: %1").arg(reason));
}

//--------------------------------------------------------------------------------------------------
// QNetworkReply slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forwards every chunk from the network reply into the SSE reader.
 */
void AI::AnthropicReply::onReplyReadyRead()
{
  if (!m_reply || m_finished)
    return;

  const auto status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (status >= 400)
    return;

  m_sse->feed(m_reply->readAll());
}

/**
 * @brief Handles end-of-stream cleanup and surfaces 4xx/5xx error bodies.
 */
void AI::AnthropicReply::onReplyFinished()
{
  if (m_finished)
    return;

  const auto status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (m_reply->error() != QNetworkReply::NoError && status < 400) {
    finishWithError(m_reply->errorString());
    return;
  }

  if (status >= 400) {
    const auto body = m_reply->readAll();
    QString msg;
    Misc::JsonValidator::Limits limits;
    limits.maxFileSize = 256 * 1024;
    const auto parsed  = Misc::JsonValidator::parseAndValidate(body, limits);
    if (parsed.valid && parsed.document.isObject()) {
      const auto err = parsed.document.object().value(QStringLiteral("error")).toObject();
      msg            = err.value(QStringLiteral("message")).toString();
    }
    if (msg.isEmpty())
      msg = tr("HTTP %1").arg(status);

    if (status == 401)
      finishWithError(tr("Invalid API key (%1)").arg(msg));
    else if (status == 429)
      finishWithError(tr("Rate limited: %1").arg(msg));
    else
      finishWithError(tr("Anthropic %1: %2").arg(QString::number(status), msg));

    return;
  }

  m_sse->feed({});
  finishOk();
}

/**
 * @brief Logs transport-level errors; surfacing happens in onReplyFinished.
 */
void AI::AnthropicReply::onReplyError()
{
  if (!m_reply || m_finished)
    return;

  qCWarning(serialStudioAI) << "QNetworkReply error:" << m_reply->errorString();
}

//--------------------------------------------------------------------------------------------------
// Finalization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the stream finished, emits @ref finished.
 */
void AI::AnthropicReply::finishOk()
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT finished();
}

/**
 * @brief Marks the stream finished with an error message.
 */
void AI::AnthropicReply::finishWithError(const QString& message)
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT errorOccurred(message);
  Q_EMIT finished();
}
