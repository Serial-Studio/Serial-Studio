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

#include "Console/AutoReply.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

#include "Core/Checksum.h"
#include "Core/Runtime.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kRulesKey    = QStringLiteral("Console/AutoReplyRules");
static const QString kReplyGapKey = QStringLiteral("Console/AutoReplyGapMs");

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the stored rules and starts disarmed, whatever the last session left behind.
 */
Console::AutoReply::AutoReply(ReplyLink link, QObject* parent)
  : QObject(parent)
  , m_link(std::move(link))
  , m_gapTimer(this)
  , m_replyCount(0)
  , m_gapMs(kReplyGapDefaultMs)
  , m_armed(false)
  , m_anyText(false)
  , m_anyHex(false)
{
  SS_ASSERT_LOG(m_link.connected && m_link.send && m_link.note);

  m_gapMs = qBound(
    kReplyGapMinMs, m_settings.value(kReplyGapKey, kReplyGapDefaultMs).toInt(), kReplyGapMaxMs);

  m_clock.start();
  m_gapTimer.setSingleShot(true);
  m_gapTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_gapTimer, &QTimer::timeout, this, &AutoReply::onGapTimeout);

  loadRules();
  recompile();
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether received messages are being answered.
 */
bool Console::AutoReply::armed() const noexcept
{
  return m_armed;
}

/**
 * @brief Idle time that ends a message for hexadecimal rules, in milliseconds.
 */
int Console::AutoReply::gapMs() const noexcept
{
  return m_gapMs;
}

/**
 * @brief Most rules the table holds.
 */
int Console::AutoReply::maxRules() const noexcept
{
  return kMaxReplyRules;
}

/**
 * @brief False in a sealed deployment session, where the panel is hidden.
 */
bool Console::AutoReply::available() const noexcept
{
  return !Core::Runtime::sealed();
}

/**
 * @brief Replies sent since the responder was last armed.
 */
quint64 Console::AutoReply::replyCount() const noexcept
{
  return m_replyCount;
}

/**
 * @brief The stored names of the checksums a reply can end with, in the order the console's
 *        translated checksum list shows them; the first name is empty and means none.
 */
QStringList Console::AutoReply::checksumNames() const
{
  return IO::availableChecksums();
}

/**
 * @brief The rule table, each entry a map of match, mode, matchHex, enabled and reply.
 */
const QVariantList& Console::AutoReply::rules() const noexcept
{
  return m_rules;
}

/**
 * @brief Why the editor's draft @a rule cannot be saved; empty when it can.
 */
QString Console::AutoReply::ruleProblem(const QVariantMap& rule) const
{
  return replyRuleProblem(replyRuleFromMap(rule));
}

//--------------------------------------------------------------------------------------------------
// Arming
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts answering. Refused in a sealed session, with the link closed, or when no enabled
 *        rule is valid; anything a device sent before this moment is not judged.
 */
void Console::AutoReply::arm()
{
  SS_ASSERT(m_link.connected, return);
  if (m_armed || Core::Runtime::sealed() || !m_link.connected() || m_compiled.empty())
    return;

  m_devices.clear();
  m_limiter.reset();
  m_replyCount = 0;
  m_armed      = true;
  Q_EMIT replyCountChanged();
  Q_EMIT armedChanged();
}

/**
 * @brief Stops answering and drops every partial message.
 */
void Console::AutoReply::disarm()
{
  if (!m_armed)
    return;

  m_armed = false;
  m_gapTimer.stop();
  m_devices.clear();
  Q_EMIT armedChanged();
}

/**
 * @brief Disarms when the link closes, so a later connection never starts out answering.
 */
void Console::AutoReply::onLinkChanged()
{
  SS_ASSERT(m_link.connected, return);
  if (!m_link.connected())
    disarm();
}

//--------------------------------------------------------------------------------------------------
// Rule table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a valid rule; an invalid one, or one past the cap, is ignored.
 */
void Console::AutoReply::addRule(const QVariantMap& rule)
{
  const auto parsed = replyRuleFromMap(rule);
  if (m_rules.count() >= kMaxReplyRules || !replyRuleProblem(parsed).isEmpty())
    return;

  m_rules.append(replyRuleToMap(parsed));
  saveRules();
}

