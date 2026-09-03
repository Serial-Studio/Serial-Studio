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

#pragma once

#include <atomic>
#include <memory>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QTimer>
#include <unordered_map>
#include <vector>

#include "IO/ConnectionManager/ConnectFanOut.h"
#include "IO/ConnectionManager/DeviceIoRouter.h"
#include "IO/ConnectionManager/DeviceTableQuery.h"
#include "IO/ConnectionManager/DriverFactory.h"
#include "IO/ConnectionManager/DriverUiRegistry.h"
#include "IO/ConnectionManager/ReplyCapture.h"
#include "IO/ConnectionManager/StreamConfigBuilder.h"
#include "IO/ConnectionManager/StreamWorkerPool.h"
#include "IO/ConnectionManager/UiDriverSync.h"
#include "IO/DeviceManager.h"
#include "IO/HAL_Driver.h"
#include "IO/StreamWorker.h"
#include "SerialStudio.h"

class AppState;
class SessionContext;

namespace API {
class Server;
}  // namespace API

namespace Console {
class Handler;
}  // namespace Console

namespace DataModel {
struct Source;
class FrameBuilder;
class ProjectModel;
}  // namespace DataModel

namespace Misc::Diagnostics {
enum class Bus : int;
}  // namespace Misc::Diagnostics

#ifdef BUILD_COMMERCIAL
namespace MQTT {
class Publisher;
}  // namespace MQTT

namespace Sessions {
class Export;
}  // namespace Sessions
#endif

#ifdef ENABLE_GRPC
namespace API::GRPC {
class GRPCServer;
}  // namespace API::GRPC
#endif

namespace IO {

class FileTransmission;

/**
 * @brief Singleton orchestrator that owns all DeviceManager instances and wires
 *        them to FrameBuilder, Console, and the API server.
 */
class ConnectionManager : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool readOnly
             READ readOnly
             NOTIFY connectedChanged)
  Q_PROPERTY(bool readWrite
             READ readWrite
             NOTIFY connectedChanged)
  Q_PROPERTY(bool isConnected
             READ isConnected
             NOTIFY connectedChanged)
  Q_PROPERTY(bool isConnecting
             READ isConnecting
             NOTIFY connectingChanged)
  Q_PROPERTY(bool paused
             READ paused
             WRITE setPaused
             NOTIFY pausedChanged)
  Q_PROPERTY(SerialStudio::BusType busType
             READ busType
             WRITE setBusType
             NOTIFY busTypeChanged)
  Q_PROPERTY(QByteArray startSequence
             READ startSequence
             WRITE setStartSequence
             NOTIFY startSequenceChanged)
  Q_PROPERTY(QByteArray finishSequence
             READ finishSequence
             WRITE setFinishSequence
             NOTIFY finishSequenceChanged)
  Q_PROPERTY(QString checksumAlgorithm
             READ checksumAlgorithm
             WRITE setChecksumAlgorithm
             NOTIFY checksumAlgorithmChanged)
  Q_PROPERTY(bool configurationOk
             READ configurationOk
             NOTIFY configurationChanged)
  Q_PROPERTY(QStringList availableBuses
             READ availableBuses
             NOTIFY busListChanged)
  Q_PROPERTY(int connectedDeviceCount
             READ connectedDeviceCount
             NOTIFY connectedChanged)
  // clang-format on

signals:
  void driverChanged();
  void pausedChanged();
  void sessionClosed();
  void busTypeChanged();
  void busListChanged();
  void connectedChanged();
  void connectingChanged();
  void contextsRebuilt();
  void writeEnabledChanged();
  void configurationChanged();
  void startSequenceChanged();
  void deviceListRefreshed();
  void finishSequenceChanged();
  void checksumAlgorithmChanged();

private:
  friend class ::SessionContext;
  explicit ConnectionManager();
  ConnectionManager(ConnectionManager&&)                 = delete;
  ConnectionManager(const ConnectionManager&)            = delete;
  ConnectionManager& operator=(ConnectionManager&&)      = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;

public:
  /**
   * @brief Whether opening a device resumes a paused session. A user connect resumes; a driver's
   *        own auto-reconnect keeps the pause, because an adapter blip is not a request to start
   *        streaming again into a session the user deliberately paused.
   */
  enum class ResumePolicy {
    Resume,
    KeepPause,
  };

  ~ConnectionManager();

  [[nodiscard]] static ConnectionManager& instance();

  [[nodiscard]] bool paused() const noexcept;
  [[nodiscard]] bool readOnly() const;
  [[nodiscard]] bool readWrite() const;
  [[nodiscard]] bool isConnected() const;
  [[nodiscard]] bool isConnecting() const;
  [[nodiscard]] bool configurationOk() const;
  [[nodiscard]] int connectedDeviceCount() const;

  [[nodiscard]] QString linkState() const;
  [[nodiscard]] LinkStats linkStats() const;
  [[nodiscard]] FrameConfig buildFrameConfig(int deviceId) const;
  [[nodiscard]] const std::vector<std::unique_ptr<StreamWorker>>& streamWorkers() const noexcept;

  [[nodiscard]] SerialStudio::BusType busType() const noexcept;

  [[nodiscard]] const QByteArray& startSequence() const noexcept;
  [[nodiscard]] const QByteArray& finishSequence() const noexcept;
  [[nodiscard]] const QString& checksumAlgorithm() const noexcept;

  [[nodiscard]] QStringList availableBuses() const;

  [[nodiscard]] HAL_Driver* driver(int deviceId = 0) const;
  [[nodiscard]] HAL_Driver* driverForEditing(int deviceId);
  [[nodiscard]] HAL_Driver* activeUiDriver() const noexcept;
  [[nodiscard]] HAL_Driver* uiDriverForBusType(SerialStudio::BusType type) const noexcept;

  [[nodiscard]] IO::Drivers::UART* uart() const noexcept;
  [[nodiscard]] IO::Drivers::Network* network() const noexcept;
  [[nodiscard]] IO::Drivers::BluetoothLE* bluetoothLE() const noexcept;
  [[nodiscard]] IO::Drivers::BluetoothLE* connectedBluetoothLE() const noexcept;
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] IO::Drivers::Audio* audio() const noexcept;
  [[nodiscard]] IO::Drivers::CANBus* canBus() const noexcept;
  [[nodiscard]] IO::Drivers::HID* hid() const noexcept;
  [[nodiscard]] IO::Drivers::MQTT* mqtt() const noexcept;
  [[nodiscard]] IO::Drivers::Modbus* modbus() const noexcept;
  [[nodiscard]] IO::Drivers::OpcUa* opcUa() const noexcept;
  [[nodiscard]] IO::Drivers::Process* process() const noexcept;
  [[nodiscard]] IO::Drivers::S7* s7() const noexcept;
  [[nodiscard]] IO::Drivers::EthernetIp* ethernetIp() const noexcept;
  [[nodiscard]] IO::Drivers::Iec104* iec104() const noexcept;
  [[nodiscard]] IO::Drivers::USB* usb() const noexcept;
