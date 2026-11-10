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

#include "Core/License.h"
#include "Core/Licensing/CommercialToken.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/Scripting/TransmitScriptCheck.h"
#include "DataModel/Scripting/TransmitScriptEnvironment.h"
#include "DataModel/TextCodec.h"
#include "Misc/IconEngine.h"

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
  , m_units(config.units)
  , m_onLabel(config.onLabel)
  , m_offLabel(config.offLabel)
  , m_txEncoding(static_cast<SerialStudio::TextEncoding>(config.txEncoding))
  , m_numeric(DataModel::outputIsNumeric(config))
  , m_quantize(config.quantize)
  , m_decimals(config.decimals)
  , m_admittedValue(config.initialValue)
  , m_hasFn(false)
  , m_watchdog(&m_jsEngine, kTransmitWatchdogMs, QStringLiteral("transmit"))
  , m_hasPending(false)
  , m_statePendingShown(false)
  , m_shownOutcome(StateBinding::Outcome::None)
  , m_target(std::move(target))
{
  m_rateLimiter.start();
  m_flushTimer.setSingleShot(true);
  connect(&m_flushTimer, &QTimer::timeout, this, &Base::flushPendingValue);

  m_windowTimer.setSingleShot(true);
  connect(&m_windowTimer, &QTimer::timeout, this, &Base::refreshState);

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
 * @brief Destructor: a live transmit surface armed per-dataset capture when its engine was
 *        prepared (spec 0086), so it is released here with the engine. The source-id guard mirrors
 *        prepareTransmitScriptEngine's precondition: a refused surface never armed.
 */
Widgets::Output::Base::~Base()
{
  if (m_target.surface == DataModel::TransmitScriptSurface::Live && m_sourceId >= 0)
    DataModel::pipelineModules().frameBuilder.releaseTableApiUser();
}

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
 * @brief Whether the control carries a number bounded by Min to Max (spec 0109).
 */
bool Widgets::Output::Base::numeric() const noexcept
{
  return m_numeric;
}

/**
 * @brief Displayed decimals; -1 means automatic from Step.
 */
int Widgets::Output::Base::decimals() const noexcept
{
  return m_decimals;
}

/**
 * @brief The refusal shown beside the control, empty once a value is accepted.
 */
const QString& Widgets::Output::Base::rangeMessage() const noexcept
{
  return m_rangeMessage;
}

/**
 * @brief The last value admitted, after Quantize, so a field shows what was actually sent.
 */
double Widgets::Output::Base::admittedValue() const noexcept
{
  return m_admittedValue;
}

/**
 * @brief @p from moved by @p steps Steps, kept inside Min to Max (arrow keys, wheel; spec 0109 R4).
 */
double Widgets::Output::Base::stepped(double from, int steps) const
{
  SS_ASSERT_LOG(m_minValue <= m_maxValue);
  return qBound(m_minValue, from + steps * m_stepSize, m_maxValue);
}

/**
 * @brief @p value with the control's decimals, or automatic ones (two below a Step of 1).
 */
QString Widgets::Output::Base::formatted(double value) const
{
  const int places = m_decimals >= 0 ? m_decimals : (m_stepSize < 1 ? 2 : 0);
  return QString::number(value, 'f', places);
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
  const auto now = StateBinding::nowMs();
  m_stateBinding.settle(now);
  if (m_stateBinding.expectsReply())
    refreshOutcome(now);

  if (!m_stateBinding.bound())
    return;

  auto verdict = m_stateBinding.verdict(now);
  adjustVerdict(verdict);
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
  armWindowTimer();
  Q_EMIT stateChanged();
}

/**
 * @brief Records what the control commanded and opens the window (spec 0110): the request for every
 *        state-bound control, so a matching report ends the wait early (R11), and the token for a
 *        reply-bound one that actually sent bytes. The timer ends the window even from a silent
 *        device (R10).
 */
void Widgets::Output::Base::noteOperatorRequest(const QVariant& value,
                                                const int token,
                                                const bool bytesSent)
{
  const bool reply = m_stateBinding.expectsReply();
  if (!m_stateBinding.bound() && !reply)
    return;

  const auto now = StateBinding::nowMs();
  m_stateBinding.noteRequested(now, value);
  if (reply && bytesSent)
    m_stateBinding.noteSent(now, token);

  armWindowTimer();
  Q_EMIT stateChanged();
}

/**
 * @brief Starts the single-shot that re-evaluates the control when its window is up, 1 ms past it.
 */
void Widgets::Output::Base::armWindowTimer()
{
  SS_ASSERT_LOG(m_stateBinding.windowMs() >= 0);
  m_windowTimer.start(static_cast<int>(m_stateBinding.windowMs()) + 1);
}

/**
 * @brief Announces an outcome change, and reports a refusal once where transmit errors already
 *        surface (spec 0110 R6).
 */
void Widgets::Output::Base::refreshOutcome(const qint64 nowMs)
{
  SS_ASSERT_LOG(m_stateBinding.expectsReply());

  const auto current = m_stateBinding.outcome(nowMs);
  if (current == m_shownOutcome)
    return;

  m_shownOutcome = current;
  if (current == StateBinding::Outcome::Refused)
    Q_EMIT transmitError(tr("Refused: %1").arg(outcomeText()));

  Q_EMIT stateChanged();
}

/**
 * @brief Hands the reply datasets' latest reading to the binding; the panel calls refreshState
 *        right after, on the same display tick. Never transmits (spec 0080 R3).
 */
void Widgets::Output::Base::observeReply(const int token, const int code, const QString& message)
{
  m_stateBinding.observeReply(StateBinding::nowMs(), token, code, message);
}

/**
 * @brief Whether the control waits for a device reply.
 */
bool Widgets::Output::Base::expectsReply() const noexcept
{
  return m_stateBinding.expectsReply();
}

/**
 * @brief Where the last command stands, as of now.
 */
SerialStudio::OutputOutcome Widgets::Output::Base::outcome() const
{
  return m_stateBinding.outcome(StateBinding::nowMs());
}

/**
 * @brief The outcome slug ui.automation.listElements reports: wire format, kept as text.
 */
[[nodiscard]] static QString outcomeSlug(const SerialStudio::OutputOutcome outcome)
{
  switch (outcome) {
    case SerialStudio::OutputOutcome::None:
      return QStringLiteral("none");
    case SerialStudio::OutputOutcome::Waiting:
      return QStringLiteral("waiting");
    case SerialStudio::OutputOutcome::Acknowledged:
      return QStringLiteral("acknowledged");
    case SerialStudio::OutputOutcome::Refused:
      return QStringLiteral("refused");
    case SerialStudio::OutputOutcome::Unanswered:
      return QStringLiteral("unanswered");
  }

  return QStringLiteral("none");
}

/**
 * @brief A refusal's message, or its code when the device sent none; empty otherwise.
 */
QString Widgets::Output::Base::outcomeText() const
{
  if (m_stateBinding.outcome(StateBinding::nowMs()) != StateBinding::Outcome::Refused)
    return QString();

  const auto& message = m_stateBinding.replyMessage();
  return message.isEmpty() ? QString::number(m_stateBinding.replyCode()) : message;
}

/**
 * @brief The machine-readable state the element catalog returns for this control (spec 0110 R7).
 */
QVariantMap Widgets::Output::Base::automationState() const
{
  const auto now = StateBinding::nowMs();
  QVariantMap map;
  map[QStringLiteral("outcome")]    = outcomeSlug(outcome());
  map[QStringLiteral("token")]      = m_stateBinding.expectedToken();
  map[QStringLiteral("code")]       = m_stateBinding.replyCode();
  map[QStringLiteral("message")]    = m_stateBinding.replyMessage();
  map[QStringLiteral("waiting")]    = m_stateBinding.pending(now);
  map[QStringLiteral("known")]      = m_stateVerdict.known;
  map[QStringLiteral("holding")]    = m_stateBinding.holdsDisplay(now);
  map[QStringLiteral("outOfRange")] = !m_rangeMessage.isEmpty();
  return map;
}

/**
 * @brief Base controls display nothing of their own; each concrete control overrides this.
 */
void Widgets::Output::Base::applyStateVerdict(const StateBinding::Verdict& verdict)
{
  Q_UNUSED(verdict);
}

/**
 * @brief Lets a control narrow what counts as a believable report before it is shown; the base
 *        accepts every verdict the binding produced.
 */
void Widgets::Output::Base::adjustVerdict(StateBinding::Verdict& verdict) const
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
  submitValue(value, true);
}

