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

#include <atomic>
#include <memory>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <unordered_map>

#include "IO/HAL_Driver.h"

class AppState;

namespace API {
class Server;

namespace GRPC {
class GRPCServer;
}  // namespace GRPC
}  // namespace API

namespace Console {
class Handler;
}  // namespace Console

namespace DataModel {
class FrameBuilder;
}  // namespace DataModel

#ifdef BUILD_COMMERCIAL
namespace MQTT {
class Publisher;
}  // namespace MQTT

namespace Sessions {
class Export;
}  // namespace Sessions
#endif

namespace IO {

class DeviceManager;
class FileTransmission;
class ReplyCapture;

/**
 * @brief Everything that crosses the device link and how it is framed (spec 0075, C14): the
 *        delimiters and checksum the readers are rebuilt from, the inbound payload fan-in to the
 *        console, the API taps and the frame builder, and the outbound write path with its reply
 *        capture. Runs on the ConnectionManager's thread only.
 */
class DeviceIoRouter {
public:
  using DeviceTable = std::unordered_map<int, std::unique_ptr<DeviceManager>>;

  DeviceIoRouter(AppState& appState,
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
  );

  DeviceIoRouter(DeviceIoRouter&&)                 = delete;
  DeviceIoRouter(const DeviceIoRouter&)            = delete;
  DeviceIoRouter& operator=(DeviceIoRouter&&)      = delete;
  DeviceIoRouter& operator=(const DeviceIoRouter&) = delete;

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
  void tapObservers(const QByteArray& bytes);

private:
  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QString m_checksumAlgorithm;

  AppState& m_appState;
  DataModel::FrameBuilder& m_frameBuilder;
  ReplyCapture& m_replyCapture;
  const DeviceTable& m_devices;

  // Bind the facade's pointer members, which the composition root fills after construction
  const std::atomic<bool>& m_paused;
  Console::Handler* const& m_console;
  API::Server* const& m_apiServer;
  FileTransmission* const& m_fileTransmission;
#ifdef BUILD_COMMERCIAL
  Sessions::Export* const& m_sessionExport;
  MQTT::Publisher* const& m_mqttPublisher;
#endif
#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer* const& m_grpcServer;
#endif
};

}  // namespace IO
