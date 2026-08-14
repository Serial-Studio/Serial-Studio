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
#include <QHash>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QTimer>
#include <unordered_map>

#include "IO/DeviceManager.h"
#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "IO/HAL_Driver.h"
#include "IO/StreamWorker.h"
#include "SerialStudio.h"

class SessionContext;

namespace DataModel {
struct Source;
}  // namespace DataModel

namespace Misc::Diagnostics {
enum class Bus : int;
}  // namespace Misc::Diagnostics

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "IO/Drivers/CANBus.h"
#  include "IO/Drivers/HID.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/MQTT.h"
#  include "IO/Drivers/Process.h"
#  include "IO/Drivers/USB.h"
#endif

namespace IO {

/**
 * @brief Link counters summed across every open device, sampled once per second by the problem
 *        center. Readers are recreated on connect/reconfigure, so a decrease means a reset.
 */
struct LinkStats {
  quint64 bytesIn;
  quint64 droppedFrames;
  quint64 overflowBytes;
  quint64 checksumErrors;
  quint64 framesExtracted;
};

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
  [[nodiscard]] IO::Drivers::Process* process() const noexcept;
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
  void onDeviceOpenFinished(int deviceId, bool ok, const QString& reason);
  void onDriverOpenFinished(bool ok, const QString& reason);

private:
  void endWaitCursor();
  void beginWaitCursor();
  void concludeConnectRequest();
  void notifyConnectedStateChanged();
  void wireUiDriver(IO::HAL_Driver* driver);
  void buildDeviceForSource(const DataModel::Source& src, bool willRebuildDevice0);

  [[nodiscard]] bool anyDeviceConnecting() const;

  [[nodiscard]] bool projectConfigurationOk() const;
  [[nodiscard]] bool diagnosticsBusFor(int deviceId, Misc::Diagnostics::Bus& bus) const;
  [[nodiscard]] std::unique_ptr<HAL_Driver> createDriver(SerialStudio::BusType type) const;
  [[nodiscard]] QString streamLaneForSource(int deviceId) const;
  [[nodiscard]] StreamConfig buildStreamConfig(int deviceId, HAL_Driver* driver) const;
  void publishStreamTemplates();
  void wireStreamWorkerSinks(StreamWorker& worker);
  void wireStreamLifecycle();
  void dropUnavailablePrimaryDevice(SerialStudio::BusType type);

private:
  std::atomic<bool> m_paused;
  bool m_writeEnabled;
  bool m_connectFanOut;
  bool m_connectPending;
  bool m_waitCursorActive;
  bool m_lastConnectedState;
  bool m_syncingFromProject;
  bool m_rebuildingDevices;
  bool m_lastConnectingState;
  int m_lastConnectedCount;
  SerialStudio::BusType m_busType;

  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QString m_checksumAlgorithm;

  QSettings m_settings;
  QTimer m_uiDriverSaveTimer;

  QSet<int> m_pendingDialVerdicts;
  std::unordered_map<int, std::unique_ptr<DeviceManager>> m_devices;
  std::vector<std::unique_ptr<StreamWorker>> m_streamWorkers;

  std::atomic<bool> m_replyCaptureArmed;
  mutable QMutex m_replyMutex;
  QHash<int, QByteArray> m_replyBuffers;

  std::unique_ptr<IO::Drivers::UART> m_uartUi;
  std::unique_ptr<IO::Drivers::Network> m_networkUi;
  std::unique_ptr<IO::Drivers::BluetoothLE> m_bluetoothLEUi;
#ifdef BUILD_COMMERCIAL
  std::unique_ptr<IO::Drivers::Audio> m_audioUi;
  std::unique_ptr<IO::Drivers::CANBus> m_canBusUi;
  std::unique_ptr<IO::Drivers::HID> m_hidUi;
  std::unique_ptr<IO::Drivers::MQTT> m_mqttUi;
  std::unique_ptr<IO::Drivers::Modbus> m_modbusUi;
  std::unique_ptr<IO::Drivers::Process> m_processUi;
  std::unique_ptr<IO::Drivers::USB> m_usbUi;
#endif
};

}  // namespace IO
