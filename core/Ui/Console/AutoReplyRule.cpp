/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Console/AutoReplyRule.h"

#include <QCoreApplication>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kMatchKey       = QStringLiteral("match");
static const QString kRuleModeKey    = QStringLiteral("mode");
static const QString kRuleHexKey     = QStringLiteral("matchHex");
static const QString kRuleEnabledKey = QStringLiteral("enabled");
static const QString kReplyKey       = QStringLiteral("reply");

/**
 * @brief Translates a rule problem in the auto-reply context.
 */
[[nodiscard]] static QString trRule(const char* text)
{
  return QCoreApplication::translate("Console::AutoReply", text);
}

//--------------------------------------------------------------------------------------------------
// Storage form
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads a rule from its stored map; an unknown mode falls back to "starts with".
 */
Console::ReplyRule Console::replyRuleFromMap(const QVariantMap& map)
{
  ReplyRule rule;
  rule.match   = map.value(kMatchKey).toString();
  rule.hex     = map.value(kRuleHexKey, false).toBool();
  rule.enabled = map.value(kRuleEnabledKey, false).toBool();
  rule.reply   = txPresetFromMap(map.value(kReplyKey).toMap());

  const int mode = map.value(kRuleModeKey, 0).toInt();
  if (mode >= 0 && mode <= static_cast<int>(ReplyMatchMode::Regex))
    rule.mode = static_cast<ReplyMatchMode>(mode);

  return rule;
}

/**
 * @brief Writes a rule as the map the settings file and the editor share.
 */
QVariantMap Console::replyRuleToMap(const ReplyRule& rule)
{
  QVariantMap reply;
  writeTxPreset(rule.reply, reply);

  QVariantMap map;
  map.insert(kMatchKey, rule.match);
  map.insert(kRuleModeKey, static_cast<int>(rule.mode));
  map.insert(kRuleHexKey, rule.hex);
  map.insert(kRuleEnabledKey, rule.enabled);
  map.insert(kReplyKey, reply);
  return map;
}

//--------------------------------------------------------------------------------------------------
// Validation and compilation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether @p text is an even run of hexadecimal digits once spaces are removed.
 */
