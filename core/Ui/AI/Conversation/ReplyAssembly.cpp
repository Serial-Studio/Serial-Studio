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

#include "AI/Conversation/ReplyAssembly.h"

#include <QRegularExpression>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Untrusted-payload framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Neutralizes any forged <untrusted> delimiter inside untrusted payload text.
 */
QString AI::ReplyAssembly::neutralizeHistoryDelimiter(const QString& payload)
{
  QString out = payload;
  out.replace(QStringLiteral("</untrusted"), QStringLiteral("< /untrusted"), Qt::CaseInsensitive);
  out.replace(QStringLiteral("<untrusted"), QStringLiteral("< untrusted"), Qt::CaseInsensitive);
  return out;
}

/**
 * @brief Wraps a serialized tool payload in the <untrusted> envelope the system prompt
 *        teaches the model to treat as data, with the source tag escaped and any forged
 *        delimiter inside the payload defanged first.
 */
QString AI::ReplyAssembly::wrapUntrusted(const QString& sourceTag, const QByteArray& contentBytes)
{
  QString wrapped;
  wrapped += QStringLiteral("<untrusted source=\"");
  wrapped += sourceTag.toHtmlEscaped();
  wrapped += QStringLiteral("\">\n");
  wrapped += neutralizeHistoryDelimiter(QString::fromUtf8(contentBytes));
  wrapped += QStringLiteral("\n</untrusted>");
  return wrapped;
}

/**
 * @brief Rewrites GitHub doc URLs in assistant text to their public help-site equivalents.
 *        Idempotent; safe to call repeatedly on streaming chunks.
 */
QString AI::ReplyAssembly::rewriteHelpLinks(const QString& text)
{
  if (text.isEmpty())
    return text;

  if (!text.contains(QLatin1String("github.com/Serial-Studio"))
      && !text.contains(QLatin1String("githubusercontent.com/Serial-Studio")))
    return text;

  static const QRegularExpression re(
    QStringLiteral("https://(?:github\\.com|raw\\.githubusercontent\\.com)/"
                   "Serial-Studio/Serial-Studio/"
                   "(?:blob|tree)?/?[A-Za-z0-9._\\-]+/"
                   "doc/(?:help/)?([A-Za-z0-9_\\-]+)\\.md"
                   "(?:#[A-Za-z0-9_\\-]*)?"));

  if (!re.isValid())
    return text;

  QString out      = text;
  int searchOffset = 0;
  // code-verify off -- bound is number of regex matches in finite `out`
  while (true) {
    const auto m = re.match(out, searchOffset);
    if (!m.hasMatch())
      break;

    const auto pageName = m.captured(1);
    QString slug        = pageName.toLower();
    slug.replace(QLatin1Char('_'), QLatin1Char('-'));
    const QString replacement = QStringLiteral("https://serial-studio.com/help#") + slug;

    out.replace(m.capturedStart(0), m.capturedLength(0), replacement);
    searchOffset = m.capturedStart(0) + replacement.size();
  }
  // code-verify on

  return out;
}

//--------------------------------------------------------------------------------------------------
// History block builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the history tool_use block, folding provider extras (underscore-prefixed
 *        passthrough fields such as Gemini thought signatures) into it.
 */
QJsonObject AI::ReplyAssembly::makeToolUseBlock(const QString& callId,
                                                const QString& name,
                                                const QJsonObject& arguments,
                                                const QJsonObject& extras)
{
  SS_ASSERT_LOG(!callId.isEmpty());

  QJsonObject block;
  block[QStringLiteral("type")]  = QStringLiteral("tool_use");
  block[QStringLiteral("id")]    = callId;
  block[QStringLiteral("name")]  = name;
  block[QStringLiteral("input")] = arguments;
  for (auto it = extras.constBegin(); it != extras.constEnd(); ++it)
    if (it.key().startsWith(QLatin1Char('_')))
      block[it.key()] = it.value();

  return block;
}

