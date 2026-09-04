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

#include "AI/SentinelProbe.h"

#include <QSettings>
#include <QStringList>

#include "AI/Logging.h"

/**
 * @brief Returns the opening marker every sentinel line starts with.
 */
static const QString& sentinelMarker()
{
  static const QString kMarker = QStringLiteral("[[SS-CHECK");
  return kMarker;
}

/**
 * @brief Returns the sentinel's key=value anchor tokens (behavioral rules, not build facts).
 */
static const QStringList& sentinelTokens()
{
  static const QStringList kTokens = {
    QStringLiteral("tools=approved"),
    QStringLiteral("untrusted=data"),
    QStringLiteral("ids=fresh"),
    QStringLiteral("batch=bulk"),
    QStringLiteral("save=auto"),
  };
  return kTokens;
}

/**
 * @brief Returns the first expected token missing from a mutated sentinel body, or a
 *        format hint when all tokens survive but the line no longer matches verbatim.
 */
static QString firstDriftedToken(const QString& body)
{
  for (const auto& token : sentinelTokens())
    if (!body.contains(token))
      return token;

  return QStringLiteral("(format)");
}

//--------------------------------------------------------------------------------------------------
// Static sentinel text
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the exact sentinel line the model must reproduce verbatim.
 */
QString AI::SentinelProbe::sentinelLine()
{
  static const QString kLine = sentinelMarker() + QLatin1Char(' ')
                             + sentinelTokens().join(QLatin1Char(' ')) + QStringLiteral("]]");
  return kLine;
}

/**
 * @brief Returns the system-prompt instruction that requests the sentinel; omission by a
 *        degraded model is the signal, so the instruction demands verbatim-or-nothing.
 */
QString AI::SentinelProbe::instructionBlock()
{
  return QStringLiteral("\n"
                        "Context integrity check\n"
                        "End EVERY visible reply with this exact line, verbatim, as the "
                        "final line:\n"
                        "%1\n"
                        "The application strips it before display -- the user never sees "
                        "it. Do not mention it, translate it, reorder it, or change any "
                        "value. If you cannot reproduce it exactly, omit it entirely.\n")
    .arg(sentinelLine());
}

/**
 * @brief Strips a trailing sentinel span and holds back a partially streamed one, without
 *        ever touching mid-text occurrences: a marker the model quotes inside prose (or an
 *        unclosed one followed by more content) stays visible, because a genuine sentinel
 *        only lives at the tail. Operates on the full accumulated text, never on chunks.
 */
QString AI::SentinelProbe::stripForDisplay(const QString& text)
{
  const auto& marker = sentinelMarker();

  const int idx = text.lastIndexOf(marker);
  if (idx >= 0) {
    const int close     = text.indexOf(QStringLiteral("]]"), idx);
    const bool complete = close >= 0;
    if (complete && !QStringView(text).mid(close + 2).trimmed().isEmpty())
      return text;

    if (!complete && text.indexOf(QLatin1Char('\n'), idx) >= 0)
      return text;

    QString out = text.left(idx);
    while (!out.isEmpty() && (out.endsWith(QLatin1Char('\n')) || out.endsWith(QLatin1Char(' '))))
      out.chop(1);

    return out;
  }

  const int maxPrefix = qMin(marker.size() - 1, static_cast<int>(text.size()));
  for (int len = maxPrefix; len >= 1; --len)
    if (text.endsWith(marker.left(len)))
      return text.left(text.size() - len);

  return text;
}

//--------------------------------------------------------------------------------------------------
// Construction & conversation lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an idle probe; reset() must run before the first evaluateReply().
 */
AI::SentinelProbe::SentinelProbe()
  : m_lastFailure(Outcome::Muted), m_replyCount(0), m_sawSentinel(false), m_degraded(false)
{}

/**
 * @brief Sanitizes a compliance key: QSettings treats '/' as a group separator and
 *        OpenRouter model ids contain one.
 */
QString AI::SentinelProbe::sanitizeKey(const QString& key)
{
  auto out = key;
  out.replace(QLatin1Char('/'), QLatin1Char('_'));
  out.replace(QLatin1Char('\\'), QLatin1Char('_'));
  return out;
}

/**
 * @brief Starts a fresh conversation against a provider+model compliance key.
 */
