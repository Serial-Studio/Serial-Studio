/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace AI {

/**
 * @brief What a user-attached path is, decided from its first bytes and never its extension.
 */
enum class AttachmentKind : quint8 {
  Pdf,
  Image,
  Text,
  Folder,
  Other,
};

/**
 * @brief One attached path as the composer shows it and the history references it.
 */
struct AttachmentInfo {
  QString path;
  QString name;
  QString mediaType;
  AttachmentKind kind;
  qint64 bytes;
};

/**
 * @brief JSON-only helpers over the history's attachment reference block
 *        (`{"type":"attachment","name","kind","media_type","bytes","path"}`). Header-only so the
 *        budgeter and the request translators use them without linking the file reader.
 */
namespace AttachmentBlock {

inline constexpr int kImageTokens       = 1600;
inline constexpr int kPdfBytesPerToken  = 40;
inline constexpr int kTextBytesPerToken = 4;

/**
 * @brief True for an attachment reference block.
 */
[[nodiscard]] inline bool isAttachment(const QJsonObject& block)
{
  return block.value(QStringLiteral("type")).toString() == QStringLiteral("attachment");
}

/**
 * @brief Token estimate of one reference block: a per-byte guess for a PDF (pages cannot be
 *        counted without a parser), a flat figure for an image, the usual ratio for text.
 */
[[nodiscard]] inline int estimateTokens(const QJsonObject& block)
{
  if (!isAttachment(block))
    return 0;

  const auto kind  = block.value(QStringLiteral("kind")).toString();
  const auto bytes = block.value(QStringLiteral("bytes")).toInteger(0);
  if (kind == QStringLiteral("image"))
    return kImageTokens;

  const int divisor   = kind == QStringLiteral("pdf") ? kPdfBytesPerToken : kTextBytesPerToken;
  const qint64 tokens = bytes / divisor;
  return static_cast<int>(qBound(qint64{0}, tokens, qint64{4'000'000}));
}

/**
 * @brief Token estimate of every reference block one history message carries.
 */
[[nodiscard]] inline int estimateMessageTokens(const QJsonObject& message)
{
  int total         = 0;
  const auto blocks = message.value(QStringLiteral("content")).toArray();
  for (const auto& value : blocks)
    total += estimateTokens(value.toObject());

  return total;
}

/**
 * @brief Display names of the reference blocks in @p blocks, in order.
 */
[[nodiscard]] inline QStringList names(const QJsonArray& blocks)
{
  QStringList out;
  for (const auto& value : blocks) {
    const auto block = value.toObject();
    if (isAttachment(block))
      out.append(block.value(QStringLiteral("name")).toString());
  }

  return out;
}

}  // namespace AttachmentBlock

/**
 * @brief Turns the history's attachment references into what a request can carry: encoded
 *        bytes for a PDF or an image the provider takes, the contents of a text file, a
 *        one-line note for the rest. Bytes are read through the file sandbox per request, so a
 *        withdrawn permission stops the file.
 */
class AttachmentResolver {
public:
  static constexpr qint64 kMaxRequestBytes    = 12 * 1024 * 1024;
  static constexpr qint64 kMaxInlineTextBytes = 256 * 1024;
  static constexpr int kSniffBytes            = 8192;

  [[nodiscard]] static AttachmentInfo probe(const QString& canonicalPath);
  [[nodiscard]] static QString kindName(AttachmentKind kind);
  [[nodiscard]] static QJsonObject toBlock(const AttachmentInfo& info);
  [[nodiscard]] static QJsonObject noteBlock(const QString& name, const QString& reason);

  [[nodiscard]] QJsonArray resolve(const QJsonArray& history, bool pdfInput, bool imageInput);
  void clear();

private:
  [[nodiscard]] QJsonObject resolveBlock(const QJsonObject& block,
                                         bool pdfInput,
                                         bool imageInput,
                                         qint64& bytesLeft);
  [[nodiscard]] QByteArray payload(const QString& path, bool base64, qint64 maxBytes);

  QHash<QString, QByteArray> m_payloads;
};

}  // namespace AI
