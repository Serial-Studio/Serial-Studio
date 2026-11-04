/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "API/Handlers/UARTHandler.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaObject>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "Core/DataModel/Frame.h"
#include "Core/IO/ConnectionSequence.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/UART.h"

static constexpr int kMaxBaudRate = 15000000;

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the UART line-settings commands (baud / parity / bits / flow).
 */
void API::Handlers::UARTHandler::registerLineSettings(CommandRegistry& registry)
{
  registry.registerCommand(
    QStringLiteral("io.uart.setBaudRate"),
    QStringLiteral("Set baud rate (params: baudRate)"),
    API::makeSchema({API::rangeProp(QStringLiteral("baudRate"),
                                    QStringLiteral("Baud rate in bits per second (1-15000000)"),
                                    1,
                                    kMaxBaudRate)}),
    &setBaudRate);
  registry.registerCommand(
    QStringLiteral("io.uart.setParity"),
    QStringLiteral("Set parity (params: parityIndex - 0=None, 1=Even, 2=Odd, 3=Space, 4=Mark)"),
    API::makeSchema(
      {API::enumProp(QStringLiteral("parityIndex"),
                     QStringLiteral("Parity index (0=None, 1=Even, 2=Odd, 3=Space, 4=Mark)"),
                     QJsonArray{0, 1, 2, 3, 4},
                     0)}),
    &setParity);
  registry.registerCommand(
    QStringLiteral("io.uart.setDataBits"),
    QStringLiteral("Set data bits (params: dataBitsIndex - 0=5, 1=6, 2=7, 3=8)"),
    API::makeSchema({API::enumProp(QStringLiteral("dataBitsIndex"),
                                   QStringLiteral("Data bits index (0=5, 1=6, 2=7, 3=8)"),
                                   QJsonArray{0, 1, 2, 3},
                                   3)}),
    &setDataBits);
  registry.registerCommand(
    QStringLiteral("io.uart.setStopBits"),
    QStringLiteral("Set stop bits (params: stopBitsIndex - 0=1, 1=1.5, 2=2)"),
    API::makeSchema({API::enumProp(QStringLiteral("stopBitsIndex"),
                                   QStringLiteral("Stop bits index (0=1, 1=1.5, 2=2)"),
                                   QJsonArray{0, 1, 2},
                                   0)}),
    &setStopBits);
  registry.registerCommand(
    QStringLiteral("io.uart.setFlowControl"),
    QStringLiteral("Set flow control (params: flowControlIndex - 0=None, 1=Hardware, 2=Software)"),
    API::makeSchema(
      {API::enumProp(QStringLiteral("flowControlIndex"),
                     QStringLiteral("Flow control index (0=None, 1=Hardware, 2=Software)"),
                     QJsonArray{0, 1, 2},
                     0)}),
    &setFlowControl);
}

/**
 * @brief Registers the live line-control commands (spec 0103). They act on an open port, so they
 *        take an optional sourceId (default 0) and target that source's live driver.
 */
void API::Handlers::UARTHandler::registerLineControl(CommandRegistry& registry)
{
  const auto source = API::rangeProp(
    QString(Keys::SourceId), QStringLiteral("Source to act on (default 0)"), 0, QJsonValue());

  registry.registerCommand(
    QStringLiteral("io.uart.sendBreak"),
    QStringLiteral("Send BREAK: hold the connected line in BREAK, then release it "
                   "(params: durationMs, optional sourceId)"),
    API::makeSchema({API::rangeProp(QStringLiteral("durationMs"),
                                    QStringLiteral("BREAK duration in milliseconds (1-10000)"),
                                    1,
                                    IO::kMaxBreakMs)},
                    {source}),
    &sendBreak);
  registry.registerCommand(
    QStringLiteral("io.uart.setLiveBaudRate"),
    QStringLiteral("Change the baud rate of the connected port without closing it; the saved "
                   "rate is unchanged (params: baudRate, optional sourceId)"),
    API::makeSchema({API::rangeProp(QStringLiteral("baudRate"),
                                    QStringLiteral("Baud rate in bits per second (1-15000000)"),
                                    1,
                                    kMaxBaudRate)},
                    {source}),
    &setLiveBaudRate);
  registry.registerCommand(
    QStringLiteral("io.uart.getLineStates"),
    QStringLiteral("Read the modem lines of the connected port: cts, dsr, dcd, ri, rts and dtr "
                   "(params: optional sourceId)"),
    API::makeSchema({}, {source}),
    &getLineStates);
  registry.registerCommand(
    QStringLiteral("io.uart.runSequence"),
    QStringLiteral("Re-run the Connection Sequence on the connected port; frames pause until it "
                   "ends and the result is reported in the console (params: optional sourceId)"),
    API::makeSchema({}, {source}),
    &runSequence);
}

