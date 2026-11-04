/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Output/StateBinding.h"

#include <chrono>
#include <QHash>
#include <QtGlobal>
#include <QThread>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Floor for the silence that counts as stale, so a zero confirm-within never means "instantly old"
static constexpr qint64 kMinStaleMs = 2000;

// Floor for a reply window, so a zero confirm-within never declares a command unanswered at once
static constexpr qint64 kMinReplyWindowMs = 100;

// Tokens run 1..32767 and wrap to 1; 0 is never issued, so a device's power-on zero matches nothing
static constexpr int kMaxReplyToken = 32767;

// Token counters per reply-token dataset (spec 0110 A1); GUI thread only, -1 = no reply bound
static QHash<int, int> s_replyTokens;

//--------------------------------------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts a widget's binding fields. Reconfiguring forgets the observed value: the new
 *        source has said nothing yet, and carrying the old reading over would attribute one
 *        source's state to another.
 */
void Widgets::Output::StateBinding::configure(const DataModel::OutputWidget& config)
{
  m_source    = config.stateSource;
  m_datasetId = config.stateDatasetId;
  m_table     = config.stateTable;
  m_variable  = config.stateVariable;
  m_onValue   = config.stateOnValue;
  m_confirmMs = qBound(0, config.stateConfirmMs, DataModel::kMaxOutputStateConfirmMs);

  m_twoState    = config.type == DataModel::OutputWidgetType::Toggle
               || (config.type == DataModel::OutputWidgetType::Button && config.checkable);
  m_textRequest = config.type == DataModel::OutputWidgetType::TextField
               && config.inputKind == DataModel::OutputInputKind::Text;

  const bool reply = DataModel::outputExpectsReply(config);
  m_replyTokenId   = reply ? config.replyTokenDatasetId : -1;
  m_replyCodeId    = reply ? config.replyCodeDatasetId : -1;
  m_replyMessageId = reply ? config.replyMessageDatasetId : -1;
  m_outcome        = Outcome::None;
  m_expectedToken  = 0;
  m_seenToken      = 0;
  m_replyCode      = 0;
  m_hasRequest     = false;
  m_replyMessage.clear();

  forget();
}

/**
 * @brief Whether this control reads its displayed state from somewhere.
 */
bool Widgets::Output::StateBinding::bound() const noexcept
{
  return m_source != DataModel::OutputStateSource::None;
}

/**
 * @brief Whether the control waits for a device reply: token and code datasets both bound.
 */
bool Widgets::Output::StateBinding::expectsReply() const noexcept
{
  return m_replyTokenId >= 0 && m_replyCodeId >= 0;
}

/**
 * @brief Unique id of the dataset echoing the command token, or -1.
 */
int Widgets::Output::StateBinding::replyTokenId() const noexcept
{
  return m_replyTokenId;
}

/**
 * @brief Unique id of the dataset carrying the result code, or -1.
 */
int Widgets::Output::StateBinding::replyCodeId() const noexcept
{
  return m_replyCodeId;
}

/**
 * @brief Unique id of the dataset carrying the reply text, or -1.
 */
int Widgets::Output::StateBinding::replyMessageId() const noexcept
{
  return m_replyMessageId;
}

/**
 * @brief Whether the source is a data-table variable rather than a dataset.
 */
bool Widgets::Output::StateBinding::readsTable() const noexcept
{
  return m_source == DataModel::OutputStateSource::Table;
}

/**
 * @brief Unique id of the dataset driving the display, or -1.
 */
int Widgets::Output::StateBinding::datasetId() const noexcept
{
  return m_datasetId;
}

/**
 * @brief Table holding the state variable.
 */
const QString& Widgets::Output::StateBinding::table() const noexcept
{
  return m_table;
}

/**
 * @brief Variable within the bound table.
 */
const QString& Widgets::Output::StateBinding::variable() const noexcept
{
  return m_variable;
}

//--------------------------------------------------------------------------------------------------
// Observation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The clock the whole feedback path measures in: the steady domain Dataset::displaySampleMs
 *        is stamped in. Every "now" that meets a sample time comes from here, because the two are
 *        subtracted from each other -- a wall-clock now against a monotonic stamp is a difference
 *        of decades, which reads as permanently stale.
 */
qint64 Widgets::Output::StateBinding::nowMs() noexcept
{
  return static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now().time_since_epoch())
                               .count());
}

/**
 * @brief Records what the source reports, stamped with the sample's own receipt time so silence
 *        is measured from when the value arrived, not from when it was last looked at. @p sampleMs
 *        is in the nowMs() domain.
 */
void Widgets::Output::StateBinding::observe(const double value,
                                            const QString& text,
                                            const qint64 sampleMs)
{
  SS_ASSERT_LOG(sampleMs >= 0);

  m_hasValue = true;
  m_value    = value;
  m_text     = text;
  m_sampleMs = sampleMs;
}

/**
 * @brief Records a table variable, which carries no receipt time. Freshness comes from the
 *        store's write clock: a variable whose writer stopped stops being believed, where
 *        stamping "now" per read made it permanently live. The clock is store-wide, so a write
 *        to another table also counts; the per-slot version is not on DataTableSnapshot.
 */
