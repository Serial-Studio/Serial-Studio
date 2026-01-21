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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/UARTHandler.h"
#include "API/CommandRegistry.h"
#include "IO/Drivers/UART.h"
#include <QMetaObject>

/**
 * @brief Register all UART commands with the registry
 */
void API::Handlers::UARTHandler::registerCommands()
{
  auto &registry = CommandRegistry::instance();

  // Mutation commands
  registry.registerCommand(
      QStringLiteral("io.driver.uart.setDevice"),
      QStringLiteral("Set serial port by device name (params: device)"),
      &setDevice);

  registry.registerCommand(
      QStringLiteral("io.driver.uart.setPortIndex"),
      QStringLiteral("Set serial port by index (params: portIndex)"),
      &setPortIndex);

  registry.registerCommand(QStringLiteral("io.driver.uart.setBaudRate"),
                           QStringLiteral("Set baud rate (params: baudRate)"),
                           &setBaudRate);

  registry.registerCommand(
      QStringLiteral("io.driver.uart.setParity"),
      QStringLiteral("Set parity (params: parityIndex - 0=None, 1=Even, 2=Odd, "
                     "3=Space, 4=Mark)"),
      &setParity);

  registry.registerCommand(
      QStringLiteral("io.driver.uart.setDataBits"),
      QStringLiteral("Set data bits (params: dataBitsIndex - 0=5, 1=6, 2=7, "
                     "3=8)"),
      &setDataBits);

  registry.registerCommand(
      QStringLiteral("io.driver.uart.setStopBits"),
      QStringLiteral("Set stop bits (params: stopBitsIndex - 0=1, 1=1.5, 2=2)"),
      &setStopBits);

  registry.registerCommand(
      QStringLiteral("io.driver.uart.setFlowControl"),
      QStringLiteral("Set flow control (params: flowControlIndex - 0=None, "
                     "1=Hardware, 2=Software)"),
      &setFlowControl);

  registry.registerCommand(
      QStringLiteral("io.driver.uart.setDtrEnabled"),
      QStringLiteral("Enable/disable DTR signal (params: dtrEnabled)"),
      &setDtrEnabled);

  registry.registerCommand(
      QStringLiteral("io.driver.uart.setAutoReconnect"),
      QStringLiteral("Set auto-reconnect behavior (params: autoReconnect)"),
      &setAutoReconnect);

  // Query commands
  registry.registerCommand(QStringLiteral("io.driver.uart.getPortList"),
                           QStringLiteral("Get list of available serial ports"),
                           &getPortList);

  registry.registerCommand(QStringLiteral("io.driver.uart.getBaudRateList"),
                           QStringLiteral("Get list of common baud rates"),
                           &getBaudRateList);

  registry.registerCommand(QStringLiteral("io.driver.uart.getConfiguration"),
                           QStringLiteral("Get current UART configuration"),
                           &getConfiguration);
}

/**
 * @brief Set serial port by device name
 * @param params Requires "device" (string, e.g., "COM3" or "/dev/ttyUSB0")
 */
API::CommandResponse
API::Handlers::UARTHandler::setDevice(const QString &id,
                                      const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("device")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "device"));
  }

  const QString device = params.value(QStringLiteral("device")).toString();
  if (device.isEmpty())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Device name cannot be empty"));
  }

  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart, [uart, device]() { uart->registerDevice(device); },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("device")] = device;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set serial port by index
 * @param params Requires "portIndex" (int)
 */
API::CommandResponse
API::Handlers::UARTHandler::setPortIndex(const QString &id,
                                         const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("portIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "portIndex"));
  }

  const int portIndex = params.value(QStringLiteral("portIndex")).toInt();
  const auto &portList = IO::Drivers::UART::instance().portList();

  if (portIndex < 0 || portIndex >= portList.count())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid portIndex: %1. Valid range: 0-%2")
            .arg(portIndex)
            .arg(portList.count() - 1));
  }

  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart,
      [uart, portIndex]() {
        uart->setPortIndex(static_cast<quint8>(portIndex));
      },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("portIndex")] = portIndex;
  result[QStringLiteral("portName")] = portList.at(portIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set baud rate
 * @param params Requires "baudRate" (int)
 */
API::CommandResponse
API::Handlers::UARTHandler::setBaudRate(const QString &id,
                                        const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("baudRate")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "baudRate"));
  }

  const int baudRate = params.value(QStringLiteral("baudRate")).toInt();
  if (baudRate <= 0)
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("baudRate must be positive"));
  }

  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart, [uart, baudRate]() { uart->setBaudRate(baudRate); },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("baudRate")] = baudRate;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set parity
 * @param params Requires "parityIndex" (int: 0=None, 1=Even, 2=Odd, 3=Space,
 * 4=Mark)
 */
