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
#include "AI/SseEventReader.h"
#include "AI/ToolDispatcher.h"
#include "API/CommandRegistry.h"
#include "Misc/JsonValidator.h"

static constexpr int kInitialResponseTimeoutMs = 30 * 1000;
static const char* const kOpenAIEndpoint       = "https://api.openai.com/v1/chat/completions";
static const char* const kOpenAIAuthHeader     = "Authorization";

/**
 * @brief Inverts provider name sanitization across the full advertised tool surface.
 */
static QString resolveCanonicalToolName(const QString& sanitized)
{
  static const auto buildReverseMap = []() {
    QHash<QString, QString> map;
    AI::ToolDispatcher dispatcher;
    const auto tools = dispatcher.availableTools();
    for (const auto& value : tools) {
      const auto canonical = value.toObject().value(QStringLiteral("name")).toString();
      QString key          = canonical;
      key.replace(QChar('.'), QChar('_'));
      key.replace(QChar(':'), QChar('_'));
      map.insert(key, canonical);
    }
    return map;
  };
  static const QHash<QString, QString> kReverse = buildReverseMap();

  const auto it = kReverse.constFind(sanitized);
  if (it != kReverse.constEnd())
    return it.value();

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
                parent)
{}

/**
 * @brief Generic constructor for any OpenAI-compatible endpoint (DeepSeek, Local, etc.).
 */
AI::OpenAIReply::OpenAIReply(QNetworkAccessManager& nam,
                             const QString& endpointUrl,
                             const QString& authHeader,
                             const QString& apiKey,
                             const QByteArray& requestBody,
                             const QString& providerLabel,
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
  req.setTransferTimeout(kInitialResponseTimeoutMs);

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
 * @brief Logs but does not abort on transient SSE parse errors.
 */
void AI::OpenAIReply::onSseError(const QString& reason)
{
  qCWarning(serialStudioAI) << m_providerLabel << "SSE parse error:" << reason;
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
    if (!chunk.isEmpty())
      Q_EMIT partialThinking(chunk);
  }

  const auto contentValue = delta.value(QStringLiteral("content"));
  if (contentValue.isString()) {
    const auto chunk = contentValue.toString();
    if (!chunk.isEmpty())
      Q_EMIT partialText(chunk);
  }

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
    if (argsValue.isString())
      state.arguments.append(argsValue.toString().toUtf8());
  }
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

    Q_EMIT toolCallRequested(
      state.id, resolveCanonicalToolName(state.name), result.document.object());
    state.emitted = true;
  }
}

//--------------------------------------------------------------------------------------------------
// QNetworkReply slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forwards every chunk into the SSE reader, stripping [DONE] sentinel.
 */
void AI::OpenAIReply::onReplyReadyRead()
{
  if (!m_reply || m_finished)
    return;

  const auto status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (status >= 400)
    return;

  auto chunk = m_reply->readAll();

  const QByteArray sentinel = "data: [DONE]";
  const int idx             = chunk.indexOf(sentinel);
  if (idx >= 0) {
    int end = chunk.indexOf("\n\n", idx);
    if (end < 0)
      chunk.truncate(idx);
    else
      chunk.remove(idx, (end + 2) - idx);
  }

  m_sse->feed(chunk);
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
    setTransientError(status == 408 || status == 429 || status >= 500);
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
      finishWithError(tr("%1 %2: %3").arg(m_providerLabel, QString::number(status), msg));

    return;
  }

  m_sse->feed({});
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
