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
  // Inject protocol helper functions before compiling user code
  m_rateLimiter.start();
  installProtocolHelpers(m_jsEngine);

  // Compile the user's transmit function
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
  // Enforce rate limit
  if (m_rateLimiter.elapsed() < kMinSendIntervalMs)
    return;

  // Validate license tier
  m_rateLimiter.restart();
  const auto& tk = Licensing::CommercialToken::current();
  if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Pro)
    return;

  // Evaluate JS function and transmit result
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
  // Abort if no transmit function was compiled
  if (!m_hasFn)
    return {};

  // Invoke the JS function with the widget value
  auto jsValue = m_jsEngine.toScriptValue(value);
  auto result  = m_transmitFn.call(QJSValueList{jsValue});

  // Handle JS execution errors
  if (result.isError()) {
    Q_EMIT transmitError(result.toString());
    return {};
  }

  // Convert result to byte array
  if (result.isString())
    return result.toString().toLatin1();

  return result.toVariant().toByteArray();
}

//--------------------------------------------------------------------------------------------------
// Protocol helper injection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs protocol-aware helper functions into the JS engine.
 *
 * These pure byte-packing utilities let users write simple transmit functions
 * like `return modbusWriteRegister(0x0001, value)` instead of manually
 * constructing binary payloads.
 *
 * @param engine The QJSEngine to install helpers into.
 */
void Widgets::Output::Base::installProtocolHelpers(QJSEngine& engine)
{
  // clang-format off
  static const QString kHelpers = QStringLiteral(
    // -----------------------------------------------------------------
    // Modbus helpers
    // -----------------------------------------------------------------

    // Write a single 16-bit value to a holding register.
    // Returns 4 bytes: [addr_hi, addr_lo, value_hi, value_lo]
    "function modbusWriteRegister(address, value) {"
    "  var a = address & 0xFFFF;"
    "  var v = Math.round(value) & 0xFFFF;"
    "  return String.fromCharCode("
    "    (a >> 8) & 0xFF, a & 0xFF,"
    "    (v >> 8) & 0xFF, v & 0xFF);"
    "}"

    // Write a coil (ON = 0xFF00, OFF = 0x0000).
    "function modbusWriteCoil(address, on) {"
    "  return modbusWriteRegister(address, on ? 0xFF00 : 0x0000);"
    "}"

    // Write an IEEE-754 float across two consecutive holding registers.
    // Returns 6 bytes: [addr_hi, addr_lo, r1_hi, r1_lo, r2_hi, r2_lo]
    "function modbusWriteFloat(address, value) {"
    "  var buf = new ArrayBuffer(4);"
    "  new DataView(buf).setFloat32(0, value, false);"
    "  var b = new Uint8Array(buf);"
    "  var a = address & 0xFFFF;"
    "  return String.fromCharCode("
    "    (a >> 8) & 0xFF, a & 0xFF,"
    "    b[0], b[1], b[2], b[3]);"
    "}"

    // -----------------------------------------------------------------
    // CAN Bus helpers
    // -----------------------------------------------------------------

    // Send an arbitrary CAN frame. Payload is a string or array of bytes.
    // Returns 3+ bytes: [id_hi, id_lo, dlc, payload...]
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

    // Send a numeric value packed into a CAN frame (big-endian, 1–8 bytes).
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
}