API::CommandResponse
API::Handlers::UARTHandler::setParity(const QString &id,
                                      const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("parityIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "parityIndex"));
  }

  const int parityIndex = params.value(QStringLiteral("parityIndex")).toInt();
  const auto &parityList = IO::Drivers::UART::instance().parityList();

  if (parityIndex < 0 || parityIndex >= parityList.count())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid parityIndex: %1. Valid range: 0-%2")
            .arg(parityIndex)
            .arg(parityList.count() - 1));
  }

  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart,
      [uart, parityIndex]() {
        uart->setParity(static_cast<quint8>(parityIndex));
      },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("parityIndex")] = parityIndex;
  result[QStringLiteral("parityName")] = parityList.at(parityIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set data bits
 * @param params Requires "dataBitsIndex" (int: 0=5, 1=6, 2=7, 3=8)
 */
API::CommandResponse
API::Handlers::UARTHandler::setDataBits(const QString &id,
                                        const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("dataBitsIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "dataBitsIndex"));
  }

  const int dataBitsIndex
      = params.value(QStringLiteral("dataBitsIndex")).toInt();
  const auto &dataBitsList = IO::Drivers::UART::instance().dataBitsList();

  if (dataBitsIndex < 0 || dataBitsIndex >= dataBitsList.count())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid dataBitsIndex: %1. Valid range: 0-%2")
            .arg(dataBitsIndex)
            .arg(dataBitsList.count() - 1));
  }

  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart,
      [uart, dataBitsIndex]() {
        uart->setDataBits(static_cast<quint8>(dataBitsIndex));
      },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("dataBitsIndex")] = dataBitsIndex;
  result[QStringLiteral("dataBitsValue")] = dataBitsList.at(dataBitsIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set stop bits
 * @param params Requires "stopBitsIndex" (int: 0=1, 1=1.5, 2=2)
 */
API::CommandResponse
API::Handlers::UARTHandler::setStopBits(const QString &id,
                                        const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("stopBitsIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "stopBitsIndex"));
  }

  const int stopBitsIndex
      = params.value(QStringLiteral("stopBitsIndex")).toInt();
  const auto &stopBitsList = IO::Drivers::UART::instance().stopBitsList();

  if (stopBitsIndex < 0 || stopBitsIndex >= stopBitsList.count())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid stopBitsIndex: %1. Valid range: 0-%2")
            .arg(stopBitsIndex)
            .arg(stopBitsList.count() - 1));
  }

  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart,
      [uart, stopBitsIndex]() {
        uart->setStopBits(static_cast<quint8>(stopBitsIndex));
      },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("stopBitsIndex")] = stopBitsIndex;
  result[QStringLiteral("stopBitsValue")] = stopBitsList.at(stopBitsIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set flow control
 * @param params Requires "flowControlIndex" (int: 0=None, 1=Hardware,
 * 2=Software)
 */
API::CommandResponse
API::Handlers::UARTHandler::setFlowControl(const QString &id,
                                           const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("flowControlIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "flowControlIndex"));
  }

  const int flowControlIndex
      = params.value(QStringLiteral("flowControlIndex")).toInt();
  const auto &flowControlList = IO::Drivers::UART::instance().flowControlList();

  if (flowControlIndex < 0 || flowControlIndex >= flowControlList.count())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid flowControlIndex: %1. Valid range: 0-%2")
            .arg(flowControlIndex)
            .arg(flowControlList.count() - 1));
  }

  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart,
      [uart, flowControlIndex]() {
        uart->setFlowControl(static_cast<quint8>(flowControlIndex));
      },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("flowControlIndex")] = flowControlIndex;
  result[QStringLiteral("flowControlName")]
      = flowControlList.at(flowControlIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable DTR signal
 * @param params Requires "dtrEnabled" (bool)
 */
API::CommandResponse
API::Handlers::UARTHandler::setDtrEnabled(const QString &id,
                                          const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("dtrEnabled")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "dtrEnabled"));
  }

  const bool dtrEnabled = params.value(QStringLiteral("dtrEnabled")).toBool();
  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart, [uart, dtrEnabled]() { uart->setDtrEnabled(dtrEnabled); },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("dtrEnabled")] = dtrEnabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set auto-reconnect behavior
 * @param params Requires "autoReconnect" (bool)
 */
