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
#include "AI/Providers/ProviderJson.h"
#include "AI/SseEventReader.h"
#include "Misc/JsonValidator.h"

static constexpr int kInitialResponseTimeoutMs = 120 * 1000;
static const char* kEndpoint                   = "https://api.anthropic.com/v1/messages";

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
{
  connect(m_sse, &SseEventReader::frameReceived, this, &AnthropicReply::onSseEvent);
  connect(m_sse, &SseEventReader::parseError, this, &AnthropicReply::onSseError);

  QNetworkRequest req((QUrl(QString::fromUtf8(kEndpoint))));
  req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  req.setRawHeader("x-api-key", apiKey.toUtf8());
  req.setRawHeader("anthropic-version", "2023-06-01");
  req.setRawHeader("accept", "text/event-stream");
  req.setTransferTimeout(kInitialResponseTimeoutMs);
  applyStreamPolicy(req);

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

//--------------------------------------------------------------------------------------------------
// SSE event handler
//--------------------------------------------------------------------------------------------------

/**
 * @brief Routes a single SSE event to the right handler branch.
 */
void AI::AnthropicReply::onSseEvent(const QString& name, const QJsonObject& data)
{
  if (isFinished())
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

  if (name == QStringLiteral("message_delta"))
    return;

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

  if (bs.type == QStringLiteral("redacted_thinking"))
    bs.redactedData = block.value(QStringLiteral("data")).toString();

  if (streamBudgetBreached(bs.redactedData.size()))
    return;

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
    if (!chunk.isEmpty() && !streamBudgetBreached(chunk.size()))
      Q_EMIT partialText(chunk);

    return;
  }

  if (deltaType == QStringLiteral("thinking_delta")) {
    const auto chunk = delta.value(QStringLiteral("thinking")).toString();
    if (!chunk.isEmpty() && !streamBudgetBreached(chunk.size())) {
      auto it = m_blocks.find(idx);
      if (it != m_blocks.end())
        it->thinkingText.append(chunk);

      Q_EMIT partialThinking(chunk);
    }

    return;
  }

  if (deltaType == QStringLiteral("signature_delta")) {
    const auto sig = delta.value(QStringLiteral("signature")).toString();
    auto it        = m_blocks.find(idx);
    if (it != m_blocks.end() && !streamBudgetBreached(sig.size()))
      it->thinkingSignature.append(sig);

    return;
  }

  if (deltaType == QStringLiteral("input_json_delta")) {
    const auto json = delta.value(QStringLiteral("partial_json")).toString().toUtf8();
    auto it         = m_blocks.find(idx);
    if (it != m_blocks.end() && !streamBudgetBreached(json.size()))
      it->toolInputJson.append(json);
  }
}

/**
 * @brief On block close, emits completed tool-use / thinking blocks. Thinking blocks keep
 *        their signature so the conversation can echo them back verbatim on the next turn.
 */
void AI::AnthropicReply::handleContentBlockStop(const QJsonObject& data)
{
  const int idx = data.value(QStringLiteral("index")).toInt(-1);
  auto it       = m_blocks.find(idx);
  if (it == m_blocks.end())
    return;

  if (it->type == QStringLiteral("tool_use"))
    emitToolUseFromBlock(*it);

  if (it->type == QStringLiteral("thinking") && !it->thinkingSignature.isEmpty()) {
    QJsonObject block;
    block[QStringLiteral("type")]      = QStringLiteral("thinking");
    block[QStringLiteral("thinking")]  = it->thinkingText;
    block[QStringLiteral("signature")] = it->thinkingSignature;
    Q_EMIT thinkingBlockFinished(block);
  }

  if (it->type == QStringLiteral("redacted_thinking") && !it->redactedData.isEmpty()) {
    QJsonObject block;
    block[QStringLiteral("type")] = QStringLiteral("redacted_thinking");
    block[QStringLiteral("data")] = it->redactedData;
    Q_EMIT thinkingBlockFinished(block);
  }

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

  Q_EMIT toolCallRequested(bs.toolUseId, bs.toolUseName, result.document.object());
}

/**
 * @brief Skips malformed frames but ends the turn on unrecoverable stream-state loss, the same
 *        rule the other backends follow: one bad frame is not worth the whole turn, and a buffer
 *        reset must not ship a silently truncated reply as success.
 */
void AI::AnthropicReply::onSseError(const QString& reason)
{
  qCWarning(serialStudioAI) << "SSE parse error:" << reason;
  if (!endsTurnOnParseError(reason))
    return;

  finishWithError(tr("Stream parse error: %1").arg(reason));
  abort();
}

//--------------------------------------------------------------------------------------------------
// QNetworkReply slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forwards every chunk from the network reply into the SSE reader.
 */
void AI::AnthropicReply::onReplyReadyRead()
{
  if (!m_reply || isFinished())
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
  if (isFinished())
    return;

  const auto status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (m_reply->error() != QNetworkReply::NoError && status < 400) {
    setTransientError(true);
    finishWithError(m_reply->errorString());
    return;
  }

  if (status >= 400) {
    setTransientError(ProviderJson::isTransientHttpStatus(status));
    QString msg = ProviderJson::errorMessageFromBody(m_reply->readAll());
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
  if (!m_reply || isFinished())
    return;

  qCWarning(serialStudioAI) << "QNetworkReply error:" << m_reply->errorString();
}
