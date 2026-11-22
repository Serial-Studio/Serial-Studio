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

#pragma once

#include <array>
#include <QByteArray>
#include <QByteArrayView>
#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QVariantMap>

#include "Console/SendSequence.h"
#include "Core/IO/ConnectionSequence.h"

namespace Console {

/**
 * @brief Bounds of the auto-reply rule table and its matching (spec 0127 R4-R7).
 */
inline constexpr int kMaxReplyRules          = 64;
inline constexpr int kMaxReplyMatchChars     = 256;
inline constexpr qsizetype kMaxMessageBytes  = 4096;
inline constexpr int kReplyGapMinMs          = 1;
inline constexpr int kReplyGapMaxMs          = 1000;
inline constexpr int kReplyGapDefaultMs      = 10;
inline constexpr int kMaxRepliesPerSecond    = 50;
inline constexpr qint64 kReplyRateWindowMs   = 1000;
inline constexpr int kMaxPatternAlternatives = 16;

/**
 * @brief How a rule's match text is compared with a received message.
 */
enum class ReplyMatchMode : quint8 {
  StartsWith,
  EndsWith,
  Equals,
  Contains,
  Pattern,
  Regex,
};

/**
 * @brief One auto-reply rule as the user wrote it. A text rule is tested against one received
 *        line; a hex rule against the bytes received until the line went idle.
 */
struct ReplyRule {
  QString match;
  TxPreset reply;
  ReplyMatchMode mode = ReplyMatchMode::StartsWith;
  bool hex            = false;
  bool enabled        = false;
};

/**
 * @brief A rule made ready to test messages: the bytes to look for, the parsed pattern
 *        alternatives or the compiled expression, and the reply already encoded.
 */
struct CompiledReplyRule {
  QByteArray needle;
  QByteArray replyBytes;
  QRegularExpression regex;
  QList<IO::ExpectPattern> patterns;
  ReplyMatchMode mode = ReplyMatchMode::StartsWith;
  bool hex            = false;
};

[[nodiscard]] ReplyRule replyRuleFromMap(const QVariantMap& map);
[[nodiscard]] QVariantMap replyRuleToMap(const ReplyRule& rule);
[[nodiscard]] QString replyRuleProblem(const ReplyRule& rule);
[[nodiscard]] bool compileReplyRule(const ReplyRule& rule, CompiledReplyRule& out);
[[nodiscard]] bool replyRuleMatches(const CompiledReplyRule& rule, QByteArrayView message);

/**
 * @brief Keeps the times of the last kMaxRepliesPerSecond replies (spec 0127 R7), so the limit
 *        holds over any one-second span and not only between fixed window edges: a device that
 *        echoes what it is sent would otherwise turn one rule into a storm.
 */
struct ReplyRateLimiter {
  std::array<qint64, kMaxRepliesPerSecond> sentMs = {};
  int head                                        = 0;
  int count                                       = 0;

  [[nodiscard]] bool allow(qint64 nowMs) noexcept;
  void reset() noexcept;
};

}  // namespace Console