/**
 * @brief Registers the Hold RTS setting and the Connection Sequence commands of the serial port
 *        settings (the UI driver, like the other io.uart setters).
 */
void API::Handlers::UARTHandler::registerSequenceCommands(CommandRegistry& registry)
{
  registry.registerCommand(
    QStringLiteral("io.uart.setRtsEnabled"),
    QStringLiteral("Enable/disable Hold RTS: assert RTS on open and hold it; "
                   "not applicable under RTS/CTS flow control "
                   "(params: rtsEnabled)"),
    API::makeSchema({
      {QStringLiteral("rtsEnabled"),
       QStringLiteral("boolean"),
       QStringLiteral("Hold RTS asserted while connected")}
  }),
    &setRtsEnabled);
  registry.registerCommand(QStringLiteral("io.uart.getSequence"),
                           QStringLiteral("Get the Connection Sequence of the serial port"),
                           &getSequence);
  registry.registerCommand(
    QStringLiteral("io.uart.setSequence"),
    QStringLiteral("Replace the Connection Sequence of the serial port; it runs on every open "
                   "before data flows (params: sequence)"),
    API::makeSchema({
      {QStringLiteral("sequence"),
       QStringLiteral("object"),
       QStringLiteral("{budgetMs, steps: [{kind: break|wait|line|send|expect|discard|baud|flow, "
                      "...}]}; an empty steps array clears the sequence")}
  }),
    &setSequence);
}

/**
 * @brief Register all UART commands with the registry
 */
