/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Output/StateBinding.h"

#include <chrono>
#include <QtGlobal>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Floor for the silence that counts as stale, so a zero confirm-within never means "instantly old"
static constexpr qint64 kMinStaleMs = 2000;

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
 * @brief Opens the confirm-within window after an operator acted.
 */
void Widgets::Output::StateBinding::noteRequested(const qint64 nowMs)
{
  m_requestedMs = nowMs;
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
  noteRequested(nowMs);
}

/**
 * @brief Whether an operator request is still awaiting confirmation.
 */
bool Widgets::Output::StateBinding::pending(const qint64 nowMs) const
{
  if (m_requestedMs < 0)
    return false;

  return (nowMs - m_requestedMs) < static_cast<qint64>(m_confirmMs);
}

/**
 * @brief Whether feedback must leave the displayed state alone right now.
 */
bool Widgets::Output::StateBinding::holdsDisplay(const qint64 nowMs) const
{
  return m_interacting || pending(nowMs);
}

//--------------------------------------------------------------------------------------------------
// Verdict
//--------------------------------------------------------------------------------------------------

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
