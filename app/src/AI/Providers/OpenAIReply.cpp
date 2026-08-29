/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/OpenAIReply.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUuid>

#include "AI/KeyVault.h"
#include "AI/Logging.h"
#include "AI/Providers/ProviderJson.h"
#include "AI/SseEventReader.h"
#include "Misc/JsonValidator.h"

static constexpr int kOpenAIInitialResponseTimeoutMs = 120 * 1000;
static const char* const kOpenAIEndpoint             = "https://api.openai.com/v1/chat/completions";
static const char* const kOpenAIAuthHeader           = "Authorization";

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Convenience constructor for the canonical OpenAI Chat Completions endpoint.
 */
AI::OpenAIReply::OpenAIReply(QNetworkAccessManager& nam,
                             const QString& apiKey,
                             const QByteArray& requestBody,
                             QObject* parent)
  : OpenAIReply(nam,
                QString::fromUtf8(kOpenAIEndpoint),
                QString::fromUtf8(kOpenAIAuthHeader),
                apiKey,
                requestBody,
                QStringLiteral("OpenAI"),
                -1,
                false,
                parent)
{}

/**
 * @brief Generic constructor for any OpenAI-compatible endpoint (DeepSeek, Local, etc.).
 *        transferTimeoutMs <= 0 selects the default; parseThinkTags routes inline
 *        <think> blocks (llama.cpp/Ollama style) into the thinking channel.
 */
AI::OpenAIReply::OpenAIReply(QNetworkAccessManager& nam,
                             const QString& endpointUrl,
                             const QString& authHeader,
                             const QString& apiKey,
                             const QByteArray& requestBody,
                             const QString& providerLabel,
                             int transferTimeoutMs,
                             bool parseThinkTags,
                             QObject* parent)
  : Reply(parent)
  , m_nam(nam)
  , m_endpointUrl(endpointUrl)
  , m_authHeader(authHeader)
  , m_apiKey(apiKey)
  , m_providerLabel(providerLabel)
  , m_requestBody(requestBody)
  , m_reply(nullptr)
  , m_sse(new SseEventReader(this))
  , m_transferTimeoutMs(transferTimeoutMs > 0 ? transferTimeoutMs : kOpenAIInitialResponseTimeoutMs)
  , m_parseThinkTags(parseThinkTags)
  , m_thinkSplitter()
  , m_finished(false)
{
  connect(m_sse, &SseEventReader::frameReceived, this, &OpenAIReply::onSseEvent);
  connect(m_sse, &SseEventReader::parseError, this, &OpenAIReply::onSseError);

  issueRequest();
}

/**
 * @brief Builds the request, sets headers, and connects QNetworkReply slots.
 */