void API::Handlers::UARTHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.uart.setDevice"),
                           QStringLiteral("Set serial port by device name (params: device)"),
                           API::makeSchema({
                             {QStringLiteral("device"),
                              QStringLiteral("string"),
                              QStringLiteral("Device path (e.g. COM3, /dev/ttyUSB0)")}
  }),
                           &setDevice);

  registry.registerCommand(
    QStringLiteral("io.uart.setPortIndex"),
    QStringLiteral("Set serial port by index (params: portIndex)"),
    API::makeSchema({API::rangeProp(QStringLiteral("portIndex"),
                                    QStringLiteral("Serial port index (0-based)"),
                                    0,
                                    QJsonValue())}),
    &setPortIndex);

  registerLineSettings(registry);
  registerLineControl(registry);
  registerSequenceCommands(registry);

  registry.registerCommand(QStringLiteral("io.uart.setDtrEnabled"),
                           QStringLiteral("Enable/disable DTR signal (params: dtrEnabled)"),
                           API::makeSchema({
                             {QStringLiteral("dtrEnabled"),
                              QStringLiteral("boolean"),
                              QStringLiteral("Enable DTR signal")}
  }),
                           &setDtrEnabled);
  registry.registerCommand(QStringLiteral("io.uart.setAutoReconnect"),
                           QStringLiteral("Set auto-reconnect behavior (params: autoReconnect)"),
                           API::makeSchema({
                             {QStringLiteral("autoReconnect"),
                              QStringLiteral("boolean"),
                              QStringLiteral("Enable auto-reconnect")}
  }),
                           &setAutoReconnect);

  registry.registerCommand(QStringLiteral("io.uart.listPorts"),
                           QStringLiteral("Get list of available serial ports"),
                           &getPortList);
  registry.registerCommand(QStringLiteral("io.uart.listBaudRates"),
                           QStringLiteral("Get list of common baud rates"),
                           &getBaudRateList);
  registry.registerCommand(QStringLiteral("io.uart.getConfig"),
                           QStringLiteral("Get current UART configuration"),
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set serial port by device name
 */
API::CommandResponse API::Handlers::UARTHandler::setDevice(const QString& id,
                                                           const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("device"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: device"));
  }

  const QString device = params.value(QStringLiteral("device")).toString();
  if (device.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Device name cannot be empty"));
  }

  auto& connectionManager = API::handlerContext().connectionManager;
  auto* uart              = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart, [uart, device]() { uart->registerDevice(device); }, Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("device")] = device;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set serial port by index
 */
API::CommandResponse API::Handlers::UARTHandler::setPortIndex(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("portIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: portIndex"));
  }

  auto& connectionManager = API::handlerContext().connectionManager;
  const int portIndex     = params.value(QStringLiteral("portIndex")).toInt();
  const auto& portList    = connectionManager.uart()->portList();

  if (portIndex < 0 || portIndex >= portList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid portIndex: %1. Valid range: 0-%2")
                                        .arg(portIndex)
                                        .arg(portList.count() - 1));
  }

  auto* uart = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart,
    [uart, portIndex]() { uart->setPortIndex(static_cast<quint8>(portIndex)); },
    Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("portIndex")] = portIndex;
  result[QStringLiteral("portName")]  = portList.at(portIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set baud rate
 */
API::CommandResponse API::Handlers::UARTHandler::setBaudRate(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("baudRate"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: baudRate"));
  }

  const int baudRate = params.value(QStringLiteral("baudRate")).toInt();
  if (baudRate <= 0 || baudRate > kMaxBaudRate) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("baudRate must be between 1 and %1").arg(kMaxBaudRate));
  }

  auto& connectionManager = API::handlerContext().connectionManager;
  auto* uart              = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart, [uart, baudRate]() { uart->setBaudRate(baudRate); }, Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("baudRate")] = baudRate;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set parity
 */
API::CommandResponse API::Handlers::UARTHandler::setParity(const QString& id,
                                                           const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("parityIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: parityIndex"));
  }

  auto& connectionManager = API::handlerContext().connectionManager;
  const int parityIndex   = params.value(QStringLiteral("parityIndex")).toInt();
  const auto& parityList  = connectionManager.uart()->parityList();

  if (parityIndex < 0 || parityIndex >= parityList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid parityIndex: %1. Valid range: 0-%2")
                                        .arg(parityIndex)
                                        .arg(parityList.count() - 1));
  }

  auto* uart = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart,
    [uart, parityIndex]() { uart->setParity(static_cast<quint8>(parityIndex)); },
    Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("parityIndex")] = parityIndex;
  result[QStringLiteral("parityName")]  = parityList.at(parityIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set data bits
 */
API::CommandResponse API::Handlers::UARTHandler::setDataBits(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("dataBitsIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: dataBitsIndex"));
  }

  auto& connectionManager  = API::handlerContext().connectionManager;
  const int dataBitsIndex  = params.value(QStringLiteral("dataBitsIndex")).toInt();
  const auto& dataBitsList = connectionManager.uart()->dataBitsList();

  if (dataBitsIndex < 0 || dataBitsIndex >= dataBitsList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid dataBitsIndex: %1. Valid range: 0-%2")
                                        .arg(dataBitsIndex)
                                        .arg(dataBitsList.count() - 1));
  }

  auto* uart = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart,
    [uart, dataBitsIndex]() { uart->setDataBits(static_cast<quint8>(dataBitsIndex)); },
    Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("dataBitsIndex")] = dataBitsIndex;
  result[QStringLiteral("dataBitsValue")] = dataBitsList.at(dataBitsIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set stop bits
 */
API::CommandResponse API::Handlers::UARTHandler::setStopBits(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("stopBitsIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: stopBitsIndex"));
  }

  auto& connectionManager  = API::handlerContext().connectionManager;
  const int stopBitsIndex  = params.value(QStringLiteral("stopBitsIndex")).toInt();
  const auto& stopBitsList = connectionManager.uart()->stopBitsList();

  if (stopBitsIndex < 0 || stopBitsIndex >= stopBitsList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid stopBitsIndex: %1. Valid range: 0-%2")
                                        .arg(stopBitsIndex)
                                        .arg(stopBitsList.count() - 1));
  }

  auto* uart = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart,
    [uart, stopBitsIndex]() { uart->setStopBits(static_cast<quint8>(stopBitsIndex)); },
    Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("stopBitsIndex")] = stopBitsIndex;
  result[QStringLiteral("stopBitsValue")] = stopBitsList.at(stopBitsIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set flow control
 */
API::CommandResponse API::Handlers::UARTHandler::setFlowControl(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("flowControlIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: flowControlIndex"));
  }

  auto& connectionManager     = API::handlerContext().connectionManager;
  const int flowControlIndex  = params.value(QStringLiteral("flowControlIndex")).toInt();
  const auto& flowControlList = connectionManager.uart()->flowControlList();

  if (flowControlIndex < 0 || flowControlIndex >= flowControlList.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid flowControlIndex: %1. Valid range: 0-%2")
        .arg(flowControlIndex)
        .arg(flowControlList.count() - 1));
  }

  auto* uart = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart,
    [uart, flowControlIndex]() { uart->setFlowControl(static_cast<quint8>(flowControlIndex)); },
    Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("flowControlIndex")] = flowControlIndex;
  result[QStringLiteral("flowControlName")]  = flowControlList.at(flowControlIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable DTR signal
 */
API::CommandResponse API::Handlers::UARTHandler::setDtrEnabled(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("dtrEnabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: dtrEnabled"));
  }

  const bool dtrEnabled   = params.value(QStringLiteral("dtrEnabled")).toBool();
  auto& connectionManager = API::handlerContext().connectionManager;
  auto* uart              = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart, [uart, dtrEnabled]() { uart->setDtrEnabled(dtrEnabled); }, Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("dtrEnabled")] = dtrEnabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set auto-reconnect behavior
 */
