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

#include "API/Handlers/IOManagerHandler.h"

#include <QJsonArray>
#include <QMetaEnum>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all IO::ConnectionManager commands with the registry
 */
void API::Handlers::IOManagerHandler::registerCommands()
{
  registerConnectionCommands();
  registerBusConfigCommands();
  registerQueryCommands();
}

/**
 * @brief Register connect/disconnect/setPaused lifecycle commands.
 */
void API::Handlers::IOManagerHandler::registerConnectionCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  registry.registerCommand(
    QStringLiteral("io.connect"),
    QStringLiteral("Open the I/O connection using the active bus type's currently-"
                   "configured device + parameters. Configure FIRST -- the active "
                   "bus's setters (io.uart.setDevice + setBaudRate, "
                   "io.network.setRemoteAddress + setTcpPort, io.ble.selectDevice, "
                   "etc.) must already be set. Use io.getStatus to verify "
                   "configurationOk before connecting. Idempotent if already "
                   "connected to the same device."),
    emptySchema,
    &connect);

  registry.registerCommand(
    QStringLiteral("io.disconnect"),
    QStringLiteral("Close the active I/O connection. Safe to call when already "
                   "disconnected. Does NOT reset the configuration -- the next "
                   "io.connect will reuse the same device + params."),
    emptySchema,
    &disconnect);

  {
    QJsonObject props;
    props[QStringLiteral("paused")] = QJsonObject{
      {       QStringLiteral("type"),              QStringLiteral("boolean")},
      {QStringLiteral("description"), QStringLiteral("Pause data streaming")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("paused")};
    registry.registerCommand(
      QStringLiteral("io.setPaused"),
      QStringLiteral("Pause or resume frame processing without disconnecting. The "
                     "device keeps streaming bytes; we drop them on the floor. Use "
                     "this when the user wants to inspect the current dashboard "
                     "state without it scrolling, or when running an export and the "
                     "user doesn't want new frames mid-export."),
      schema,
      &setPaused);
  }
}

/**
 * @brief Register setBusType + writeData configuration commands.
 */
void API::Handlers::IOManagerHandler::registerBusConfigCommands()
{
  auto& registry = CommandRegistry::instance();

  {
    QJsonObject props;
    props[Keys::BusType] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("integer")                                     },
      {QStringLiteral("description"),
       QStringLiteral("0=UART (serial port; most embedded boards), "
       "1=Network (TCP/UDP client/server), "
       "2=BLE (BLE GATT characteristic; Pro), "
       "3=Audio (microphone-as-data; Pro), "
       "4=Modbus (RTU/TCP; Pro), "
       "5=CAN (CANopen; Pro), "
       "6=USB (raw USB endpoints via libusb; Pro), "
       "7=HID (HID via hidapi; Pro), "
       "8=Process (spawn a subprocess and read its stdout; Pro).")               },
      {       QStringLiteral("enum"),       QJsonArray{0, 1, 2, 3, 4, 5, 6, 7, 8}},
      {    QStringLiteral("default"),                                           0}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QString(Keys::BusType)};
    registry.registerCommand(
      QStringLiteral("io.setBusType"),
      QStringLiteral("Switch the active bus. CRITICAL: this discards the previous "
                     "bus's runtime config (port, baud rate, host, etc. are NOT "
                     "carried over). After switching, configure the new bus with "
                     "its setters before calling io.connect. The project's source "
                     "list is independent -- this only affects what's currently "
                     "live."),
      schema,
      &setBusType);
  }

  {
    QJsonObject props;
    props[QStringLiteral("data")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("string")},
      {QStringLiteral("description"),
       QStringLiteral("Base64-encoded raw bytes to transmit. The device receives "
       "the decoded bytes verbatim -- no line ending appended, no "
       "encoding applied. For text + line ending, use console.send "
       "instead.")       }
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("data")};
    registry.registerCommand(
      QStringLiteral("io.writeData"),
      QStringLiteral("Transmit raw bytes to the device (params: data - base64 "
                     "encoded). Hardware-write: remote clients must clear the "
                     "one-time device-write consent prompt (the user's answer is "
                     "persisted); in-process scripts are not prompted. "
                     "Failure modes: not connected (connection_lost), bus busy "
                     "(bus_busy). Prefer console.send for text protocols."),
      schema,
      &writeData);
  }
}

/**
 * @brief Register io.getStatus and io.listBuses query commands.
 */
