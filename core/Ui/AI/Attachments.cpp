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

#include "AI/Attachments.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

#include "AI/FileSandbox.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Kind detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Media type of an image by its magic number, empty when @p head is not one the
 *        providers take (PNG, JPEG, GIF, WebP).
 */
static QString imageMediaType(const QByteArray& head)
{
  if (head.startsWith("\x89PNG\r\n\x1a\n"))
    return QStringLiteral("image/png");

  if (head.startsWith("\xFF\xD8\xFF"))
    return QStringLiteral("image/jpeg");

  if (head.startsWith("GIF87a") || head.startsWith("GIF89a"))
    return QStringLiteral("image/gif");

  if (head.startsWith("RIFF") && head.mid(8, 4) == "WEBP")
    return QStringLiteral("image/webp");

  return {};
}

/**
 * @brief True when a byte sample decodes as UTF-8 text with no NUL byte.
 */
static bool looksLikeText(const QByteArray& sample)
{
  if (sample.contains('\0'))
    return false;

  auto decoder = QStringDecoder(QStringDecoder::Utf8);
  (void)QString(decoder.decode(sample));
  return !decoder.hasError();
}

/**
 * @brief Classifies a canonical path by its first bytes. A file that cannot be opened reads
 *        as Other, which the send gate refuses by name.
 */
AI::AttachmentInfo AI::AttachmentResolver::probe(const QString& canonicalPath)
{
  const QFileInfo file_info(canonicalPath);
  AttachmentInfo info{canonicalPath, file_info.fileName(), QString(), AttachmentKind::Other, 0};
  if (file_info.isDir()) {
    info.kind = AttachmentKind::Folder;
    return info;
  }

  QFile file(canonicalPath);
  if (!file.open(QIODevice::ReadOnly))
    return info;

  info.bytes      = file.size();
  const auto head = file.read(kSniffBytes);
  if (head.startsWith("%PDF-")) {
    info.kind      = AttachmentKind::Pdf;
    info.mediaType = QStringLiteral("application/pdf");
    return info;
  }

  const auto image = imageMediaType(head);
  if (!image.isEmpty()) {
    info.kind      = AttachmentKind::Image;
    info.mediaType = image;
    return info;
  }

  if (looksLikeText(head)) {
    info.kind      = AttachmentKind::Text;
    info.mediaType = QStringLiteral("text/plain");
  }

  return info;
}

/**
 * @brief The kind's name as the history block and the composer chip carry it.
 */
QString AI::AttachmentResolver::kindName(AttachmentKind kind)
{
  switch (kind) {
    case AttachmentKind::Pdf:
      return QStringLiteral("pdf");
    case AttachmentKind::Image:
      return QStringLiteral("image");
    case AttachmentKind::Text:
      return QStringLiteral("text");
    case AttachmentKind::Folder:
      return QStringLiteral("folder");
    case AttachmentKind::Other:
      return QStringLiteral("other");
  }

  return QStringLiteral("other");
}

//--------------------------------------------------------------------------------------------------
// History blocks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the reference block the history and the saved chat keep: metadata and the
 *        path, never the bytes.
 */
QJsonObject AI::AttachmentResolver::toBlock(const AttachmentInfo& info)
{
  SS_ASSERT_LOG(!info.path.isEmpty());

  QJsonObject block;
  block[QStringLiteral("type")]       = QStringLiteral("attachment");
  block[QStringLiteral("name")]       = info.name;
  block[QStringLiteral("kind")]       = kindName(info.kind);
  block[QStringLiteral("media_type")] = info.mediaType;
  block[QStringLiteral("bytes")]      = static_cast<double>(info.bytes);
  block[QStringLiteral("path")]       = info.path;
  return block;
}

/**
 * @brief The one fallback every unsendable attachment becomes: a text block telling the model
 *        the file exists, why it did not arrive, and what to ask for.
 */
QJsonObject AI::AttachmentResolver::noteBlock(const QString& name, const QString& reason)
{
  QJsonObject block;
  block[QStringLiteral("type")] = QStringLiteral("text");
  block[QStringLiteral("text")] =
    QStringLiteral("[Attachment \"%1\" is not available to you: %2. Ask the user to attach it "
                   "again if you need it.]")
      .arg(name, reason);
  return block;
}

