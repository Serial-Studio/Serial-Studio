/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/GeminiReply.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QUrl>
#include <QUuid>

#include "AI/KeyVault.h"
#include "AI/Logging.h"
#include "AI/SseEventReader.h"
#include "Misc/JsonValidator.h"

static constexpr int kInitialResponseTimeoutMs = 30 * 1000;

/**
 * @brief Returns a redacted form of a Gemini URL with the key query stripped.
 */
static QString redactUrl(const QUrl& url)
{
  auto u = url;
  if (u.hasQuery()) {
    auto query       = u.query();
    const int keyIdx = query.indexOf(QStringLiteral("key="));
    if (keyIdx >= 0) {
      const int end       = query.indexOf('&', keyIdx);
      const auto replaced = QStringLiteral("key=***");
      if (end < 0)
        query.replace(keyIdx, query.size() - keyIdx, replaced);
      else
        query.replace(keyIdx, end - keyIdx, replaced);
    }
    u.setQuery(query);
  }
  return u.toString();
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Issues the POST and wires SSE / network slots.
 */
AI::GeminiReply::GeminiReply(QNetworkAccessManager& nam,
                             const QUrl& endpoint,
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
  connect(m_sse, &SseEventReader::frameReceived, this, &GeminiReply::onSseEvent);
  connect(m_sse, &SseEventReader::parseError, this, &GeminiReply::onSseError);

  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  req.setRawHeader("accept", "text/event-stream");
  req.setTransferTimeout(kInitialResponseTimeoutMs);

  qCDebug(serialStudioAI) << "POST gemini url=" << redactUrl(endpoint)
                          << "key=" << KeyVault::redact(apiKey)
                          << "body_bytes=" << requestBody.size();

  m_reply = m_nam.post(req, requestBody);
  m_reply->setParent(this);

  connect(m_reply, &QNetworkReply::readyRead, this, &GeminiReply::onReplyReadyRead);
  connect(m_reply, &QNetworkReply::finished, this, &GeminiReply::onReplyFinished);
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cancels the in-flight network reply if any.
 */
void AI::GeminiReply::abort()
{
  if (m_reply && m_reply->isRunning())
    m_reply->abort();
}

//--------------------------------------------------------------------------------------------------
// SSE event handler
//--------------------------------------------------------------------------------------------------

/**
 * @brief Routes a single SSE event into the Gemini chunk processor.
 */
void AI::GeminiReply::onSseEvent(const QString& name, const QJsonObject& data)
{
  Q_UNUSED(name);

  if (m_finished)
    return;

  processChunk(data);
}

/**
 * @brief Logs but does not abort on transient SSE parse errors.
 */
void AI::GeminiReply::onSseError(const QString& reason)
{
  qCWarning(serialStudioAI) << "Gemini SSE parse error:" << reason;
}

//--------------------------------------------------------------------------------------------------
// Chunk processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Walks one GenerateContentResponse chunk and emits text + tool calls.
 */
void AI::GeminiReply::processChunk(const QJsonObject& chunk)
{
  if (chunk.contains(QStringLiteral("error"))) {
    const auto err = chunk.value(QStringLiteral("error")).toObject();
    finishWithError(err.value(QStringLiteral("message")).toString());
    return;
  }

  const auto promptFeedback = chunk.value(QStringLiteral("promptFeedback")).toObject();
  const auto promptBlock    = promptFeedback.value(QStringLiteral("blockReason")).toString();
  if (!promptBlock.isEmpty()) {
    finishWithError(tr("Prompt blocked by Gemini safety filter: %1").arg(promptBlock));
    return;
  }

  const auto candidates = chunk.value(QStringLiteral("candidates")).toArray();
  if (candidates.isEmpty())
    return;

  const auto candidate = candidates.first().toObject();
  const auto content   = candidate.value(QStringLiteral("content")).toObject();
  const auto parts     = content.value(QStringLiteral("parts")).toArray();

  for (const auto& v : parts) {
    const auto part      = v.toObject();
    const bool isThought = part.value(QStringLiteral("thought")).toBool(false);

    const auto textValue = part.value(QStringLiteral("text"));
    if (textValue.isString()) {
      const auto chunkText = textValue.toString();
      if (chunkText.isEmpty())
        continue;

      if (isThought)
        Q_EMIT partialThinking(chunkText);
      else
        Q_EMIT partialText(chunkText);

      continue;
    }

    const auto fcValue = part.value(QStringLiteral("functionCall"));
    if (fcValue.isObject()) {
      const auto fc     = fcValue.toObject();
      const auto name   = fc.value(QStringLiteral("name")).toString();
      const auto args   = fc.value(QStringLiteral("args")).toObject();
      const auto callId = QUuid::createUuid().toString(QUuid::WithoutBraces);

      QJsonObject extras;
      const auto signature = part.value(QStringLiteral("thoughtSignature")).toString();
      if (!signature.isEmpty())
        extras[QStringLiteral("_gemini_thought_signature")] = signature;

      Q_EMIT toolCallRequested(callId, name, args, extras);
    }
  }

  const auto finishReason           = candidate.value(QStringLiteral("finishReason")).toString();
  static const QSet<QString> kFatal = {QStringLiteral("SAFETY"),
                                       QStringLiteral("RECITATION"),
                                       QStringLiteral("OTHER"),
                                       QStringLiteral("BLOCKLIST"),
                                       QStringLiteral("PROHIBITED_CONTENT"),
                                       QStringLiteral("SPII")};
  if (kFatal.contains(finishReason))
    finishWithError(tr("Gemini stopped without producing a response: %1").arg(finishReason));
}

//--------------------------------------------------------------------------------------------------
// QNetworkReply slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forwards every chunk into the SSE reader.
 */
void AI::GeminiReply::onReplyReadyRead()
{
  if (!m_reply || m_finished)
    return;

  const auto status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (status >= 400)
    return;

  m_sse->feed(m_reply->readAll());
}

/**
 * @brief Handles end-of-stream: drain remaining buffer and finalize.
 */
void AI::GeminiReply::onReplyFinished()
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
    handleHttpError(status);
    return;
  }

  m_sse->feed({});
  finishOk();
}

/**
 * @brief Reads a Gemini error body and dispatches finishWithError with a friendly message.
 */
void AI::GeminiReply::handleHttpError(int status)
{
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

  if (status == 401 || status == 403)
    finishWithError(tr("Invalid API key (%1)").arg(msg));
  else if (status == 429)
    finishWithError(tr("Rate limited: %1").arg(msg));
  else if (status == 400 && msg.contains(QStringLiteral("API_KEY_INVALID")))
    finishWithError(tr("Invalid API key"));
  else
    finishWithError(tr("Gemini %1: %2").arg(QString::number(status), msg));
}

//--------------------------------------------------------------------------------------------------
// Finalization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the stream finished, emits @ref finished.
 */
void AI::GeminiReply::finishOk()
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT finished();
}

/**
 * @brief Marks the stream finished with an error message.
 */
void AI::GeminiReply::finishWithError(const QString& message)
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT errorOccurred(message);
  Q_EMIT finished();
}