/**
 * @brief Replaces the rule at @a index with a valid one.
 */
void Console::AutoReply::updateRule(int index, const QVariantMap& rule)
{
  SS_ASSERT(index >= 0 && index < m_rules.count(), return);

  const auto parsed = replyRuleFromMap(rule);
  if (!replyRuleProblem(parsed).isEmpty())
    return;

  m_rules.replace(index, replyRuleToMap(parsed));
  saveRules();
}

/**
 * @brief Removes the rule at @a index.
 */
void Console::AutoReply::removeRule(int index)
{
  SS_ASSERT(index >= 0 && index < m_rules.count(), return);

  m_rules.removeAt(index);
  saveRules();
}

/**
 * @brief Moves the rule at @a from to position @a to; rules reply in table order.
 */
void Console::AutoReply::moveRule(int from, int to)
{
  SS_ASSERT(from >= 0 && from < m_rules.count(), return);
  SS_ASSERT(to >= 0 && to < m_rules.count(), return);
  if (from == to)
    return;

  m_rules.move(from, to);
  saveRules();
}

/**
 * @brief Includes the rule at @a index in matching, or leaves it out.
 */
void Console::AutoReply::setRuleEnabled(int index, bool enabled)
{
  SS_ASSERT(index >= 0 && index < m_rules.count(), return);

  auto rule    = replyRuleFromMap(m_rules.at(index).toMap());
  rule.enabled = enabled;
  m_rules.replace(index, replyRuleToMap(rule));
  saveRules();
}

/**
 * @brief Sets and stores the idle gap that ends a hexadecimal message.
 */
void Console::AutoReply::setGapMs(int gapMs)
{
  const int clamped = qBound(kReplyGapMinMs, gapMs, kReplyGapMaxMs);
  if (m_gapMs == clamped)
    return;

  m_gapMs = clamped;
  m_settings.setValue(kReplyGapKey, m_gapMs);
  Q_EMIT gapChanged();
}

/**
 * @brief Loads the rules from app settings. Settings are user-editable, so a corrupt blob yields
 *        an empty table and a malformed or invalid entry is skipped; the loop is bounded.
 */
void Console::AutoReply::loadRules()
{
  const auto doc = QJsonDocument::fromJson(m_settings.value(kRulesKey).toByteArray());
  if (!doc.isArray())
    return;

  const auto array = doc.array();
  for (const auto& entry : array) {
    if (m_rules.count() >= kMaxReplyRules)
      break;

    if (!entry.isObject())
      continue;

    const auto rule = replyRuleFromMap(entry.toObject().toVariantMap());
    if (replyRuleProblem(rule).isEmpty())
      m_rules.append(replyRuleToMap(rule));
  }
}

/**
 * @brief Stores the table, rebuilds the matchers and notifies the editor; edit rate only. With
 *        no enabled rule left there is nothing to answer with, so the responder disarms.
 */
void Console::AutoReply::saveRules()
{
  const auto doc = QJsonDocument(QJsonArray::fromVariantList(m_rules));
  m_settings.setValue(kRulesKey, doc.toJson(QJsonDocument::Compact));
  recompile();
  if (m_compiled.empty())
    disarm();

  Q_EMIT rulesChanged();
}

/**
 * @brief Compiles the enabled rules once, so a received message is only compared.
 */
void Console::AutoReply::recompile()
{
  m_compiled.clear();
  m_anyText = false;
  m_anyHex  = false;

  for (const auto& entry : std::as_const(m_rules)) {
    const auto rule = replyRuleFromMap(entry.toMap());
    CompiledReplyRule compiled;
    if (!rule.enabled || !compileReplyRule(rule, compiled))
      continue;

    m_anyText |= !compiled.hex;
    m_anyHex  |= compiled.hex;
    m_compiled.push_back(std::move(compiled));
  }

  SS_ASSERT_LOG(m_compiled.size() <= static_cast<size_t>(kMaxReplyRules));
}

//--------------------------------------------------------------------------------------------------
// Receive path
//--------------------------------------------------------------------------------------------------