void API::Handlers::IOManagerHandler::registerQueryCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  registry.registerCommand(
    QStringLiteral("io.getStatus"),
    QStringLiteral("Returns the current connection state: isConnected, paused, "
                   "busType + busTypeLabel, configurationOk (true when the active "
                   "bus has enough config to call io.connect), readOnly/readWrite "
                   "capability flags. Call this first when the user reports a "
                   "connection issue."),
    emptySchema,
    &getStatus);

  registry.registerCommand(
    QStringLiteral("io.listBuses"),
    QStringLiteral("Returns the list of bus types this build supports. Pro builds "
                   "have all 9; GPL builds only ship UART/Network/BLE/Audio. Use "
                   "before io.setBusType to confirm the bus is available."),
    emptySchema,
    &getAvailableBuses);

  {
    QJsonObject sourceProp;
    sourceProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
    sourceProp.insert(QStringLiteral("description"),
                      QStringLiteral("Optional source filter; omit for the newest frame "
                                     "across all sources."));

    QJsonObject encodingProp;
    encodingProp.insert(QStringLiteral("type"), QStringLiteral("string"));
    encodingProp.insert(
      QStringLiteral("enum"),
      QJsonArray{QStringLiteral("text"), QStringLiteral("base64"), QStringLiteral("both")});
    encodingProp.insert(QStringLiteral("description"),
                        QStringLiteral("Payload encoding in the response (default: text)."));

    QJsonObject props;
    props.insert(Keys::SourceId, sourceProp);
    props.insert(QStringLiteral("encoding"), encodingProp);

    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), props);

    registry.registerCommand(
      QStringLiteral("io.getLatestFrame"),
      QStringLiteral("Returns the latest RAW frame received from the device: sequence "
                     "number, payload (text/base64) and the parser's channel tokens in "
                     "values[] -- including channels NOT yet mapped to any dataset "
                     "(dashboard.getData only shows mapped data). Poll it from a control "
                     "script loop() to detect new channels and auto-build widgets via "
                     "project mutations; the sequence field tells you when a new frame "
                     "arrived. Capture runs only while a control script is running or "
                     "the API server is enabled, so hasData:false means no data OR no "
                     "active consumer. timestampMs is a monotonic clock in milliseconds "
                     "(not Unix epoch time); use it only for deltas between frames."),
      schema,
      &getLatestFrame);
  }
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Connect to the configured device
 */
API::CommandResponse API::Handlers::IOManagerHandler::connect(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = IO::ConnectionManager::instance();

  if (manager.isConnected()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Already connected"));
  }

  if (!manager.configurationOk()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Device configuration is invalid"));
  }

  manager.connectDevice();

  QJsonObject result;
  result[QStringLiteral("connected")] = manager.isConnected();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Disconnect from the current device
 */
API::CommandResponse API::Handlers::IOManagerHandler::disconnect(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = IO::ConnectionManager::instance();

  if (!manager.isConnected()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Not connected"));
  }

  manager.disconnectDevice();

  QJsonObject result;
  result[QStringLiteral("connected")] = false;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Pause or resume data streaming
 */
API::CommandResponse API::Handlers::IOManagerHandler::setPaused(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("paused"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: paused"));
  }

  const bool paused = params.value(QStringLiteral("paused")).toBool();
  IO::ConnectionManager::instance().setPaused(paused);

  QJsonObject result;
  result[QStringLiteral("paused")] = paused;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the bus type
 */
API::CommandResponse API::Handlers::IOManagerHandler::setBusType(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(Keys::BusType)) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: busType"));
  }

  const int busType   = params.value(Keys::BusType).toInt();
  const auto metaEnum = QMetaEnum::fromType<SerialStudio::BusType>();
  if (!metaEnum.valueToKey(busType)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid busType: %1. Use io.listBuses to discover the values this "
                     "build supports.")
        .arg(busType));
  }

  IO::ConnectionManager::instance().setBusType(static_cast<SerialStudio::BusType>(busType));

  QJsonObject result;
  result[Keys::BusType]                 = busType;
  result[QStringLiteral("busTypeName")] = EnumLabels::busTypeLabel(busType);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Write raw data to the connected device
 */