API::CommandResponse API::Handlers::UARTHandler::setAutoReconnect(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("autoReconnect"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: autoReconnect"));
  }

  const bool autoReconnect = params.value(QStringLiteral("autoReconnect")).toBool();
  auto& connectionManager  = API::handlerContext().connectionManager;
  auto* uart               = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart, [uart, autoReconnect]() { uart->setAutoReconnect(autoReconnect); }, Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("autoReconnect")] = autoReconnect;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Line control & Connection Sequence (spec 0103)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves the live UART driver of the source a live-control command names. The io.*
 *        convention for a command that acts on an open port: an optional sourceId, default 0,
 *        because the UI driver has no port and a multi-source project has one per source. A
 *        source that is not a serial port, or not connected, is refused with the reason.
 */
static IO::Drivers::UART* liveUart(const QJsonObject& params, QString& error)
{
  const int sourceId = params.value(Keys::SourceId).toInt(0);
  auto& manager      = API::handlerContext().connectionManager;
  auto* uart         = qobject_cast<IO::Drivers::UART*>(manager.driver(sourceId));
  if (!uart) {
    error = QStringLiteral("Source %1 is not a serial port").arg(sourceId);
    return nullptr;
  }

  if (!uart->isOpen()) {
    error = QStringLiteral("Source %1 is not connected").arg(sourceId);
    return nullptr;
  }

  return uart;
}

/**
 * @brief Enable or disable Hold RTS
 */
API::CommandResponse API::Handlers::UARTHandler::setRtsEnabled(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("rtsEnabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: rtsEnabled"));
  }

  const bool rtsEnabled   = params.value(QStringLiteral("rtsEnabled")).toBool();
  auto& connectionManager = API::handlerContext().connectionManager;
  auto* uart              = connectionManager.uart();
  QMetaObject::invokeMethod(
    uart, [uart, rtsEnabled]() { uart->setRtsEnabled(rtsEnabled); }, Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("rtsEnabled")] = rtsEnabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Hold a connected source's line in BREAK for durationMs, then release it
 */
