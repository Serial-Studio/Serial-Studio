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

#include "UI/Widgets/Output/Base.h"

#include <utility>

#include "Core/Licensing/CommercialToken.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/Scripting/TransmitScriptCheck.h"
#include "DataModel/Scripting/TransmitScriptEnvironment.h"
#include "DataModel/TextCodec.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an output widget base from a config and the target its payloads go to, which
 *        also selects the host surface. The compile is watchdog-armed because evaluating the
 *        wrapper RUNS the script's top level (spec 0079), and a surface that failed to install is
 *        refused rather than compiled against: its bridges are inert only once masking succeeded.
 */
Widgets::Output::Base::Base(const DataModel::OutputWidget& config,
                            TransmitTarget target,
                            QQuickItem* parent)
  : QQuickItem(parent)
  , m_sourceId(config.sourceId)
  , m_minValue(config.minValue)
  , m_maxValue(config.maxValue)
  , m_stepSize(config.stepSize)
  , m_title(config.title)
  , m_onLabel(config.onLabel)
  , m_offLabel(config.offLabel)
  , m_txEncoding(static_cast<SerialStudio::TextEncoding>(config.txEncoding))
  , m_hasFn(false)
  , m_watchdog(&m_jsEngine, kTransmitWatchdogMs, QStringLiteral("transmit"))
  , m_hasPending(false)
  , m_statePendingShown(false)
  , m_target(std::move(target))
{
  m_rateLimiter.start();
  m_flushTimer.setSingleShot(true);
  connect(&m_flushTimer, &QTimer::timeout, this, &Base::flushPendingValue);

  m_stateBinding.configure(config);
  const bool prepared =
    DataModel::prepareTransmitScriptEngine(m_jsEngine, m_sourceId, m_target.surface);

  if (!prepared)
    qWarning() << "Output widget" << m_title << "- script host surface refused; not compiling";

  if (prepared && !config.transmitFunction.isEmpty()) {
    m_watchdog.arm();
    m_transmitFn = m_jsEngine.evaluate(DataModel::wrapTransmitScript(config.transmitFunction));
    m_watchdog.disarm();
    m_hasFn = m_transmitFn.isCallable() && !m_watchdog.lastCallTimedOut();

    if (!m_hasFn)
      qWarning() << "Output widget" << m_title
                 << "- transmit function is not callable:" << m_transmitFn.toString();
  }
}

/**
 * @brief Destructor.
 */
Widgets::Output::Base::~Base() = default;

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the source/device ID this widget transmits to.
 */
int Widgets::Output::Base::sourceId() const noexcept
{
  return m_sourceId;
}

/**
 * @brief Returns the minimum value for numeric output widgets.
 */
double Widgets::Output::Base::minValue() const noexcept
{
  return m_minValue;
}

/**
 * @brief Returns the maximum value for numeric output widgets.
 */
double Widgets::Output::Base::maxValue() const noexcept
{
  return m_maxValue;
}

/**
 * @brief Returns the step size for numeric output widgets.
 */
double Widgets::Output::Base::stepSize() const noexcept
{
  return m_stepSize;
}

/**
 * @brief Returns the widget title.
 */
const QString& Widgets::Output::Base::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the units string.
 */
const QString& Widgets::Output::Base::units() const noexcept
{
  return m_units;
}

/**
 * @brief Returns the ON state label (for toggle widgets).
 */
const QString& Widgets::Output::Base::onLabel() const noexcept
{
  return m_onLabel;
}

/**
 * @brief Returns the OFF state label (for toggle widgets).
 */
const QString& Widgets::Output::Base::offLabel() const noexcept
{
  return m_offLabel;
}

/**
 * @brief Returns true if a valid transmit function was compiled.
 */
bool Widgets::Output::Base::hasTransmitFunction() const noexcept
{
  return m_hasFn;
}

/**
 * @brief Whether this control reads what it displays from a state source.
 */
bool Widgets::Output::Base::stateBound() const noexcept
{
  return m_stateBinding.bound();
}

/**
 * @brief Whether the source has said anything believable yet. False is NOT off: a control that
 *        has heard nothing must say so rather than show a default (spec 0080 R7).
 */
bool Widgets::Output::Base::stateKnown() const noexcept
{
  return m_stateVerdict.known;
}

/**
 * @brief Whether an operator request is still awaiting confirmation.
 */
bool Widgets::Output::Base::statePending() const
{
  return m_stateBinding.pending(StateBinding::nowMs());
}

/**
 * @brief Whether the source reports this control as on, under its truth rule.
 */
bool Widgets::Output::Base::stateOn() const noexcept
{
  return m_stateVerdict.on;
}

/**
 * @brief Numeric value the source last reported.
 */
double Widgets::Output::Base::stateValue() const noexcept
{
  return m_stateVerdict.value;
}

/**
 * @brief Text the source last reported.
 */
QString Widgets::Output::Base::stateText() const
{
  return m_stateVerdict.text;
}

/**
 * @brief The binding itself, so the panel can read what source to resolve.
 */
Widgets::Output::StateBinding& Widgets::Output::Base::stateBinding() noexcept
{
  return m_stateBinding;
}

//--------------------------------------------------------------------------------------------------
// State feedback
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records what the state source reports. This is the ONLY way a value enters the control
 *        from outside, and it deliberately leads nowhere near m_target: reflecting equipment state
 *        is not an operator action, and a control that transmitted from here would command its own
 *        equipment in a loop (spec 0080 R3).
 */
void Widgets::Output::Base::observeState(const double value,
                                         const QString& text,
                                         const qint64 sampleMs)
{
  m_stateBinding.observe(value, text, sampleMs);
}

