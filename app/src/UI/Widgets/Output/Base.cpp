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

#include "IO/ConnectionManager.h"
#include "Licensing/CommercialToken.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an output widget base from a config struct.
 *
 * Compiles the user's transmit(value) JavaScript function once at construction
 * time and caches it for repeated evaluation.
 */
Widgets::Output::Base::Base(const DataModel::OutputWidget& config, QQuickItem* parent)
  : QQuickItem(parent)
  , m_sourceId(config.sourceId)
  , m_minValue(config.minValue)
  , m_maxValue(config.maxValue)
  , m_stepSize(config.stepSize)
  , m_title(config.title)
  , m_hasFn(false)
{
  m_rateLimiter.start();
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
 *
 * Rate-limited to prevent flooding the connection. Requires an active Pro
 * license and a connected device.
 *
 * @param value The widget value to transmit (double for sliders, bool for
 *              toggles, string for text fields, 1 for buttons).
 */
void Widgets::Output::Base::sendValue(const QVariant& value)
{
  if (m_rateLimiter.elapsed() < kMinSendIntervalMs)
    return;

  m_rateLimiter.restart();
  const auto& tk = Licensing::CommercialToken::current();
  if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Pro)
    return;

  const auto data = evaluateTransmitFunction(value);
  if (!data.isEmpty())
    IO::ConnectionManager::instance().writeDataToDevice(m_sourceId, data);
}

/**
 * @brief Runs the JavaScript transmit(value) function and returns the result
 *        as a QByteArray.
 *
 * @param value The value to pass to the JS function.
 * @return The bytes to transmit, or empty on error.
 */
QByteArray Widgets::Output::Base::evaluateTransmitFunction(const QVariant& value)
{
  if (!m_hasFn)
    return {};

  auto jsValue = m_jsEngine.toScriptValue(value);
  auto result  = m_transmitFn.call(QJSValueList{jsValue});

  if (result.isError()) {
    Q_EMIT transmitError(result.toString());
    return {};
  }

  if (result.isString())
    return result.toString().toUtf8();

  return result.toVariant().toByteArray();
}
