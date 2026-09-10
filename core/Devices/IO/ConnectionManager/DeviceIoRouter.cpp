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

#include "Core/IO/HAL_Driver.h"
#include "Core/IO/IIngestBinder.h"
#include "Core/IO/IRawByteTap.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "IO/ConnectionManager/ReplyCapture.h"
#include "IO/DeviceManager.h"
#include "IO/FileTransmission.h"

static const QByteArray kDefaultStart = QByteArray("/*");
static const QByteArray kDefaultEnd   = QByteArray("*/");

/**
 * @brief Binds the collaborators the payload and write paths reach; the file-transmission pointer
 *        is bound BY REFERENCE because the composition root fills it after this.
 */
IO::DeviceIoRouter::DeviceIoRouter(IIngestBinder& binder,
                                   ReplyCapture& replyCapture,
                                   const DeviceTable& devices,
                                   const std::atomic<bool>& paused,
                                   FileTransmission* const& fileTransmission)
  : m_startSequence(kDefaultStart)
  , m_finishSequence(kDefaultEnd)
  , m_binder(binder)
  , m_replyCapture(replyCapture)
  , m_devices(devices)
  , m_paused(paused)
  , m_fileTransmission(fileTransmission)
  , m_tapCount(0)
  , m_taps{}
{}

/**
 * @brief Adopts the raw-byte observers the root resolved, in the order they are to see each chunk.
 *        Bound once, before the first device opens; a null entry is skipped, an overflow is
 *        truncated and reported rather than grown, because the per-chunk loop is fixed-bound.
 */
void IO::DeviceIoRouter::bindRawTaps(std::span<IRawByteTap* const> taps)
{
  SS_ASSERT_LOG(m_tapCount == 0);
  SS_ASSERT_LOG(taps.size() <= kMaxRawTaps);

  m_tapCount = 0;
  m_taps.fill(nullptr);
  for (auto* tap : taps) {
    if (!tap || m_tapCount >= kMaxRawTaps)
      continue;

    m_taps[m_tapCount++] = tap;
  }
}

//--------------------------------------------------------------------------------------------------
// Inbound payloads
//--------------------------------------------------------------------------------------------------

/**
 * @brief Feeds one injected payload to the taps that observe that lane (the API servers and the
 *        console). Never the recording sinks, which see blocks and not bytes.
 */
void IO::DeviceIoRouter::tapInjectedBytes(const CapturedDataPtr& data)
{
  SS_ASSERT(data != nullptr, return);

  for (std::size_t i = 0; i < m_tapCount; ++i)
    m_taps[i]->onInjectedBytes(data);
}

/**
 * @brief Forwards raw bytes from device @p deviceId to every bound tap and to the file-transfer
 *        facade while a transfer runs, and feeds the reply capture while it is armed.
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

  if (m_fileTransmission && m_fileTransmission->active()) [[unlikely]]
    m_fileTransmission->onRawDataReceived(data->data);

  for (std::size_t i = 0; i < m_tapCount; ++i)
    m_taps[i]->onDeviceBytes(deviceId, data);
}

/**
 * @brief Forwards a stream-lane source's terminal-only bytes from device @p deviceId to the taps
 *        that show text (the console). The typed sample blocks already fed the dashboard, the
 *        exports and the API, so this text stops at the terminal and nothing is recorded twice.
 */
void IO::DeviceIoRouter::onConsoleDataReceived(int deviceId, const CapturedDataPtr& data)
{
  SS_ASSERT(data != nullptr, return);
  SS_ASSERT_LOG(deviceId >= 0);

  if (m_paused)
    return;

  for (std::size_t i = 0; i < m_tapCount; ++i)
    m_taps[i]->onConsoleBytes(deviceId, data);
}

/**
 * @brief Feeds a pre-built payload into the frame pipeline through the binder (queued to the
 *        pipeline thread; command-rate, never per frame) while the taps see it on this thread.
 */
void IO::DeviceIoRouter::processPayload(const QByteArray& payload)
{
  if (payload.isEmpty())
    return;

  const auto captured = makeCapturedData(payload);
  tapInjectedBytes(captured);
  m_binder.injectPayload(0, captured);
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

  const auto captured = makeCapturedData(fullPayload);
  tapInjectedBytes(captured);

  for (auto it = sourcePayloads.constBegin(); it != sourcePayloads.constEnd(); ++it) {
    const int sourceId  = it.key();
    const auto srcChunk = makeCapturedData(it.value(), captured->timestamp);
    m_binder.injectPayload(sourceId, srcChunk);
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
    for (std::size_t i = 0; i < m_tapCount; ++i)
      m_taps[i]->onSentBytes(deviceId, writtenData);
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