void AI::OpenAIReply::issueRequest()
{
  QNetworkRequest req((QUrl(m_endpointUrl)));
  req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  if (!m_authHeader.isEmpty() && !m_apiKey.isEmpty())
    req.setRawHeader(m_authHeader.toUtf8(), QStringLiteral("Bearer %1").arg(m_apiKey).toUtf8());

  req.setRawHeader("accept", "text/event-stream");
  req.setTransferTimeout(m_transferTimeoutMs);

  qCDebug(serialStudioAI) << "POST" << m_providerLabel << m_endpointUrl
                          << "key=" << KeyVault::redact(m_apiKey)
                          << "body_bytes=" << m_requestBody.size();

  m_reply = m_nam.post(req, m_requestBody);
  m_reply->setParent(this);

  connect(m_reply, &QNetworkReply::readyRead, this, &OpenAIReply::onReplyReadyRead);
  connect(m_reply, &QNetworkReply::finished, this, &OpenAIReply::onReplyFinished);
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cancels the in-flight network reply if any.
 */
void AI::OpenAIReply::abort()
{
  if (m_reply && m_reply->isRunning())
    m_reply->abort();
}

//--------------------------------------------------------------------------------------------------
// SSE event handler
//--------------------------------------------------------------------------------------------------

/**
 * @brief Routes a single SSE event into the OpenAI delta processor.
 */
void AI::OpenAIReply::onSseEvent(const QString& name, const QJsonObject& data)
{
  Q_UNUSED(name);

  if (m_finished)
    return;

  const auto choices = data.value(QStringLiteral("choices")).toArray();
  if (choices.isEmpty())
    return;

  const auto choice = choices.first().toObject();
  processChoiceDelta(choice);

  const auto reason = choice.value(QStringLiteral("finish_reason"));
  if (reason.isString())
    m_finishReason = reason.toString();
}

/**
 * @brief Skips malformed frames but ends the turn on unrecoverable stream-state loss, so a
 *        buffer reset cannot ship a silently truncated reply as success.
 */
void AI::OpenAIReply::onSseError(const QString& reason)
{
  qCWarning(serialStudioAI) << m_providerLabel << "SSE parse error:" << reason;
  if (!SseEventReader::fatalReason(reason))
    return;

  finishWithError(tr("Stream parse error: %1").arg(reason));
  abort();
}

//--------------------------------------------------------------------------------------------------
// Choice / tool call deltas
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes a single choice delta: text, reasoning, and tool-call fragments.
 *        reasoning_content (DeepSeek R1) and reasoning (Ollama-style servers) carry the
 *        model's thinking stream; dropping them would hide reasoning from the user.
 */
void AI::OpenAIReply::processChoiceDelta(const QJsonObject& choice)
{
  const auto delta = choice.value(QStringLiteral("delta")).toObject();

  auto reasoningValue = delta.value(QStringLiteral("reasoning_content"));
  if (!reasoningValue.isString())
    reasoningValue = delta.value(QStringLiteral("reasoning"));

  if (reasoningValue.isString()) {
    const auto chunk = reasoningValue.toString();
    if (!chunk.isEmpty() && !streamBudgetBreached(chunk.size()))
      Q_EMIT partialThinking(chunk);
  }

  if (m_finished)
    return;

  const auto contentValue = delta.value(QStringLiteral("content"));
  if (contentValue.isString()) {
    const auto chunk = contentValue.toString();
    if (!chunk.isEmpty() && !streamBudgetBreached(chunk.size()))
      routeContentChunk(chunk);
  }

  if (m_finished)
    return;

  const auto toolCallsValue = delta.value(QStringLiteral("tool_calls"));
  if (!toolCallsValue.isArray())
    return;

  for (const auto& v : toolCallsValue.toArray()) {
    const auto tc = v.toObject();
    const int idx = tc.value(QStringLiteral("index")).toInt(-1);
    if (idx < 0)
      continue;

    auto& state = m_toolCalls[idx];

    const auto idValue = tc.value(QStringLiteral("id"));
    if (idValue.isString() && !idValue.toString().isEmpty())
      state.id = idValue.toString();

    const auto fn        = tc.value(QStringLiteral("function")).toObject();
    const auto nameValue = fn.value(QStringLiteral("name"));
    if (nameValue.isString() && !nameValue.toString().isEmpty())
      state.name = nameValue.toString();

    const auto argsValue = fn.value(QStringLiteral("arguments"));
    if (argsValue.isString()) {
      const auto args = argsValue.toString().toUtf8();
      if (streamBudgetBreached(args.size()))
        return;

      state.arguments.append(args);
    }
  }
}

/**
 * @brief Forwards a content chunk as text, or through the <think> scanner when local
 *        servers stream reasoning inline instead of in a reasoning delta field.
 */
void AI::OpenAIReply::routeContentChunk(const QString& chunk)
{
  if (!m_parseThinkTags) {
    Q_EMIT partialText(chunk);
    return;
  }

  publishThinkChunks(m_thinkSplitter.append(chunk));
}

/**
 * @brief Emits each scanned run on the channel the splitter assigned it, in stream order.
 */
void AI::OpenAIReply::publishThinkChunks(const std::vector<ThinkChunk>& chunks)
{
  for (const auto& chunk : chunks)
    if (chunk.channel == ThinkChannel::Thinking)
      Q_EMIT partialThinking(chunk.text);

    else
      Q_EMIT partialText(chunk.text);
}

/**
 * @brief Emits accumulated tool calls once the choice closes with stop_reason.
 */
void AI::OpenAIReply::emitPendingToolCalls()
{
  if (m_toolCalls.isEmpty())
    return;

  Misc::JsonValidator::Limits limits;
  limits.maxFileSize  = 1 * 1024 * 1024;
  limits.maxDepth     = 32;
  limits.maxArraySize = 1000;

  QList<int> indices = m_toolCalls.keys();
  std::sort(indices.begin(), indices.end());

  for (int idx : indices) {
    auto& state = m_toolCalls[idx];
    if (state.emitted)
      continue;

    if (state.id.isEmpty())
      state.id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QByteArray payload = state.arguments;
    if (payload.isEmpty())
      payload = "{}";

    const auto result = Misc::JsonValidator::parseAndValidate(payload, limits);
    if (!result.valid || !result.document.isObject()) {
      qCWarning(serialStudioAI) << m_providerLabel << "tool args invalid for" << state.name << ":"
                                << result.errorMessage;
      state.emitted = true;
      continue;
    }

    Q_EMIT toolCallRequested(state.id, state.name, result.document.object());
    state.emitted = true;
  }
}

//--------------------------------------------------------------------------------------------------
// QNetworkReply slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forwards every chunk into the SSE reader; [DONE] frames are dropped there so the
 *        sentinel can never corrupt a frame it merely appears inside of.
 */
void AI::OpenAIReply::onReplyReadyRead()
{
  if (!m_reply || m_finished)
    return;

  const auto status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (status >= 400)
    return;

  m_sse->feed(m_reply->readAll());
}

/**
 * @brief Handles end-of-stream: flushes any pending tool calls and finalizes.
 */
void AI::OpenAIReply::onReplyFinished()
{
  if (m_finished)
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
      finishWithError(tr("%1 %2: %3").arg(m_providerLabel, QString::number(status), msg));

    return;
  }

  m_sse->feed({});
  if (m_parseThinkTags)
    publishThinkChunks(m_thinkSplitter.flush());

  emitPendingToolCalls();

  if (m_finishReason == QStringLiteral("tool_calls") && m_toolCalls.isEmpty()) {
    qCWarning(serialStudioAI) << m_providerLabel
                              << "finish_reason=tool_calls but no tool_calls deltas were received "
                                 "-- treat as malformed stream from the provider";
  }

  finishOk();
}

//--------------------------------------------------------------------------------------------------
// Finalization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Charges bytes against the shared per-reply budget; on breach, ends the turn with a
 *        visible error and aborts the transport so Qt stops buffering the runaway stream.
 */
bool AI::OpenAIReply::streamBudgetBreached(qsizetype bytes)
{
  if (!chargeStreamBudget(bytes))
    return false;

  finishWithError(
    tr("Reply exceeded the %1 MB stream limit").arg(kMaxStreamedReplyBytes / (1024 * 1024)));
  abort();
  return true;
}

/**
 * @brief Marks the stream finished, emits @ref finished.
 */
void AI::OpenAIReply::finishOk()
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT finished();
}

/**
 * @brief Marks the stream finished with an error message.
 */
void AI::OpenAIReply::finishWithError(const QString& message)
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT errorOccurred(message);
  Q_EMIT finished();
}
