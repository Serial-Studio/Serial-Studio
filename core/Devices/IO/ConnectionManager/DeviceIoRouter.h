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

#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <span>
#include <unordered_map>

#include "Core/IO/HAL_Driver.h"
#include "Core/SerialStudio.h"

namespace IO {

class DeviceManager;
class FileTransmission;
class IIngestBinder;
class IRawByteTap;
class ReplyCapture;

/**
 * @brief Everything that crosses the device link and how it is framed (spec 0075, C14): the
 *        delimiters and checksum the readers are rebuilt from, the inbound chunk fan-out to the
 *        root-bound raw taps and the ingest binder (spec 0077), and the outbound write path with
 *        its reply capture. Runs on the ConnectionManager's thread only.
 */
class DeviceIoRouter {
public:
  using DeviceTable = std::unordered_map<int, std::unique_ptr<DeviceManager>>;

  static constexpr std::size_t kMaxRawTaps = 6;

  DeviceIoRouter(IIngestBinder& binder,
                 ReplyCapture& replyCapture,
                 const DeviceTable& devices,
                 const std::atomic<bool>& paused,
                 FileTransmission* const& fileTransmission);

  DeviceIoRouter(DeviceIoRouter&&)                 = delete;
  DeviceIoRouter(const DeviceIoRouter&)            = delete;
  DeviceIoRouter& operator=(DeviceIoRouter&&)      = delete;
  DeviceIoRouter& operator=(const DeviceIoRouter&) = delete;

  void bindRawTaps(std::span<IRawByteTap* const> taps);

  void onRawDataReceived(int deviceId, const CapturedDataPtr& data);
  void onConsoleDataReceived(int deviceId, const CapturedDataPtr& data);
  void processPayload(const QByteArray& payload);
  void processMultiSourcePayload(const QByteArray& fullPayload,
                                 const QMap<int, QByteArray>& sourcePayloads);
  void disarmReplyCapture(int deviceId);

  [[nodiscard]] qint64 writeToDevice(int deviceId, const QByteArray& data);
  [[nodiscard]] qint64 writeAndArmReply(int deviceId, const QByteArray& data);
  [[nodiscard]] QByteArray pollReplyBuffer(int deviceId) const;

  [[nodiscard]] bool setStartSequence(const QByteArray& sequence);
  [[nodiscard]] bool setFinishSequence(const QByteArray& sequence);
  [[nodiscard]] bool setChecksumAlgorithm(const QString& algorithm);

  [[nodiscard]] const QByteArray& startSequence() const noexcept;
  [[nodiscard]] const QByteArray& finishSequence() const noexcept;
  [[nodiscard]] const QString& checksumAlgorithm() const noexcept;

private:
  void tapInjectedBytes(const CapturedDataPtr& data);

private:
  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QString m_checksumAlgorithm;

  IIngestBinder& m_binder;
  ReplyCapture& m_replyCapture;
  const DeviceTable& m_devices;

  // Binds the facade's members, which the composition root fills after construction
  const std::atomic<bool>& m_paused;
  FileTransmission* const& m_fileTransmission;

  // Bound once by the root before the first open; iterated per chunk, never resized after
  std::size_t m_tapCount;
  std::array<IRawByteTap*, kMaxRawTaps> m_taps;
};

}  // namespace IO