/**
 * @brief Sends @p value at once, past the pacing window, dropping any value the window was
 *        holding. The one caller is an emergency-stop press (spec 0113 R26): a stop held back
 *        behind the release that preceded it is a stop that arrives late. Every gate
 *        sendValue() applies still applies.
 */
void Widgets::Output::Base::sendValueNow(const QVariant& value)
{
  submitValue(value, false);
}

/**
 * @brief The one path an operator action takes to the transmit script: license gates, admission,
 *        then pacing when @p paced, then delivery.
 */
void Widgets::Output::Base::submitValue(const QVariant& value, const bool paced)
{
  if (!SS_LICENSE_GUARD())
    return;

  if (!Licensing::CommercialToken::current().isValid()) {
    Core::License::requestProFeature(QStringLiteral("output.send"));
    return;
  }

  QVariant admitted = value;
  if (!admit(admitted))
    return;

  const auto interval = static_cast<qint64>(m_target.minIntervalMs);
  const auto elapsed  = m_rateLimiter.elapsed();
  if (paced && interval > 0 && elapsed < interval) {
    m_pendingValue = admitted;
    m_hasPending   = true;
    m_flushTimer.start(static_cast<int>(interval - elapsed));
    return;
  }

  deliverValue(admitted);
}

/**
 * @brief The admission step every value takes after the license gates and before pacing (spec
 *        0109 R3): a numeric control's value is replaced by its admitted double, or refused with
 *        the message beside the control, in which case nothing is held, run or written.
 */
