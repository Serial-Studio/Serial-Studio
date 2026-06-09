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

#include "DataModel/NotificationCenter.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/ConnectionManager.h"
#include "Licensing/CommercialToken.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an output widget base from a config struct.
 */
Widgets::Output::Base::Base(const DataModel::OutputWidget& config, QQuickItem* parent)
  : QQuickItem(parent)
  , m_sourceId(config.sourceId)
  , m_minValue(config.minValue)
  , m_maxValue(config.maxValue)
  , m_stepSize(config.stepSize)
  , m_title(config.title)
  , m_txEncoding(static_cast<SerialStudio::TextEncoding>(config.txEncoding))
  , m_hasFn(false)
  , m_watchdog(&m_jsEngine, kTransmitWatchdogMs, QStringLiteral("transmit"))
{
  m_rateLimiter.start();
  DataModel::ScriptApiCall::installAll(&m_jsEngine, m_sourceId);

  if (!config.transmitFunction.isEmpty()) {
    const auto wrapped =
      QStringLiteral("(function() { %1; return transmit; })()").arg(config.transmitFunction);
    m_transmitFn = m_jsEngine.evaluate(wrapped);
    m_hasFn      = m_transmitFn.isCallable();

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

//--------------------------------------------------------------------------------------------------
// Transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Evaluates the transmit function and sends the result to the device.
 */
void Widgets::Output::Base::sendValue(const QVariant& value)
{
  if (m_rateLimiter.elapsed() < kMinSendIntervalMs)
    return;

  m_rateLimiter.restart();
  const auto& tk = Licensing::CommercialToken::current();
  if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Trial)
    return;

  const auto data = evaluateTransmitFunction(value);
  if (!data.isEmpty())
    (void)IO::ConnectionManager::instance().writeDataToDevice(m_sourceId, data);
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

//--------------------------------------------------------------------------------------------------
// Protocol helper injection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs the full SerialStudio SDK (protocol helpers included) into the engine.
 */
void Widgets::Output::Base::installProtocolHelpers(QJSEngine& engine)
{
  DataModel::ScriptApiCall::installAll(&engine, 0);
}