#endif

  [[nodiscard]] Q_INVOKABLE qint64 writeData(const QByteArray& data);
  [[nodiscard]] Q_INVOKABLE qint64 writeDataToDevice(int deviceId, const QByteArray& data);
  [[nodiscard]] Q_INVOKABLE bool isDeviceConnected(int deviceId) const;

  [[nodiscard]] qint64 writeAndArmReply(int deviceId, const QByteArray& data);
  [[nodiscard]] QByteArray pollReplyBuffer(int deviceId) const;
  void disarmReplyCapture(int deviceId);

public slots:
  void connectDevice();
  void toggleConnection();
  void disconnectDevice();
  void connectAllDevices();
  void disconnectAllDevices();
  void shutdownDrivers();
  void connectDevice(int deviceId);
  void connectDevice(HAL_Driver* driver);
  void disconnectDevice(int deviceId);
  void disconnectDevice(HAL_Driver* driver);

  void resetFrameReader();
  void setupExternalConnections();
  void rebuildStreamWorkers();
  void refreshStreamExportFlags();
  void stopStreamWorkers();

  void setPaused(bool paused);
  void setWriteEnabled(bool enabled);
  void setStartSequence(const QByteArray& sequence);
  void setFinishSequence(const QByteArray& sequence);
  void setChecksumAlgorithm(const QString& algorithm);
  void setBusType(SerialStudio::BusType type);
  void setUiDriverProperty(const QString& key, const QVariant& value);
  void processPayload(const QByteArray& payload);

  void processMultiSourcePayload(const QByteArray& fullPayload,
                                 const QMap<int, QByteArray>& sourcePayloads);

private slots:
  void rebuildDevices();
  void onProjectSourceChanged(int sourceId);
  void syncUiDriverToLive();
  void syncUiDriverFromSource0();
  void wireDevice(DeviceManager* dm);
  void refreshConnectedState();
  void onUiDriverConfigurationChanged();
  void onRawDataReceived(int deviceId, const IO::CapturedDataPtr& data);
  void onConsoleDataReceived(int deviceId, const IO::CapturedDataPtr& data);
  void onDeviceOpenFinished(int deviceId, bool ok, const QString& reason);
  void onDriverOpenFinished(bool ok, const QString& reason);

private:
  void concludeConnectRequest();
  void notifyConnectedStateChanged();
  void openDevice(int deviceId, ResumePolicy policy);
  void wireUiDriver(IO::HAL_Driver* driver);
  void buildDeviceForSource(const DataModel::Source& src, bool willRebuildDevice0);

  [[nodiscard]] bool anyDeviceConnecting() const;

  [[nodiscard]] bool projectConfigurationOk() const;
  [[nodiscard]] int deviceIdForDriver(const HAL_Driver* driver) const;
  [[nodiscard]] std::vector<int> deviceIdSnapshot(bool projectSourcesOnly) const;
  void wireStreamLifecycle();
  void dropUnavailablePrimaryDevice(SerialStudio::BusType type);

private:
  std::atomic<bool> m_paused;
  bool m_writeEnabled;
  bool m_rebuildingDevices;
  SerialStudio::BusType m_busType;

  QSettings m_settings;
  QTimer m_uiDriverSaveTimer;

  AppState& m_appState;
  DataModel::FrameBuilder& m_frameBuilder;
  DataModel::ProjectModel& m_projectModel;

  API::Server* m_apiServer;
  Console::Handler* m_console;
  IO::FileTransmission* m_fileTransmission;
#ifdef BUILD_COMMERCIAL
  MQTT::Publisher* m_mqttPublisher;
  Sessions::Export* m_sessionExport;
#endif
#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer* m_grpcServer;
#endif

  std::unordered_map<int, std::unique_ptr<DeviceManager>> m_devices;

  ConnectFanOut m_fanOut;
  ReplyCapture m_replyCapture;
  DeviceIoRouter m_io;
  DeviceTableQuery m_query;
  DriverUiRegistry m_uiDrivers;
  DriverFactory m_driverFactory;
  StreamConfigBuilder m_streamConfigs;
  StreamWorkerPool m_streamPool;
  UiDriverSync m_uiSync;
};

}  // namespace IO
