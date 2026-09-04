/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

namespace AI {

/**
 * @brief Streaming Server-Sent-Events parser used by AnthropicReply.
 */
class SseEventReader : public QObject {
  Q_OBJECT

public:
  static constexpr int kMaxPayloadBytes = 1 * 1024 * 1024;
  static constexpr int kMaxBufferBytes  = 4 * 1024 * 1024;

  static constexpr const char* kErrBufferOverflow  = "buffer_overflow";
  static constexpr const char* kErrPayloadTooLarge = "payload_too_large";

  explicit SseEventReader(QObject* parent = nullptr);

  void feed(const QByteArray& chunk);
  void reset();

  [[nodiscard]] qsizetype bufferedBytes() const noexcept;
  [[nodiscard]] static bool fatalReason(const QString& reason);

signals:
  void frameReceived(const QString& name, const QJsonObject& data);
  void parseError(const QString& reason);

private:
  void drainFrames();
  void emitFrame(const QString& name, const QByteArray& data);
  [[nodiscard]] QByteArray normalizeNewlines(const QByteArray& chunk);
  static void parseFrameLines(const QByteArray& frame, QString& eventName, QByteArray& dataPayload);

  QByteArray m_buffer;
  bool m_pendingCr;
};

}  // namespace AI
