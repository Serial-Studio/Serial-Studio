/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "IO/ConnectionManager/DeviceIoRouter.h"

#include "API/Server.h"
#include "AppState.h"
#include "Console/Handler.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager/ReplyCapture.h"
#include "IO/DeviceManager.h"
#include "IO/FileTransmission.h"
#include "IO/HAL_Driver.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#  include "Sessions/Export.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

static const QByteArray kDefaultStart = QByteArray("/*");
static const QByteArray kDefaultEnd   = QByteArray("*/");

/**
 * @brief Binds the collaborators the payload and write paths reach; the console, API and gRPC
 *        pointers are bound BY REFERENCE because the composition root fills them after this.
 */
IO::DeviceIoRouter::DeviceIoRouter(AppState& appState,
                                   DataModel::FrameBuilder& frameBuilder,
                                   ReplyCapture& replyCapture,
                                   const DeviceTable& devices,
                                   const std::atomic<bool>& paused,
                                   Console::Handler* const& console,
                                   API::Server* const& apiServer,
                                   FileTransmission* const& fileTransmission
#ifdef BUILD_COMMERCIAL
                                   ,
                                   Sessions::Export* const& sessionExport,
                                   MQTT::Publisher* const& mqttPublisher
#endif
#ifdef ENABLE_GRPC
                                   ,
                                   API::GRPC::GRPCServer* const& grpcServer
#endif
                                   )
  : m_startSequence(kDefaultStart)
  , m_finishSequence(kDefaultEnd)
  , m_appState(appState)
  , m_frameBuilder(frameBuilder)
  , m_replyCapture(replyCapture)
  , m_devices(devices)
  , m_paused(paused)
  , m_console(console)
  , m_apiServer(apiServer)
  , m_fileTransmission(fileTransmission)
#ifdef BUILD_COMMERCIAL
  , m_sessionExport(sessionExport)
  , m_mqttPublisher(mqttPublisher)
#endif
#ifdef ENABLE_GRPC
  , m_grpcServer(grpcServer)
#endif
{}

//--------------------------------------------------------------------------------------------------
// Inbound payloads
//--------------------------------------------------------------------------------------------------

/**
 * @brief Feeds one payload to the read-only observers: the API tap, the console, and gRPC when
 *        the build has it. Never the recording sinks, which see blocks and not bytes.
 */
void IO::DeviceIoRouter::tapObservers(const QByteArray& bytes)
{
  SS_ASSERT(m_apiServer != nullptr, return);
  SS_ASSERT(m_console != nullptr, return);

  m_apiServer->hotpathTxData(bytes);
  m_console->hotpathRxData(bytes);

#ifdef ENABLE_GRPC
  if (m_grpcServer)
    m_grpcServer->hotpathTxData(bytes);
#endif
}

/**
 * @brief Forwards raw bytes from device @p deviceId to the console, the API server and the
 *        recording taps, and feeds the reply capture while it is armed.
 */
void IO::DeviceIoRouter::onRawDataReceived(int deviceId, const CapturedDataPtr& data)
{
  SS_ASSERT(data != nullptr, return);
  SS_ASSERT_LOG(!data->data.isEmpty());
  SS_ASSERT_LOG(deviceId >= 0);

  if (m_paused)
    return;

  if (m_replyCapture.armed()) [[unlikely]]
    m_replyCapture.record(deviceId, data->data);

  SS_ASSERT(m_console != nullptr && m_apiServer != nullptr && m_fileTransmission != nullptr,
            return);

  m_apiServer->hotpathTxData(data->data);
  m_console->hotpathRxDeviceData(deviceId, data->data);

  if (m_fileTransmission->active()) [[unlikely]]
    m_fileTransmission->onRawDataReceived(data->data);

#ifdef BUILD_COMMERCIAL
  m_sessionExport->hotpathTxRawBytes(deviceId, data);
  m_mqttPublisher->hotpathTxRawBytes(deviceId, data);
#endif

#ifdef ENABLE_GRPC
  m_grpcServer->hotpathTxData(data->data);
#endif
}

/**
 * @brief Forwards a stream-lane source's terminal-only bytes from device @p deviceId to the
 *        console. The typed sample blocks already fed the dashboard, the exports and the API,
 *        so this text stops at the terminal and nothing is recorded twice.
 */
void IO::DeviceIoRouter::onConsoleDataReceived(int deviceId, const CapturedDataPtr& data)
{
  SS_ASSERT(data != nullptr, return);
  SS_ASSERT_LOG(deviceId >= 0);

  if (m_paused)
    return;

  SS_ASSERT(m_console != nullptr, return);

  m_console->hotpathRxDeviceData(deviceId, data->data);
}

/**
 * @brief Feeds a pre-built payload into the frame pipeline. The builder call marshals to the
 *        pipeline thread (queued; command-rate, never per frame) while the console/API taps
 *        stay on this thread.
 */
