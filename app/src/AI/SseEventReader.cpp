/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/SseEventReader.h"

#include <QStringList>

#include "AI/Logging.h"
#include "Misc/JsonValidator.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty SSE reader with no parent ownership transfer.
 */
AI::SseEventReader::SseEventReader(QObject* parent) : QObject(parent) {}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a network chunk to the rolling buffer and drains complete frames.
 */
void AI::SseEventReader::feed(const QByteArray& chunk)
{
  if (chunk.isEmpty()) {
    drainFrames();
    return;
  }

  if (m_buffer.size() + chunk.size() > kMaxBufferBytes) {
    qCWarning(serialStudioAI) << "SSE buffer would exceed" << kMaxBufferBytes
                              << "bytes; dropping connection state";
    Q_EMIT parseError(QStringLiteral("buffer_overflow"));
    reset();
    return;
  }

  m_buffer.append(chunk);
  drainFrames();
}

/**
 * @brief Discards any partially-received frame data.
 */
void AI::SseEventReader::reset()
{
  m_buffer.clear();
}

/**
 * @brief Returns the number of bytes currently held in the buffer.
 */
qsizetype AI::SseEventReader::bufferedBytes() const noexcept
{
  return m_buffer.size();
}

//--------------------------------------------------------------------------------------------------
// Frame extraction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pulls every complete frame out of the buffer in order.
 */
void AI::SseEventReader::drainFrames()
{
  const qsizetype maxIterations = m_buffer.size();
  for (qsizetype safety = 0; safety < maxIterations; ++safety) {
    const auto end = m_buffer.indexOf("\n\n");
    if (end < 0)
      return;

    const auto frame = m_buffer.left(end);
    m_buffer.remove(0, end + 2);

    QString eventName;
    QByteArray dataPayload;
    parseFrameLines(frame, eventName, dataPayload);

    if (eventName.isEmpty())
      eventName = QStringLiteral("message");

    emitFrame(eventName, dataPayload);
  }
}

/**
 * @brief Parses one SSE frame's lines into eventName + dataPayload.
 */
void AI::SseEventReader::parseFrameLines(const QByteArray& frame,
                                         QString& eventName,
                                         QByteArray& dataPayload)
{
  const auto lines = frame.split('\n');
  for (const auto& line : lines) {
    if (line.isEmpty() || line.startsWith(':'))
      continue;

    const auto colon = line.indexOf(':');
    if (colon < 0)
      continue;

    const auto field = QString::fromUtf8(line.left(colon)).trimmed();
    auto value       = line.mid(colon + 1);
    if (value.startsWith(' '))
      value.remove(0, 1);

    if (field == QStringLiteral("event")) {
      eventName = QString::fromUtf8(value).trimmed();
      continue;
    }

    if (field != QStringLiteral("data"))
      continue;

    if (!dataPayload.isEmpty())
      dataPayload.append('\n');

    dataPayload.append(value);
  }
}

/**
 * @brief Validates and emits a single SSE event after JSON parsing.
 */
void AI::SseEventReader::emitFrame(const QString& name, const QByteArray& data)
{
  if (name == QStringLiteral("ping"))
    return;

  if (data.isEmpty()) {
    Q_EMIT frameReceived(name, QJsonObject());
    return;
  }

  if (data.size() > kMaxPayloadBytes) {
    qCWarning(serialStudioAI) << "SSE frame" << name << "exceeds payload limit";
    Q_EMIT parseError(QStringLiteral("payload_too_large"));
    return;
  }

  Misc::JsonValidator::Limits limits;
  limits.maxFileSize  = kMaxPayloadBytes;
  limits.maxDepth     = 32;
  limits.maxArraySize = 1000;

  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);
  if (!result.valid || !result.document.isObject()) {
    qCWarning(serialStudioAI) << "SSE frame" << name << "JSON invalid:" << result.errorMessage;
    Q_EMIT parseError(result.errorMessage);
    return;
  }

  Q_EMIT frameReceived(name, result.document.object());
}
