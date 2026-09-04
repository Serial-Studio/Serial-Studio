/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Redactor.h"

#include <QJsonValue>
#include <QRegularExpression>

namespace detail {

/**
 * @brief One redaction pattern paired with its replacement reason.
 */
struct Pattern {
  QRegularExpression re;
  QString reason;
};

}  // namespace detail

/**
 * @brief Defensive bound on JSON recursion depth; deeper values pass through unmodified.
 */
static constexpr int kMaxScrubDepth = 128;

/**
 * @brief Returns the static pattern list. Built lazily on first use and intentionally
 *        never destroyed: scrub() runs inside ~Assistant during static finalization
 *        (snapshot -> handoff digest), and a stack-destroyed list crashed on quit there
 *        (2026-07-14). The one-time heap allocation is released by the OS at exit.
 *
 * Patterns are ordered most-specific first to keep the redaction reason
 * informative for the model. JWT regex is anchored on the three-base64-
 * segment shape with a leading `eyJ` so it doesn't match arbitrary base64.
 */
static const QList<detail::Pattern>& patterns()
{
  static const auto* kPatterns = new QList<detail::Pattern>{
    {                               QRegularExpression(QStringLiteral("\\bsk-[A-Za-z0-9_\\-]{20,}\\b")),
     QStringLiteral("api_key")          },
    {                               QRegularExpression(QStringLiteral("\\bpk-[A-Za-z0-9_\\-]{20,}\\b")),
     QStringLiteral("api_key")          },
    {                               QRegularExpression(QStringLiteral("\\brk_[A-Za-z0-9_\\-]{20,}\\b")),
     QStringLiteral("api_key")          },
    {                          QRegularExpression(QStringLiteral("\\bxox[baprs]-[A-Za-z0-9-]{10,}\\b")),
     QStringLiteral("slack_token")      },
    {                                QRegularExpression(QStringLiteral("\\bxapp-[A-Za-z0-9-]{10,}\\b")),
     QStringLiteral("slack_token")      },
    {                             QRegularExpression(QStringLiteral("\\bgh[opsu]_[A-Za-z0-9]{36,}\\b")),
     QStringLiteral("github_token")     },
    {                            QRegularExpression(QStringLiteral("\\bglpat-[A-Za-z0-9_\\-]{20,}\\b")),
     QStringLiteral("gitlab_token")     },
    {                               QRegularExpression(QStringLiteral("\\bAIza[A-Za-z0-9_\\-]{35}\\b")),
     QStringLiteral("google_api_key")   },
    {                                      QRegularExpression(QStringLiteral("\\bAKIA[A-Z0-9]{16}\\b")),
     QStringLiteral("aws_access_key_id")},
    {QRegularExpression(
QStringLiteral("\\beyJ[A-Za-z0-9_\\-]{8,}\\.[A-Za-z0-9_\\-]{8,}\\.[A-Za-z0-9_\\-]{8,}\\b")),
     QStringLiteral("jwt")              },
    {                 QRegularExpression(QStringLiteral("(?i)\\bbearer\\s+[A-Za-z0-9_\\-\\.]{20,}\\b")),
     QStringLiteral("bearer_token")     },
    {                      QRegularExpression(QStringLiteral("\\bSS-PRO-[A-Z0-9]{4,}-[A-Z0-9]{4,}\\b")),
     QStringLiteral("license_key")      },
    {                                                                QRegularExpression(
                                                                QStringLiteral("-----BEGIN (?:RSA|OPENSSH|EC|DSA|ENCRYPTED|PRIVATE) ?(?:PRIVATE )?KEY-----"
                                                                "[\\s\\S]*?-----END (?:RSA|OPENSSH|EC|DSA|ENCRYPTED|PRIVATE) ?(?:PRIVATE )?"
                                                                "KEY-----")),
     QStringLiteral("private_key")      },
  };
  return *kPatterns;
}

/**
 * @brief Replaces any sensitive-shaped substring with [REDACTED:<reason>].
 */
bool AI::Redactor::scrub(QString& text)
{
  if (text.isEmpty())
    return false;

  bool changed = false;
  for (const auto& p : patterns()) {
    if (!p.re.match(text).hasMatch())
      continue;

    const auto replacement = QStringLiteral("[REDACTED:") + p.reason + QStringLiteral("]");
    text.replace(p.re, replacement);
    changed = true;
  }
  return changed;
}

/**
 * @brief Recursively scrubs a single JSON leaf, returning the redacted value.
 */
static QJsonValue scrubValue(const QJsonValue& v, int depth)
{
  if (v.isString()) {
    auto s = v.toString();
    (void)AI::Redactor::scrub(s);
    return s;
  }

  if (v.isObject())
    return AI::Redactor::scrubObject(v.toObject(), depth + 1);

  if (v.isArray())
    return AI::Redactor::scrubArray(v.toArray(), depth + 1);

  return v;
}

/**
 * @brief Walks every string leaf inside a QJsonObject and scrubs it; objects nested
 *        beyond kMaxScrubDepth are returned unmodified.
 */
QJsonObject AI::Redactor::scrubObject(const QJsonObject& obj, int depth)
{
  if (depth >= kMaxScrubDepth)
    return obj;

  QJsonObject out;
  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
    out.insert(it.key(), scrubValue(it.value(), depth));

  return out;
}

/**
 * @brief Walks every string leaf inside a QJsonArray and scrubs it; arrays nested
 *        beyond kMaxScrubDepth are returned unmodified.
 */
QJsonArray AI::Redactor::scrubArray(const QJsonArray& arr, int depth)
{
  if (depth >= kMaxScrubDepth)
    return arr;

  QJsonArray out;
  for (const auto& v : arr)
    out.append(scrubValue(v, depth));

  return out;
}