void IO::DeviceIoRouter::processPayload(const QByteArray& payload)
{
  if (payload.isEmpty())
    return;

  SS_ASSERT(m_console != nullptr && m_apiServer != nullptr, return);

  auto* frameBuilder = &m_frameBuilder;

  const auto captured = makeCapturedData(payload);
  tapObservers(captured->data);

  const bool projectMode = (m_appState.operationMode() == SerialStudio::ProjectFile);
  frameBuilder->invokeOnBuilderThread([frameBuilder, captured, projectMode] {
    if (projectMode)
      frameBuilder->hotpathRxSourceFrame(0, captured);
    else
      frameBuilder->hotpathRxFrame(captured);
  });
}

/**
 * @brief Injects per-source payloads for multi-source playback.
 */
void IO::DeviceIoRouter::processMultiSourcePayload(const QByteArray& fullPayload,
                                                   const QMap<int, QByteArray>& sourcePayloads)
{
  SS_ASSERT_LOG(!sourcePayloads.isEmpty());

  if (fullPayload.isEmpty())
    return;

  SS_ASSERT(m_console != nullptr && m_apiServer != nullptr, return);

  auto* frameBuilder = &m_frameBuilder;

  const auto captured = makeCapturedData(fullPayload);
  tapObservers(captured->data);

  for (auto it = sourcePayloads.constBegin(); it != sourcePayloads.constEnd(); ++it) {
    const int sourceId  = it.key();
    const auto srcChunk = makeCapturedData(it.value(), captured->timestamp);
    frameBuilder->invokeOnBuilderThread([frameBuilder, sourceId, srcChunk] {
      frameBuilder->hotpathRxSourceFrame(sourceId, srcChunk);
    });
  }
}

//--------------------------------------------------------------------------------------------------
// Outbound writes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes @p data to @p deviceId and echoes only the bytes the driver reported written, so
 *        the console never shows a partial write as a full one.
 */
qint64 IO::DeviceIoRouter::writeToDevice(int deviceId, const QByteArray& data)
{
  SS_ASSERT(deviceId >= 0, return -1);
  SS_ASSERT(!data.isEmpty(), return -1);

  auto it = m_devices.find(deviceId);
  if (it == m_devices.end() || !it->second)
    return -1;

  const qint64 bytes = it->second->write(data);
  if (bytes > 0) {
    auto writtenData          = data;
    const qint64 boundedBytes = qMin<qint64>(bytes, writtenData.size());
    writtenData.chop(writtenData.length() - boundedBytes);
    if (m_console)
      m_console->displaySentData(deviceId, writtenData);
  }

  return bytes;
}

/**
 * @brief Arms reply capture for @p deviceId then writes @p data, atomically on this thread so
 *        no inbound bytes can slip in between the arm and the write. Backs deviceWriteAndWait():
 *        a control-script worker marshals here, then polls pollReplyBuffer() until satisfied.
 */
qint64 IO::DeviceIoRouter::writeAndArmReply(int deviceId, const QByteArray& data)
{
  SS_ASSERT(deviceId >= 0, return -1);
  SS_ASSERT(!data.isEmpty(), return -1);

  m_replyCapture.arm(deviceId);

  return writeToDevice(deviceId, data);
}

/**
 * @brief Returns a copy of the bytes captured for @p deviceId since the last arm.
 */
QByteArray IO::DeviceIoRouter::pollReplyBuffer(int deviceId) const
{
  SS_ASSERT(deviceId >= 0, return {});

  return m_replyCapture.poll(deviceId);
}

/**
 * @brief Drops the capture buffer for @p deviceId, disarming the tap once no buffers remain.
 */
void IO::DeviceIoRouter::disarmReplyCapture(int deviceId)
{
  SS_ASSERT(deviceId >= 0, return);

  m_replyCapture.disarm(deviceId);
}

//--------------------------------------------------------------------------------------------------
// Framing configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the start delimiter, falling back to the default when cleared; true when it moved,
 *        which is what makes the caller recreate device 0's FrameReader.
 */
bool IO::DeviceIoRouter::setStartSequence(const QByteArray& sequence)
{
  const auto effective = sequence.isEmpty() ? kDefaultStart : sequence;
  if (m_startSequence == effective)
    return false;

  m_startSequence = effective;
  return true;
}

/**
 * @brief Sets the end delimiter, falling back to the default when cleared; true when it moved.
 */
bool IO::DeviceIoRouter::setFinishSequence(const QByteArray& sequence)
{
  const auto effective = sequence.isEmpty() ? kDefaultEnd : sequence;
  if (m_finishSequence == effective)
    return false;

  m_finishSequence = effective;
  return true;
}

/**
 * @brief Sets the checksum algorithm; true when it moved.
 */
bool IO::DeviceIoRouter::setChecksumAlgorithm(const QString& algorithm)
{
  if (m_checksumAlgorithm == algorithm)
    return false;

  m_checksumAlgorithm = algorithm;
  return true;
}

/**
 * @brief Returns the configured frame start delimiter.
 */
const QByteArray& IO::DeviceIoRouter::startSequence() const noexcept
{
  return m_startSequence;
}

/**
 * @brief Returns the configured frame end delimiter.
 */
const QByteArray& IO::DeviceIoRouter::finishSequence() const noexcept
{
  return m_finishSequence;
}

/**
 * @brief Returns the name of the active checksum algorithm.
 */
const QString& IO::DeviceIoRouter::checksumAlgorithm() const noexcept
{
  return m_checksumAlgorithm;
}