void Widgets::Output::StateBinding::observeTable(const double value,
                                                 const QString& text,
                                                 const quint64 writeClock)
{
  const bool moved = !m_hasValue || writeClock != m_lastWriteClock
                  || !qFuzzyCompare(1.0 + value, 1.0 + m_value) || text != m_text;

  m_lastWriteClock = writeClock;
  if (moved)
    observe(value, text, nowMs());
}

/**
 * @brief Drops the observed value: the source is gone, or it was just repointed.
 */
void Widgets::Output::StateBinding::forget()
{
  m_hasValue       = false;
  m_value          = 0;
  m_sampleMs       = 0;
  m_requestedMs    = -1;
  m_lastWriteClock = 0;
  m_text.clear();
}

//--------------------------------------------------------------------------------------------------
// Outstanding requests and interaction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the confirm-within window after an operator acted and records what was asked, so a
 *        state report that matches it ends the wait early (spec 0110 R11). A new request also
 *        clears the previous reply outcome: a refusal stays shown until the next send.
 */
void Widgets::Output::StateBinding::noteRequested(const qint64 nowMs, const QVariant& request)
{
  m_requestedMs  = nowMs;
  m_outcome      = Outcome::None;
  m_hasRequest   = request.isValid();
  m_requestText  = request.toString();
  m_requestValue = m_hasRequest ? SerialStudio::toDouble(request) : 0;
}

/**
 * @brief Freezes the display for the duration of an interaction, so feedback never moves a
 *        control under the operator's finger.
 */
void Widgets::Output::StateBinding::beginInteraction()
{
  m_interacting = true;
}

/**
 * @brief Ends the interaction and opens the confirm-within window from the release, so a
 *        released control is not snapped back before the equipment can answer.
 */
void Widgets::Output::StateBinding::endInteraction(const qint64 nowMs)
{
  m_interacting = false;
  m_requestedMs = nowMs;
}

/**
 * @brief Whether an operator request is still awaiting confirmation: a reply inside its window, or
 *        a state-only request inside its window that the source has not yet confirmed. A settled
 *        or unanswered reply is never pending.
 */
bool Widgets::Output::StateBinding::pending(const qint64 nowMs) const
{
  if (m_requestedMs < 0)
    return false;

  const bool open = (nowMs - m_requestedMs) < windowMs();
  if (m_outcome == Outcome::Waiting)
    return open;

  if (m_outcome != Outcome::None)
    return false;

  return open && !stateConfirms(nowMs);
}

/**
 * @brief Whether feedback must leave the displayed state alone right now: while the operator holds
 *        the control, while a request is pending, and after an acknowledgement until the state
 *        confirms it or the window ends. A refused or unanswered command lets the state win at
 * once.
 */
bool Widgets::Output::StateBinding::holdsDisplay(const qint64 nowMs) const
{
  if (m_interacting)
    return true;

  if (m_outcome == Outcome::Acknowledged)
    return m_requestedMs >= 0 && (nowMs - m_requestedMs) < windowMs() && !stateConfirms(nowMs);

  if (m_outcome == Outcome::Refused || m_outcome == Outcome::Unanswered)
    return false;

  return pending(nowMs);
}

/**
 * @brief Latches what the clock and the source have decided: a reply that ran out of window is
 *        unanswered, and a confirmed request closes its window so the control follows its source
 *        again. A window still waiting on a reply is never closed by the state.
 */
void Widgets::Output::StateBinding::settle(const qint64 nowMs)
{
  if (m_outcome == Outcome::Waiting && (nowMs - m_requestedMs) >= windowMs())
    m_outcome = Outcome::Unanswered;

  if (m_outcome != Outcome::Waiting && m_requestedMs >= 0 && stateConfirms(nowMs))
    m_requestedMs = -1;
}

/**
 * @brief Whether the state verdict matches the recorded request: the truth rule for a two-state
 *        control, the text for a Text field, a fuzzy compare for a value.
 */
bool Widgets::Output::StateBinding::stateConfirms(const qint64 nowMs) const
{
  if (!m_hasRequest || !bound())
    return false;

  const auto current = verdict(nowMs);
  if (!current.known)
    return false;

  if (m_twoState)
    return current.on == !qFuzzyIsNull(m_requestValue);

  if (m_textRequest)
    return current.text.trimmed() == m_requestText.trimmed();

  return qFuzzyCompare(1.0 + current.value, 1.0 + m_requestValue);
}

//--------------------------------------------------------------------------------------------------
// Replies (spec 0110)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forgets every token counter; called when a link connects, so tokens restart at 1.
 */
void Widgets::Output::StateBinding::resetTokens()
{
  SS_ASSERT_LOG(QThread::isMainThread());
  s_replyTokens.clear();
}

/**
 * @brief The next token on this control's reply line: 1..32767, wrapping to 1, never 0, and never
 *        the token the line showed last, so a device that kept running across a reconnect can not
 *        acknowledge a fresh command with its stale reply.
 */