/**
 * @brief Assembles the assistant history message for a finished turn: echoed thinking
 *        blocks first, then the visible text, then the tool_use blocks in request order.
 *        Returns an empty object when the turn produced neither text nor tool calls, which
 *        is the caller's signal to append nothing.
 */
QJsonObject AI::ReplyAssembly::makeAssistantMessage(const QJsonArray& thinkingBlocks,
                                                    const QString& text,
                                                    const QJsonArray& toolUseBlocks)
{
  if (text.isEmpty() && toolUseBlocks.isEmpty())
    return {};

  QJsonArray content;
  for (const auto& tb : thinkingBlocks)
    content.append(tb);

  if (!text.isEmpty()) {
    QJsonObject textBlock;
    textBlock[QStringLiteral("type")] = QStringLiteral("text");
    textBlock[QStringLiteral("text")] = text;
    content.append(textBlock);
  }

  for (const auto& tu : toolUseBlocks)
    content.append(tu);

  QJsonObject assistant;
  assistant[QStringLiteral("role")]    = QStringLiteral("assistant");
  assistant[QStringLiteral("content")] = content;
  return assistant;
}

/**
 * @brief Builds a budget-respecting replacement for an oversized tool result: keeps the
 *        ok/error fields, flags the cut, and carries a raw-JSON preview plus guidance so
 *        the model narrows the call instead of retrying it verbatim.
 */
QJsonObject AI::ReplyAssembly::makeTruncatedResult(const QJsonObject& scrubbed,
                                                   const QByteArray& fullBytes,
                                                   int budgetBytes)
{
  SS_ASSERT(budgetBytes > 512, budgetBytes = 1024);

  QJsonObject out;
  if (scrubbed.contains(QStringLiteral("ok")))
    out[QStringLiteral("ok")] = scrubbed.value(QStringLiteral("ok"));

  if (scrubbed.contains(QStringLiteral("error")))
    out[QStringLiteral("error")] = scrubbed.value(QStringLiteral("error"));

  out[QStringLiteral("truncated")] = true;
  out[QStringLiteral("note")] =
    QStringLiteral("Result was TOO LARGE for the %1-byte tool-result budget; 'preview' holds "
                   "only its first bytes, and retrying the identical call will truncate again. "
                   "Narrow the call instead: pass offset/limit to page, a query/type filter "
                   "where supported, or find the item directly with project.search / "
                   "meta.search (meta.describeCommand{name} lists each command's paging "
                   "params). This is a size limit, not the transcript-aging stub.")
      .arg(budgetBytes);
  out[QStringLiteral("preview")] = QString::fromUtf8(fullBytes.left(budgetBytes - 512));
  return out;
}

/**
 * @brief Builds the tool_result history block for one completed call: the untrusted-wrapped
 *        text form every provider reads, the structured Gemini mirror, and the _tool_name
 *        tag the transcript-aging pass keys its exemptions off.
 */
QJsonObject AI::ReplyAssembly::makeToolResultBlock(const QString& callId,
                                                   const QString& name,
                                                   const QJsonObject& effective,
                                                   const QByteArray& contentBytes)
{
  SS_ASSERT_LOG(!callId.isEmpty());

  const auto sourceTag = name.isEmpty() ? QStringLiteral("tool_result") : name;

  QJsonObject block;
  block[QStringLiteral("type")]                       = QStringLiteral("tool_result");
  block[QStringLiteral("tool_use_id")]                = callId;
  block[QStringLiteral("content")]                    = wrapUntrusted(sourceTag, contentBytes);
  QJsonObject geminiPayload                           = effective;
  geminiPayload[QStringLiteral("__untrusted_source")] = sourceTag;
  block[QStringLiteral("_gemini_response")]           = geminiPayload;
  if (!name.isEmpty())
    block[QStringLiteral("_tool_name")] = name;

  return block;
}