/**
 * @brief Records a table-variable reading, whose freshness comes from the store's write clock
 *        rather than from a receipt time the table does not carry.
 */
void Widgets::Output::Base::observeTableState(const double value,
                                              const QString& text,
                                              const quint64 writeClock)
{
  m_stateBinding.observeTable(value, text, writeClock);
}

/**
 * @brief Drops the observed value; the source no longer resolves.
 */
void Widgets::Output::Base::forgetState()
{
  m_stateBinding.forget();
}

/**
 * @brief Whether @p other says anything different from the verdict on display.
 */
bool Widgets::Output::Base::verdictDiffers(const StateBinding::Verdict& other) const noexcept
{
  return other.known != m_stateVerdict.known || other.on != m_stateVerdict.on
      || !qFuzzyCompare(1.0 + other.value, 1.0 + m_stateVerdict.value)
      || other.text != m_stateVerdict.text;
}

/**
 * @brief Recomputes the verdict and lets the control adopt it, unless the operator is holding it
 *        or still waiting on a request. Runs every display tick whether or not a sample arrived,
 *        because the outstanding-request window expires on a clock: equipment that went silent
 *        after a command must stop saying "waiting". The notify is gated on a real change.
 */
void Widgets::Output::Base::refreshState()
{
  if (!m_stateBinding.bound())
    return;

  const auto now      = StateBinding::nowMs();
  const auto verdict  = m_stateBinding.verdict(now);
  const bool pending  = m_stateBinding.pending(now);
  const bool announce = verdictDiffers(verdict) || pending != m_statePendingShown;

  m_stateVerdict      = verdict;
  m_statePendingShown = pending;

  if (announce)
    Q_EMIT stateChanged();

  if (m_stateBinding.holdsDisplay(now))
    return;

  applyStateVerdict(m_stateVerdict);
}

/**
 * @brief Freezes the display while the operator holds the control.
 */
void Widgets::Output::Base::beginInteraction()
{
  m_stateBinding.beginInteraction();
}

/**
 * @brief Releases the control and opens the confirm-within window.
 */
void Widgets::Output::Base::endInteraction()
{
  m_stateBinding.endInteraction(StateBinding::nowMs());
  Q_EMIT stateChanged();
}

/**
 * @brief Opens the confirm-within window after the control commanded something.
 */
void Widgets::Output::Base::noteOperatorRequest()
{
  if (!m_stateBinding.bound())
    return;

  m_stateBinding.noteRequested(StateBinding::nowMs());
  Q_EMIT stateChanged();
}

/**
 * @brief Base controls display nothing of their own; each concrete control overrides this.
 */
void Widgets::Output::Base::applyStateVerdict(const StateBinding::Verdict& verdict)
{
  Q_UNUSED(verdict);
}

//--------------------------------------------------------------------------------------------------
// Transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paces an operator action, leading and trailing. A call inside the pacing window is held
 *        rather than discarded, replacing any older held value, so a drag collapses to the
 *        position it ended at instead of losing it: dropping the last call also skipped the
 *        transmit script, and a script whose effect is a table write loses that effect outright.
 */
void Widgets::Output::Base::sendValue(const QVariant& value)
{
  const auto interval = static_cast<qint64>(m_target.minIntervalMs);
  const auto elapsed  = m_rateLimiter.elapsed();
  if (interval > 0 && elapsed < interval) {
    m_pendingValue = value;
    m_hasPending   = true;
    m_flushTimer.start(static_cast<int>(interval - elapsed));
    return;
  }

  deliverValue(value);
}

/**
 * @brief Sends the value the pacing window held back.
 */
void Widgets::Output::Base::flushPendingValue()
{
  if (!m_hasPending)
    return;

  const QVariant value = m_pendingValue;
  deliverValue(value);
}

/**
 * @brief Evaluates the transmit function and hands the payload to this widget's target. The
 *        license gate stays ahead of the evaluation, because every surface that runs a transmit
 *        script reaches it through here. Takes @p value by value: the caller may be handing over
 *        the held value this clears.
 */
void Widgets::Output::Base::deliverValue(const QVariant& value)
{
  m_flushTimer.stop();
  m_hasPending = false;
  m_pendingValue.clear();
  m_rateLimiter.restart();

  const auto& tk = Licensing::CommercialToken::current();
  if (!tk.isValid() || !SS_LICENSE_GUARD())
    return;

  const auto data = evaluateTransmitFunction(value);
  if (!data.isEmpty() && m_target.deliver)
    m_target.deliver(data);

  noteOperatorRequest();
}

/**
 * @brief Runs the JavaScript transmit(value) function and returns the result as a QByteArray.
 */
QByteArray Widgets::Output::Base::evaluateTransmitFunction(const QVariant& value)
{
  if (!m_hasFn)
    return {};

  auto jsValue      = m_jsEngine.toScriptValue(value);
  QJSValueList args = QJSValueList{jsValue};
  auto result       = m_watchdog.call(m_transmitFn, args);

  if (m_watchdog.lastCallTimedOut()) [[unlikely]] {
    Q_EMIT transmitError(tr("Transmit script timed out after %1 ms").arg(kTransmitWatchdogMs));
    return {};
  }

  if (result.isError()) [[unlikely]] {
    Q_EMIT transmitError(result.toString());
    return {};
  }

  QByteArray data;
  if (result.isString())
    data = SerialStudio::encodeText(result.toString(), m_txEncoding);
  else
    data = result.toVariant().toByteArray();

  if (data.size() > kMaxPayloadBytes) [[unlikely]] {
    Q_EMIT transmitError(tr("Payload exceeds maximum size"));
    return {};
  }

  return data;
}