API::CommandResponse API::Handlers::UARTHandler::sendBreak(const QString& id,
                                                           const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("durationMs"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: durationMs"));
  }

  const int durationMs = params.value(QStringLiteral("durationMs")).toInt(0);
  if (durationMs < 1 || durationMs > IO::kMaxBreakMs) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("durationMs must be between 1 and %1").arg(IO::kMaxBreakMs));
  }

  QString error;
  auto* uart = liveUart(params, error);
  if (!uart)
    return CommandResponse::makeError(id, ErrorCode::ExecutionError, error);

  if (!uart->sendBreak(durationMs)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("The port refused BREAK, or a BREAK or a Connection Sequence is running"));
  }

  QJsonObject result;
  result[QStringLiteral("durationMs")] = durationMs;
  result[Keys::SourceId]               = params.value(Keys::SourceId).toInt(0);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Change a connected source's running baud rate, leaving the saved one alone
 */
API::CommandResponse API::Handlers::UARTHandler::setLiveBaudRate(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("baudRate"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: baudRate"));
  }

  const int baudRate = params.value(QStringLiteral("baudRate")).toInt();
  if (baudRate <= 0 || baudRate > kMaxBaudRate) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("baudRate must be between 1 and %1").arg(kMaxBaudRate));
  }

  QString error;
  auto* uart = liveUart(params, error);
  if (!uart)
    return CommandResponse::makeError(id, ErrorCode::ExecutionError, error);

  if (!uart->setLiveBaudRate(baudRate)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("The port refused baud rate %1, or a Connection Sequence is running")
        .arg(baudRate));
  }

  QJsonObject result;
  result[QStringLiteral("liveBaudRate")] = uart->liveBaudRate();
  result[QStringLiteral("baudRate")]     = uart->baudRate();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Read the modem lines of a connected source
 */