API::CommandResponse API::Handlers::IOManagerHandler::writeData(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("data"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"));
  }

  const QString dataStr = params.value(QStringLiteral("data")).toString();
  const QByteArray data = QByteArray::fromBase64(dataStr.toUtf8());

  if (data.isEmpty() && !dataStr.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid base64 data"));
  }

  auto& manager = IO::ConnectionManager::instance();
  if (!manager.isConnected()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Not connected"));
  }

  const qint64 bytesWritten = manager.writeData(data);

  QJsonObject result;
  result[QStringLiteral("bytesWritten")] = bytesWritten;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get current connection status and configuration
 */
API::CommandResponse API::Handlers::IOManagerHandler::getStatus(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager     = IO::ConnectionManager::instance();
  const int busInt  = static_cast<int>(manager.busType());
  const bool ok     = manager.configurationOk();
  const bool live   = manager.isConnected();
  const bool paused = manager.paused();

  QJsonObject result;
  result[QStringLiteral("isConnected")]     = live;
  result[QStringLiteral("paused")]          = paused;
  result[Keys::BusType]                     = busInt;
  result[QStringLiteral("busTypeLabel")]    = API::EnumLabels::busTypeLabel(busInt);
  result[QStringLiteral("busTypeSlug")]     = API::EnumLabels::busTypeSlug(busInt);
  result[QStringLiteral("configurationOk")] = ok;
  result[QStringLiteral("readOnly")]        = manager.readOnly();
  result[QStringLiteral("readWrite")]       = manager.readWrite();

  const auto& availableBuses = manager.availableBuses();
  if (busInt >= 0 && busInt < availableBuses.count())
    result[QStringLiteral("busTypeName")] = availableBuses.at(busInt);

  QString summary;
  if (live)
    summary = QStringLiteral("Connected via %1%2.")
                .arg(API::EnumLabels::busTypeLabel(busInt))
                .arg(paused ? QStringLiteral(" (paused)") : QString());
  else if (!ok)
    summary = QStringLiteral("Not connected. The %1 driver is not fully "
                             "configured yet.")
                .arg(API::EnumLabels::busTypeLabel(busInt));
  else
    summary = QStringLiteral("Not connected. %1 is configured and ready "
                             "to open.")
                .arg(API::EnumLabels::busTypeLabel(busInt));

  result[QStringLiteral("_summary")] = summary;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the latest raw frame captured by FrameBuilder: payload plus parser tokens.
 *        An empty store is a state (hasData:false), not an error.
 */
API::CommandResponse API::Handlers::IOManagerHandler::getLatestFrame(const QString& id,
                                                                     const QJsonObject& params)
{
  int sourceId = -1;
  if (params.contains(Keys::SourceId))
    sourceId = params.value(Keys::SourceId).toInt(-1);

  QString encoding = QStringLiteral("text");
  if (params.contains(QStringLiteral("encoding")))
    encoding = params.value(QStringLiteral("encoding")).toString(encoding).toLower();

  const bool wantText   = (encoding != QStringLiteral("base64"));
  const bool wantBase64 = (encoding != QStringLiteral("text"));
  if (encoding != QStringLiteral("text") && encoding != QStringLiteral("base64")
      && encoding != QStringLiteral("both")) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid encoding: %1. Valid values: text, base64, both").arg(encoding));
  }

  const auto* latest = DataModel::FrameBuilder::instance().latestFrame(sourceId);
  if (!latest || !latest->chunk) {
    QJsonObject empty;
    empty[QStringLiteral("hasData")]  = false;
    empty[QStringLiteral("sequence")] = 0;
    empty[QStringLiteral("_summary")] =
      QStringLiteral("No frame captured yet. Capture runs only while a control script is "
                     "running or the API server is enabled, and requires incoming data.");
    return CommandResponse::makeSuccess(id, empty);
  }

  QJsonObject result;
  result[QStringLiteral("hasData")]     = true;
  result[QStringLiteral("sequence")]    = static_cast<qint64>(latest->sequence);
  result[QStringLiteral("timestampMs")] = latest->timestampMs;
  result[Keys::SourceId]                = latest->sourceId;

  const QByteArray& payload = latest->chunk->data;
  if (wantText)
    result[QStringLiteral("text")] = QString::fromUtf8(payload);

  if (wantBase64)
    result[QStringLiteral("base64")] = QString::fromLatin1(payload.toBase64());

  QJsonArray values;
  if (latest->channelsSequence == latest->sequence)
    for (const auto& channel : latest->channels)
      values.append(channel);

  result[QStringLiteral("valueCount")] = values.size();
  result[QStringLiteral("values")]     = values;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of available bus types
 */
API::CommandResponse API::Handlers::IOManagerHandler::getAvailableBuses(const QString& id,
                                                                        const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto buses = IO::ConnectionManager::instance().availableBuses();

  QJsonArray busArray;
  for (int i = 0; i < buses.count(); ++i) {
    QJsonObject bus;
    bus[QStringLiteral("index")] = i;
    bus[QStringLiteral("name")]  = buses.at(i);
    busArray.append(bus);
  }

  QJsonObject result;
  result[QStringLiteral("buses")] = busArray;
  return CommandResponse::makeSuccess(id, result);
}