/**
 * @brief Takes one received chunk of @a deviceId. Text rules see it line by line; hexadecimal
 *        rules see what arrived until the line went idle for the gap. Disarmed, this returns at
 *        once, which is all the receive path pays for the feature.
 */
void Console::AutoReply::feed(int deviceId, const QByteArray& bytes)
{
  if (!m_armed || bytes.isEmpty())
    return;

  auto& state = m_devices[deviceId];
  if (m_anyText)
    feedLines(deviceId, state, bytes);

  if (!m_anyHex || !m_armed)
    return;

  state.lastRxMs = m_clock.elapsed();
  if (!m_gapTimer.isActive())
    m_gapTimer.start(m_gapMs);

  if (state.skipping)
    return;

  const qsizetype room = kMaxMessageBytes - state.packet.size();
  state.packet.append(bytes.constData(), qMin(room, bytes.size()));
  if (state.packet.size() < kMaxMessageBytes)
    return;

  state.skipping    = true;
  const auto packet = std::exchange(state.packet, QByteArray());
  dispatch(deviceId, packet, true);
}

/**
 * @brief Splits @a bytes into lines at LF, dropping a CR before it, and judges each complete
 *        line. A line that reaches the message cap without an LF is judged as it stands.
 */
void Console::AutoReply::feedLines(int deviceId, DeviceState& state, const QByteArray& bytes)
{
  SS_ASSERT(state.line.size() <= kMaxMessageBytes, state.line.clear());

  qsizetype start = 0;
  while (start < bytes.size() && m_armed) {
    const qsizetype lf   = bytes.indexOf('\n', start);
    const qsizetype end  = lf < 0 ? bytes.size() : lf;
    const qsizetype room = kMaxMessageBytes - state.line.size();
    state.line.append(bytes.constData() + start, qMin(room, end - start));
    start = end + 1;

    if (lf < 0 && state.line.size() < kMaxMessageBytes)
      break;

    if (state.line.endsWith('\r'))
      state.line.chop(1);

    const auto line = std::exchange(state.line, QByteArray());
    dispatch(deviceId, line, false);
  }
}

/**
 * @brief Judges the hexadecimal message of every device that has been idle for the gap, and
 *        waits out the remainder for the others.
 */
void Console::AutoReply::onGapTimeout()
{
  if (!m_armed)
    return;

  const qint64 now = m_clock.elapsed();
  qint64 wait      = -1;
  QList<int> ready;
  for (auto& [device, state] : m_devices) {
    if (state.packet.isEmpty() && !state.skipping)
      continue;

    const qint64 left = m_gapMs - (now - state.lastRxMs);
    if (left > 0) {
      wait = wait < 0 ? left : qMin(wait, left);
      continue;
    }

    state.skipping = false;
    if (!state.packet.isEmpty())
      ready.append(device);
  }

  for (const int device : std::as_const(ready)) {
    if (!m_armed)
      return;

    const auto packet = std::exchange(m_devices[device].packet, QByteArray());
    dispatch(device, packet, true);
  }

  if (wait > 0 && m_armed)
    m_gapTimer.start(static_cast<int>(wait));
}

/**
 * @brief Sends the reply of every enabled rule of the right kind that matches @a message, to the
 *        device the message came from. Past the rate limit the responder disarms and says so
 *        once in the console instead of feeding an echo loop.
 */
void Console::AutoReply::dispatch(int deviceId, QByteArrayView message, bool hex)
{
  SS_ASSERT(m_link.send && m_link.note, return);
  if (message.isEmpty())
    return;

  for (const auto& rule : m_compiled) {
    if (rule.hex != hex || !replyRuleMatches(rule, message))
      continue;

    if (!m_limiter.allow(m_clock.elapsed())) {
      disarm();
      m_link.note(
        tr("Auto reply stopped: more than %1 replies in one second.").arg(kMaxRepliesPerSecond));
      return;
    }

    m_link.send(deviceId, rule.replyBytes);
    ++m_replyCount;
    Q_EMIT replyCountChanged();
    if (!m_armed)
      return;
  }
}