[[nodiscard]] static bool isHexBytes(const QString& text)
{
  QString compact = text.simplified();
  compact.remove(QLatin1Char(' '));
  if (compact.isEmpty() || compact.size() % 2 != 0)
    return false;

  for (const QChar c : std::as_const(compact)) {
    const bool digit = (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
                    || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
                    || (c >= QLatin1Char('A') && c <= QLatin1Char('F'));
    if (!digit)
      return false;
  }

  return true;
}

/**
 * @brief Parses the "|"-separated alternatives of a pattern rule; false when one is malformed.
 */
[[nodiscard]] static bool parsePatterns(const QString& text, QList<IO::ExpectPattern>& out)
{
  out.clear();
  const auto parts = text.split(QLatin1Char('|'), Qt::SkipEmptyParts);
  if (parts.isEmpty() || parts.size() > Console::kMaxPatternAlternatives)
    return false;

  for (const auto& part : parts) {
    IO::ExpectPattern pattern;
    if (!IO::ExpectPattern::parse(part, pattern))
      return false;

    out.append(pattern);
  }

  return true;
}

/**
 * @brief The first reason @p rule cannot be saved, as a sentence for the editor; empty when it is
 *        valid. This is the one place a rule is judged, so the editor and the loader agree.
 */
QString Console::replyRuleProblem(const ReplyRule& rule)
{
  if (rule.match.isEmpty())
    return trRule("Enter the text or bytes to look for.");

  if (rule.match.size() > kMaxReplyMatchChars)
    return trRule("The match text is too long.");

  if (rule.reply.payload.isEmpty())
    return trRule("Enter the reply to send.");

  QList<IO::ExpectPattern> patterns;
  if (rule.mode == ReplyMatchMode::Pattern && !parsePatterns(rule.match, patterns))
    return trRule("A pattern is hexadecimal bytes such as 5D ?? 7F, with | between alternatives.");

  if (rule.mode == ReplyMatchMode::Regex && rule.hex)
    return trRule("Regular expressions work on text rules only.");

  if (rule.mode == ReplyMatchMode::Regex && !QRegularExpression(rule.match).isValid())
    return trRule("The regular expression is not valid.");

  const bool literal = rule.mode != ReplyMatchMode::Pattern && rule.mode != ReplyMatchMode::Regex;
  if (rule.hex && literal && !isHexBytes(rule.match))
    return trRule("A hexadecimal rule needs whole bytes, for example 01 03 00.");

  if ((rule.reply.hex && !isHexBytes(rule.reply.payload)) || encodeTxPreset(rule.reply).isEmpty())
    return trRule("The reply could not be encoded. Check its hexadecimal digits.");

  return {};
}

/**
 * @brief Builds the matcher for a valid rule and encodes its reply once, so nothing is parsed or
 *        compiled when a message arrives. False for a rule replyRuleProblem() refuses.
 */
bool Console::compileReplyRule(const ReplyRule& rule, CompiledReplyRule& out)
{
  out = CompiledReplyRule();
  if (!replyRuleProblem(rule).isEmpty())
    return false;

  out.mode       = rule.mode;
  out.hex        = rule.hex;
  out.replyBytes = encodeTxPreset(rule.reply);

  if (rule.mode == ReplyMatchMode::Pattern)
    return parsePatterns(rule.match, out.patterns);

  if (rule.mode == ReplyMatchMode::Regex) {
    out.regex = QRegularExpression(rule.match);
    out.regex.optimize();
    return out.regex.isValid();
  }

  out.needle = rule.hex ? QByteArray::fromHex(rule.match.toLatin1()) : rule.match.toUtf8();
  SS_ASSERT(!out.needle.isEmpty(), return false);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Matching
//--------------------------------------------------------------------------------------------------

/**
 * @brief Tests one message against one compiled rule. A message longer than the cap is judged on
 *        its first kMaxMessageBytes bytes, which bounds every mode's input.
 */
bool Console::replyRuleMatches(const CompiledReplyRule& rule, const QByteArrayView message)
{
  SS_ASSERT(message.size() >= 0, return false);
  const auto head = message.first(qMin(message.size(), kMaxMessageBytes));
  if (head.isEmpty())
    return false;

  switch (rule.mode) {
    case ReplyMatchMode::StartsWith:
      return head.startsWith(rule.needle);
    case ReplyMatchMode::EndsWith:
      return head.endsWith(rule.needle);
    case ReplyMatchMode::Equals:
      return head == QByteArrayView(rule.needle);
    case ReplyMatchMode::Contains:
      return head.indexOf(QByteArrayView(rule.needle)) >= 0;
    case ReplyMatchMode::Regex:
      return rule.regex.match(QString::fromUtf8(head)).hasMatch();
    case ReplyMatchMode::Pattern:
      break;
  }

  const auto buffer = QByteArray::fromRawData(head.data(), head.size());
  for (const auto& pattern : rule.patterns)
    if (pattern.matchIn(buffer) >= 0)
      return true;

  return false;
}

//--------------------------------------------------------------------------------------------------
// Rate limit
//--------------------------------------------------------------------------------------------------

/**
 * @brief Counts one reply at @p nowMs and returns whether it is still within the limit of
 *        kMaxRepliesPerSecond per window.
 */
bool Console::ReplyRateLimiter::allow(const qint64 nowMs) noexcept
{
  SS_ASSERT(count >= 0 && count <= kMaxRepliesPerSecond, reset());
  SS_ASSERT(head >= 0 && head < kMaxRepliesPerSecond, reset());

  const qint64 oldest = sentMs[static_cast<size_t>(head)];
  if (count == kMaxRepliesPerSecond && nowMs >= oldest && nowMs - oldest < kReplyRateWindowMs)
    return false;

  sentMs[static_cast<size_t>(head)] = nowMs;
  head                              = (head + 1) % kMaxRepliesPerSecond;
  count                             = qMin(count + 1, kMaxRepliesPerSecond);
  return true;
}

/**
 * @brief Forgets the current window.
 */
void Console::ReplyRateLimiter::reset() noexcept
{
  head  = 0;
  count = 0;
}