int Widgets::Output::StateBinding::issueToken()
{
  SS_ASSERT_LOG(QThread::isMainThread());

  auto it = s_replyTokens.find(m_replyTokenId);
  if (it == s_replyTokens.end())
    it = s_replyTokens.insert(m_replyTokenId, 0);

  int next = it.value();
  for (int attempt = 0; attempt < 2; ++attempt) {
    next = next >= kMaxReplyToken ? 1 : next + 1;
    if (next != m_seenToken)
      break;
  }

  it.value() = next;
  return next;
}

/**
 * @brief Records that bytes carrying @p token went out: the outcome is waiting from @p nowMs.
 */
void Widgets::Output::StateBinding::noteSent(const qint64 nowMs, const int token)
{
  SS_ASSERT(expectsReply(), return);
  SS_ASSERT_LOG(token > 0);

  m_requestedMs   = nowMs;
  m_expectedToken = token;
  m_outcome       = Outcome::Waiting;
  m_replyCode     = 0;
  m_replyMessage.clear();
}

/**
 * @brief Reads the reply line: remembers the token it shows, and settles a waiting command whose
 *        token it echoes inside the window (0 acknowledged, anything else refused). An older token,
 *        or one arriving after the window, changes nothing but the remembered token.
 */
void Widgets::Output::StateBinding::observeReply(const qint64 nowMs,
                                                 const int token,
                                                 const int code,
                                                 const QString& message)
{
  SS_ASSERT_LOG(expectsReply());

  m_seenToken = token;
  if (m_outcome != Outcome::Waiting || token != m_expectedToken)
    return;

  if ((nowMs - m_requestedMs) >= windowMs()) {
    m_outcome = Outcome::Unanswered;
    return;
  }

  m_replyCode    = code;
  m_replyMessage = message;
  m_outcome      = code == 0 ? Outcome::Acknowledged : Outcome::Refused;
}

/**
 * @brief The outcome as of @p nowMs: a waiting command past its window reads unanswered even
 *        before settle() latches it.
 */
Widgets::Output::StateBinding::Outcome Widgets::Output::StateBinding::outcome(
  const qint64 nowMs) const
{
  if (m_outcome == Outcome::Waiting && (nowMs - m_requestedMs) >= windowMs())
    return Outcome::Unanswered;

  return m_outcome;
}

/**
 * @brief The token the control is waiting for (or last waited for), 0 before any send.
 */
int Widgets::Output::StateBinding::expectedToken() const noexcept
{
  return m_expectedToken;
}

/**
 * @brief The result code of the settled reply.
 */
int Widgets::Output::StateBinding::replyCode() const noexcept
{
  return m_replyCode;
}

/**
 * @brief The message of the settled reply, empty when the device sent none.
 */
const QString& Widgets::Output::StateBinding::replyMessage() const noexcept
{
  return m_replyMessage;
}

//--------------------------------------------------------------------------------------------------
// Verdict
//--------------------------------------------------------------------------------------------------

/**
 * @brief The one window that governs both state and reply (spec 0110 R5): confirm-within, floored
 *        at 100 ms for a reply so a zero setting never reads as unanswered at once.
 */
qint64 Widgets::Output::StateBinding::windowMs() const noexcept
{
  const auto window = static_cast<qint64>(m_confirmMs);
  return expectsReply() ? qMax(kMinReplyWindowMs, window) : window;
}

/**
 * @brief Silence that counts as stale: twice the equipment's own confirm-within, floored. Deriving
 *        it avoids a second timing field, and a control whose equipment answers in 3 s is fairly
 *        called unknown after 6 s of nothing.
 */
qint64 Widgets::Output::StateBinding::staleWindowMs() const noexcept
{
  return qMax(kMinStaleMs, static_cast<qint64>(m_confirmMs) * 2);
}

/**
 * @brief What the control should display. Unknown is a distinct outcome from off: nothing has
 *        arrived, or nothing has arrived recently enough to still be believed.
 */
Widgets::Output::StateBinding::Verdict Widgets::Output::StateBinding::verdict(
  const qint64 nowMs) const
{
  Verdict out;
  if (!bound() || !m_hasValue)
    return out;

  if ((nowMs - m_sampleMs) > staleWindowMs())
    return out;

  out.known = true;
  out.value = m_value;
  out.text  = m_text;
  out.on    = truth(m_onValue, m_value, m_text);
  return out;
}

/**
 * @brief The two-state truth rule. An empty on-value means "any non-zero number", which a text
 *        source can never satisfy -- a device reporting RUN/STOP therefore needs an explicit
 *        on-value, and saying so is exactly why the field exists. A filled on-value matches
 *        numerically when it parses as a number and case-insensitively as text otherwise.
 */
bool Widgets::Output::StateBinding::truth(const QString& onValue,
                                          const double value,
                                          const QString& text)
{
  const auto expected = onValue.trimmed();
  if (expected.isEmpty())
    return !qFuzzyIsNull(value);

  bool numeric        = false;
  const double wanted = SerialStudio::toDouble(expected, &numeric);
  if (numeric)
    return qFuzzyCompare(1.0 + value, 1.0 + wanted);

  return QString::compare(text.trimmed(), expected, Qt::CaseInsensitive) == 0;
}