API::CommandResponse
API::Handlers::UARTHandler::setAutoReconnect(const QString &id,
                                             const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("autoReconnect")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: "
                       "autoReconnect"));
  }

  const bool autoReconnect
      = params.value(QStringLiteral("autoReconnect")).toBool();
  auto *uart = &IO::Drivers::UART::instance();
  QMetaObject::invokeMethod(
      uart, [uart, autoReconnect]() { uart->setAutoReconnect(autoReconnect); },
      Qt::AutoConnection);

  QJsonObject result;
  result[QStringLiteral("autoReconnect")] = autoReconnect;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of available serial ports
 */
API::CommandResponse
API::Handlers::UARTHandler::getPortList(const QString &id,
                                        const QJsonObject &params)
{
  Q_UNUSED(params)

  const auto &portList = IO::Drivers::UART::instance().portList();

  QJsonArray ports;
  for (int i = 0; i < portList.count(); ++i)
  {
    QJsonObject port;
    port[QStringLiteral("index")] = i;
    port[QStringLiteral("name")] = portList.at(i);
    ports.append(port);
  }

  QJsonObject result;
  result[QStringLiteral("portList")] = ports;
  result[QStringLiteral("currentPortIndex")]
      = IO::Drivers::UART::instance().portIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of common baud rates
 */
API::CommandResponse
API::Handlers::UARTHandler::getBaudRateList(const QString &id,
                                            const QJsonObject &params)
{
  Q_UNUSED(params)

  const auto &baudRateList = IO::Drivers::UART::instance().baudRateList();

  QJsonArray baudRates;
  for (const auto &rate : baudRateList)
    baudRates.append(rate.toInt());

  QJsonObject result;
  result[QStringLiteral("baudRateList")] = baudRates;
  result[QStringLiteral("currentBaudRate")]
      = IO::Drivers::UART::instance().baudRate();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current UART configuration
 */
API::CommandResponse
API::Handlers::UARTHandler::getConfiguration(const QString &id,
                                             const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &uart = IO::Drivers::UART::instance();

  QJsonObject result;

  // Port info
  result[QStringLiteral("portIndex")] = uart.portIndex();
  const auto &portList = uart.portList();
  if (uart.portIndex() < portList.count())
    result[QStringLiteral("portName")] = portList.at(uart.portIndex());

  // Baud rate
  result[QStringLiteral("baudRate")] = uart.baudRate();

  // Parity
  result[QStringLiteral("parityIndex")] = uart.parityIndex();
  const auto &parityList = uart.parityList();
  if (uart.parityIndex() < parityList.count())
    result[QStringLiteral("parityName")] = parityList.at(uart.parityIndex());

  // Data bits
  result[QStringLiteral("dataBitsIndex")] = uart.dataBitsIndex();
  const auto &dataBitsList = uart.dataBitsList();
  if (uart.dataBitsIndex() < dataBitsList.count())
    result[QStringLiteral("dataBitsValue")]
        = dataBitsList.at(uart.dataBitsIndex());

  // Stop bits
  result[QStringLiteral("stopBitsIndex")] = uart.stopBitsIndex();
  const auto &stopBitsList = uart.stopBitsList();
  if (uart.stopBitsIndex() < stopBitsList.count())
    result[QStringLiteral("stopBitsValue")]
        = stopBitsList.at(uart.stopBitsIndex());

  // Flow control
  result[QStringLiteral("flowControlIndex")] = uart.flowControlIndex();
  const auto &flowControlList = uart.flowControlList();
  if (uart.flowControlIndex() < flowControlList.count())
    result[QStringLiteral("flowControlName")]
        = flowControlList.at(uart.flowControlIndex());

  // Other settings
  result[QStringLiteral("dtrEnabled")] = uart.dtrEnabled();
  result[QStringLiteral("autoReconnect")] = uart.autoReconnect();
  result[QStringLiteral("isOpen")] = uart.isOpen();
  result[QStringLiteral("configurationOk")] = uart.configurationOk();

  return CommandResponse::makeSuccess(id, result);
}