API::CommandResponse API::Handlers::UARTHandler::getLineStates(const QString& id,
                                                               const QJsonObject& params)
{
  QString error;
  auto* uart = liveUart(params, error);
  if (!uart)
    return CommandResponse::makeError(id, ErrorCode::ExecutionError, error);

  auto result            = uart->lineStates();
  result[Keys::SourceId] = params.value(Keys::SourceId).toInt(0);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Re-run the Connection Sequence of a connected source; the outcome is reported in the
 *        console and, on failure, the notification center
 */
API::CommandResponse API::Handlers::UARTHandler::runSequence(const QString& id,
                                                             const QJsonObject& params)
{
  QString error;
  auto* uart = liveUart(params, error);
  if (!uart)
    return CommandResponse::makeError(id, ErrorCode::ExecutionError, error);

  if (!uart->runSequence()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("No Connection Sequence is set, or a BREAK or a sequence is running"));
  }

  QJsonObject result;
  result[QStringLiteral("started")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the Connection Sequence of the serial port settings
 */
API::CommandResponse API::Handlers::UARTHandler::getSequence(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto json         = connectionManager.uart()->connectionSequenceJson().toUtf8();
  const auto sequence = IO::ConnectionSequence::fromJson(QJsonDocument::fromJson(json).object());
  const auto error    = sequence.validate();

  QJsonObject result;
  result[QStringLiteral("sequence")]  = sequence.toJson();
  result[QStringLiteral("stepCount")] = static_cast<int>(sequence.steps.size());
  result[QStringLiteral("valid")]     = error.message.isEmpty();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Replace the Connection Sequence of the serial port settings; a sequence that does not
 *        validate is refused with the index of the step at fault
 */
API::CommandResponse API::Handlers::UARTHandler::setSequence(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.value(QStringLiteral("sequence")).isObject()) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required object parameter: sequence"));
  }

  const auto sequence =
    IO::ConnectionSequence::fromJson(params.value(QStringLiteral("sequence")).toObject());
  const auto error = sequence.validate();
  if (!error.message.isEmpty()) {
    QJsonObject data;
    data[QStringLiteral("step")] = error.step;
    const auto where =
      error.step < 0 ? QStringLiteral("Sequence") : QStringLiteral("Step %1").arg(error.step + 1);
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("%1: %2").arg(where, error.message), data);
  }

  auto& connectionManager = API::handlerContext().connectionManager;
  auto* uart              = connectionManager.uart();
  const auto json         = sequence.toJson();
  QMetaObject::invokeMethod(
    uart, [uart, json]() { uart->setConnectionSequence(json); }, Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("stepCount")] = static_cast<int>(sequence.steps.size());
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get list of available serial ports
 */
API::CommandResponse API::Handlers::UARTHandler::getPortList(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& portList    = connectionManager.uart()->portList();

  QJsonArray ports;
  for (int i = 0; i < portList.count(); ++i) {
    QJsonObject port;
    port[QStringLiteral("index")] = i;
    port[QStringLiteral("name")]  = portList.at(i);
    ports.append(port);
  }

  QJsonObject result;
  result[QStringLiteral("portList")]         = ports;
  result[QStringLiteral("currentPortIndex")] = connectionManager.uart()->portIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of common baud rates
 */
API::CommandResponse API::Handlers::UARTHandler::getBaudRateList(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& connectionManager  = API::handlerContext().connectionManager;
  const auto& baudRateList = connectionManager.uart()->baudRateList();

  QJsonArray baudRates;
  for (const auto& rate : baudRateList)
    baudRates.append(rate.toInt());

  QJsonObject result;
  result[QStringLiteral("baudRateList")]    = baudRates;
  result[QStringLiteral("currentBaudRate")] = connectionManager.uart()->baudRate();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current UART configuration
 */
API::CommandResponse API::Handlers::UARTHandler::getConfiguration(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& connectionManager = API::handlerContext().connectionManager;
  auto* uart              = connectionManager.uart();

  QJsonObject result;

  result[QStringLiteral("portIndex")] = uart->portIndex();
  const auto& portList                = uart->portList();
  if (uart->portIndex() < portList.count())
    result[QStringLiteral("portName")] = portList.at(uart->portIndex());

  result[QStringLiteral("baudRate")] = uart->baudRate();

  result[QStringLiteral("parityIndex")] = uart->parityIndex();
  const auto& parityList                = uart->parityList();
  if (uart->parityIndex() < parityList.count())
    result[QStringLiteral("parityName")] = parityList.at(uart->parityIndex());

  result[QStringLiteral("dataBitsIndex")] = uart->dataBitsIndex();
  const auto& dataBitsList                = uart->dataBitsList();
  if (uart->dataBitsIndex() < dataBitsList.count())
    result[QStringLiteral("dataBitsValue")] = dataBitsList.at(uart->dataBitsIndex());

  result[QStringLiteral("stopBitsIndex")] = uart->stopBitsIndex();
  const auto& stopBitsList                = uart->stopBitsList();
  if (uart->stopBitsIndex() < stopBitsList.count())
    result[QStringLiteral("stopBitsValue")] = stopBitsList.at(uart->stopBitsIndex());

  result[QStringLiteral("flowControlIndex")] = uart->flowControlIndex();
  const auto& flowControlList                = uart->flowControlList();
  if (uart->flowControlIndex() < flowControlList.count())
    result[QStringLiteral("flowControlName")] = flowControlList.at(uart->flowControlIndex());

  result[QStringLiteral("dtrEnabled")]      = uart->dtrEnabled();
  result[QStringLiteral("rtsEnabled")]      = uart->rtsEnabled();
  result[QStringLiteral("autoReconnect")]   = uart->autoReconnect();
  result[QStringLiteral("isOpen")]          = uart->isOpen();
  result[QStringLiteral("configurationOk")] = uart->configurationOk();

  const auto* live = qobject_cast<IO::Drivers::UART*>(connectionManager.driver(0));
  const auto steps = IO::ConnectionSequence::fromVariant(uart->connectionSequenceJson()).steps;
  result[QStringLiteral("liveBaudRate")]      = live ? live->liveBaudRate() : uart->baudRate();
  result[QStringLiteral("sequenceStepCount")] = static_cast<int>(steps.size());

  return CommandResponse::makeSuccess(id, result);
}