//--------------------------------------------------------------------------------------------------
// Resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the file's bytes for a request, base64 for a native block and raw for text,
 *        empty when the sandbox no longer lets the file be read. The cache key carries size and
 *        modification time, so an edited file is read again.
 */
QByteArray AI::AttachmentResolver::payload(const QString& path, bool base64, qint64 maxBytes)
{
  const QFileInfo info(path);
  const auto key = QStringLiteral("%1|%2|%3|%4")
                     .arg(path)
                     .arg(info.size())
                     .arg(info.lastModified().toMSecsSinceEpoch())
                     .arg(base64 ? 1 : 0);

  static auto& fileSandbox = AI::FileSandbox::instance();
  const auto raw           = fileSandbox.readAttachment(path, maxBytes);
  if (raw.isEmpty()) {
    m_payloads.remove(key);
    return {};
  }

  const auto cached = m_payloads.constFind(key);
  if (cached != m_payloads.constEnd())
    return cached.value();

  const auto stale = path + QLatin1Char('|');
  m_payloads.removeIf([&stale](const auto& entry) { return entry.key().startsWith(stale); });

  const auto data = base64 ? raw.toBase64() : raw;
  m_payloads.insert(key, data);
  return data;
}

/**
 * @brief Resolves one reference block against the provider's abilities and the bytes still
 *        allowed in this request. The result is a text block, or the reference with its
 *        encoded bytes under `_data` and the path removed.
 */
QJsonObject AI::AttachmentResolver::resolveBlock(const QJsonObject& block,
                                                 bool pdfInput,
                                                 bool imageInput,
                                                 qint64& bytesLeft)
{
  const auto name  = block.value(QStringLiteral("name")).toString();
  const auto kind  = block.value(QStringLiteral("kind")).toString();
  const auto path  = block.value(QStringLiteral("path")).toString();
  const auto bytes = block.value(QStringLiteral("bytes")).toInteger(0);

  const bool is_text = kind == QStringLiteral("text");
  if (kind == QStringLiteral("pdf") && !pdfInput)
    return noteBlock(name, QStringLiteral("the selected model cannot read PDF files"));

  if (kind == QStringLiteral("image") && !imageInput)
    return noteBlock(name, QStringLiteral("the selected model cannot read images"));

  if (bytes > bytesLeft)
    return noteBlock(name, QStringLiteral("the request size limit was reached"));

  const auto data = payload(path, !is_text, is_text ? kMaxInlineTextBytes : kMaxRequestBytes);
  if (data.isEmpty())
    return noteBlock(name, QStringLiteral("the file can no longer be read, or it has grown"));

  bytesLeft -= bytes;
  if (is_text) {
    QJsonObject text;
    text[QStringLiteral("type")] = QStringLiteral("text");
    text[QStringLiteral("text")] =
      QStringLiteral("Attached file \"%1\":\n\n%2").arg(name, QString::fromUtf8(data));
    return text;
  }

  auto resolved = block;
  resolved.remove(QStringLiteral("path"));
  resolved[QStringLiteral("_data")] = QString::fromLatin1(data);
  return resolved;
}

/**
 * @brief Returns @p history with every attachment reference resolved for one request. The
 *        newest messages are served first, so when the request size cap runs out it is the
 *        oldest attachment that becomes a note.
 */
QJsonArray AI::AttachmentResolver::resolve(const QJsonArray& history,
                                           bool pdfInput,
                                           bool imageInput)
{
  auto out          = history;
  qint64 bytes_left = kMaxRequestBytes;
  for (auto i = history.size() - 1; i >= 0; --i) {
    auto message      = history.at(i).toObject();
    const auto blocks = message.value(QStringLiteral("content")).toArray();
    if (AttachmentBlock::names(blocks).isEmpty())
      continue;

    QJsonArray content;
    for (const auto& value : blocks) {
      const auto block = value.toObject();
      if (AttachmentBlock::isAttachment(block))
        content.append(resolveBlock(block, pdfInput, imageInput, bytes_left));
      else
        content.append(block);
    }

    message[QStringLiteral("content")] = content;
    out[i]                             = message;
  }

  SS_ASSERT_LOG(out.size() == history.size());
  return out;
}

/**
 * @brief Drops every cached payload; a new or reloaded chat starts with none.
 */
void AI::AttachmentResolver::clear()
{
  m_payloads.clear();
}