bool Widgets::Output::Base::admit(QVariant& value)
{
  if (!m_numeric)
    return true;

  double admitted = 0;
  const DataModel::OutputLimits limits{
    .minValue = m_minValue, .maxValue = m_maxValue, .stepSize = m_stepSize, .quantize = m_quantize};
  const auto verdict = DataModel::admitOutputValue(limits, value, admitted);
  if (verdict != DataModel::OutputAdmission::Accepted) {
    const auto suffix = m_units.isEmpty() ? QString() : QStringLiteral(" ") + m_units;
    const auto text   = verdict == DataModel::OutputAdmission::NotANumber
                        ? tr("Enter a number from %1 to %2%3")
                        : tr("Allowed range: %1 to %2%3");
    setRangeMessage(text.arg(formatted(m_minValue), formatted(m_maxValue), suffix));
    return false;
  }

  value = admitted;
  setRangeMessage(QString());
  if (m_admittedValue != admitted) {
    m_admittedValue = admitted;
    Q_EMIT admittedValueChanged();
  }

  return true;
}

/**
 * @brief Sets the refusal text, notifying only on a change.
 */
void Widgets::Output::Base::setRangeMessage(const QString& message)
{
  if (m_rangeMessage == message)
    return;

  m_rangeMessage = message;
  Q_EMIT rangeMessageChanged();
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

  const int token = m_stateBinding.issueToken();
  const auto data = evaluateTransmitFunction(value, token);
  if (!data.isEmpty() && m_target.deliver)
    m_target.deliver(data);

  noteOperatorRequest(value, token, !data.isEmpty());
}

/**
 * @brief Runs the JavaScript transmit(value, token) function and returns the result as bytes.
 */
QByteArray Widgets::Output::Base::evaluateTransmitFunction(const QVariant& value, const int token)
{
  if (!m_hasFn)
    return {};

  auto jsValue      = m_jsEngine.toScriptValue(value);
  QJSValueList args = QJSValueList{jsValue, QJSValue(token)};
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

//--------------------------------------------------------------------------------------------------
// Presentation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The map QML reads to draw one control, shared by the dashboard panel and the editor
 *        preview so the two cannot drift apart.
 */
QVariantMap Widgets::Output::Base::presentation(const DataModel::OutputWidget& config)
{
  QVariantMap map;
  map[QStringLiteral("icon")] =
    config.icon.isEmpty() ? QString() : Misc::IconEngine::resolveActionIconSource(config.icon);
  map[QStringLiteral("title")]        = config.title;
  map[QStringLiteral("type")]         = static_cast<int>(config.type);
  map[QStringLiteral("minValue")]     = config.minValue;
  map[QStringLiteral("maxValue")]     = config.maxValue;
  map[QStringLiteral("stepSize")]     = config.stepSize;
  map[QStringLiteral("initialValue")] = config.initialValue;
  map[QStringLiteral("monoIcon")]     = config.monoIcon;
  map[QStringLiteral("checkable")]    = config.checkable;
  map[QStringLiteral("style")]        = static_cast<int>(config.style);
  map[QStringLiteral("labels")]       = config.positionLabels;
  map[QStringLiteral("springReturn")] = config.springReturn;
  map[QStringLiteral("hideTitle")]    = config.hideTitle;
  map[QStringLiteral("color")]        = config.color;
  map[QStringLiteral("sizeScale")]    = DataModel::outputSizeScale(config.size);
  map[QStringLiteral("onLabel")]      = config.onLabel;
  map[QStringLiteral("offLabel")]     = config.offLabel;
  map[QStringLiteral("groupId")]      = config.groupId;
  map[QStringLiteral("widgetId")]     = config.widgetId;
  map[QStringLiteral("inputKind")]    = static_cast<int>(config.inputKind);
  map[QStringLiteral("decimals")]     = config.decimals;
  map[QStringLiteral("units")]        = config.units;
  return map;
}
