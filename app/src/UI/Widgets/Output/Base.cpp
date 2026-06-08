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
  installProtocolHelpers(m_jsEngine);

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
 * @brief Installs protocol-aware helper functions into the JS engine.
 */
void Widgets::Output::Base::installProtocolHelpers(QJSEngine& engine)
{
  // clang-format off
  static const QString kHelpers = QStringLiteral(

    "function modbusWriteRegister(address, value) {"
    "  var a = address & 0xFFFF;"
    "  var v = Math.round(value) & 0xFFFF;"
    "  return String.fromCharCode("
    "    (a >> 8) & 0xFF, a & 0xFF,"
    "    (v >> 8) & 0xFF, v & 0xFF);"
    "}"

    "function modbusWriteCoil(address, on) {"
    "  return modbusWriteRegister(address, on ? 0xFF00 : 0x0000);"
    "}"

    "function modbusWriteFloat(address, value) {"
    "  var buf = new ArrayBuffer(4);"
    "  new DataView(buf).setFloat32(0, value, false);"
    "  var b = new Uint8Array(buf);"
    "  var a = address & 0xFFFF;"
    "  return String.fromCharCode("
    "    (a >> 8) & 0xFF, a & 0xFF,"
    "    b[0], b[1], b[2], b[3]);"
    "}"

    "function canSendFrame(id, payload) {"
    "  var canId = id & 0xFFFF;"
    "  var data = '';"
    "  if (typeof payload === 'string')"
    "    data = payload;"
    "  else if (Array.isArray(payload)) {"
    "    for (var i = 0; i < payload.length; i++)"
    "      data += String.fromCharCode(payload[i] & 0xFF);"
    "  }"
    "  var dlc = data.length;"
    "  return String.fromCharCode("
    "    (canId >> 8) & 0xFF, canId & 0xFF, dlc) + data;"
    "}"

    "function canSendValue(id, value, bytes) {"
    "  bytes = bytes || 2;"
    "  var arr = [];"
    "  var v = Math.round(value);"
    "  for (var i = bytes - 1; i >= 0; i--)"
    "    arr.push((v >> (i * 8)) & 0xFF);"
    "  return canSendFrame(id, arr);"
    "}"
  );
  // clang-format on

  auto result = engine.evaluate(kHelpers);
  if (result.isError())
    qWarning() << "Failed to install protocol helpers:" << result.toString();

  DataModel::NotificationCenter::installScriptApi(&engine);
}