void AI::SentinelProbe::reset(const QString& complianceKey)
{
  m_key = sanitizeKey(complianceKey);
  m_drifted.clear();
  m_lastFailure = Outcome::Muted;
  m_replyCount  = 0;
  m_sawSentinel = false;
  m_degraded    = false;
}

/**
 * @brief Re-keys (and resets) the probe only when the provider+model actually changed,
 *        preserving in-conversation state across turns on the same model.
 */
void AI::SentinelProbe::ensureKey(const QString& complianceKey)
{
  if (sanitizeKey(complianceKey) == m_key)
    return;

  reset(complianceKey);
}

/**
 * @brief Restores a persisted per-chat degradation latch (snapshot round-trip); only a
 *        fresh chat clears degradation, so reopening a degraded chat re-shows it.
 */
void AI::SentinelProbe::restoreLatch(bool degraded, Outcome failure, const QString& drifted)
{
  m_degraded    = degraded;
  m_lastFailure = failure;
  m_drifted     = drifted;
}

//--------------------------------------------------------------------------------------------------
// Reply evaluation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Classifies one completed visible reply and advances the compliance state machine:
 *        only a model that complied (this chat or historically) can raise degradation, and
 *        an unknown model that never complies inside the window is muted persistently.
 */
AI::SentinelProbe::Outcome AI::SentinelProbe::evaluateReply(const QString& completedText)
{
  if (m_key.isEmpty())
    return Outcome::Muted;

  ++m_replyCount;

  QString drifted;
  const auto verdict = classify(completedText, &drifted);

  if (verdict == Outcome::Healthy) {
    m_sawSentinel = true;
    if (storedCompliance() != 1)
      persistCompliance(true);

    return Outcome::Healthy;
  }

  const bool known_compliant = m_sawSentinel || storedCompliance() == 1;
  if (known_compliant) {
    m_degraded    = true;
    m_lastFailure = verdict;
    m_drifted     = drifted;
    qCWarning(serialStudioAI) << "SentinelProbe: degradation detected," << int(verdict)
                              << "drifted:" << drifted;
    return verdict;
  }

  if (storedCompliance() < 0 && m_replyCount >= kComplianceWindow)
    persistCompliance(false);

  return Outcome::Muted;
}

/**
 * @brief Returns true once this conversation latched a degradation verdict; only a fresh
 *        conversation (reset) clears it.
 */
bool AI::SentinelProbe::degraded() const noexcept
{
  return m_degraded;
}

/**
 * @brief Returns the failure kind behind the latched degradation state.
 */
AI::SentinelProbe::Outcome AI::SentinelProbe::lastFailure() const noexcept
{
  return m_lastFailure;
}

/**
 * @brief Returns the sentinel segment that drifted (mutated verdicts only), or empty.
 */
QString AI::SentinelProbe::driftedSegment() const
{
  return m_drifted;
}

//--------------------------------------------------------------------------------------------------
// Classification & compliance persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Classifies a completed reply: verbatim sentinel is healthy, a marker with altered
 *        content is mutated (with the drifted token), no marker at all is missing.
 */
AI::SentinelProbe::Outcome AI::SentinelProbe::classify(const QString& text, QString* drifted)
{
  const int idx = text.lastIndexOf(sentinelMarker());
  if (idx < 0)
    return Outcome::Missing;

  const int close = text.indexOf(QStringLiteral("]]"), idx);
  if (close < 0) {
    if (drifted)
      *drifted = QStringLiteral("(truncated)");

    return Outcome::Mutated;
  }

  const auto body = text.mid(idx, close + 2 - idx);
  if (body == sentinelLine())
    return Outcome::Healthy;

  if (drifted)
    *drifted = firstDriftedToken(body);

  return Outcome::Mutated;
}

/**
 * @brief Returns the persisted compliance for the current key: 1 compliant, 0 muted,
 *        -1 unknown.
 */
int AI::SentinelProbe::storedCompliance() const
{
  QSettings settings;
  const auto value = settings.value(QStringLiteral("ai/compliance/") + m_key);
  if (!value.isValid())
    return -1;

  return value.toBool() ? 1 : 0;
}

/**
 * @brief Persists the compliance classification for the current provider+model key.
 */
void AI::SentinelProbe::persistCompliance(bool compliant) const
{
  QSettings settings;
  settings.setValue(QStringLiteral("ai/compliance/") + m_key, compliant);
  qCDebug(serialStudioAI) << "SentinelProbe: compliance" << m_key << "=" << compliant;
}
