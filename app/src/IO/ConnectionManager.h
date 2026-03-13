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

#pragma once

#include <atomic>
#include <memory>
#include <QObject>
#include <QSettings>
#include <unordered_map>

#include "IO/DeviceManager.h"
#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "IO/HAL_Driver.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "IO/Drivers/CANBus.h"
#  include "IO/Drivers/HID.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/Process.h"
#  include "IO/Drivers/USB.h"
#endif

namespace IO {

/**
 * @brief Singleton orchestrator that owns all DeviceManager instances and wires
 *        them to FrameBuilder, Console, and the API server.
 *
 * ConnectionManager replaces both IO::Manager and IO::SourceManager. It exposes
 * the same Q_PROPERTYs and signals as the old IO::Manager so that all existing
 * QML bindings continue to work without modification (the context property
 * "Cpp_IO_Manager" is pointed at this class instead).
 *
 * Device 0 is the primary (UI-configured) device. Devices 1+ correspond to
 * additional project sources. All devices are owned as DeviceManager instances
 * inside m_devices.
 *
 * Drivers are NEVER singletons for connection purposes. Driver singletons
 * (e.g. UART::instance()) are used ONLY for device enumeration.
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
  Q_PROPERTY(bool paused
             READ paused
             WRITE setPaused
             NOTIFY pausedChanged)
  Q_PROPERTY(bool threadedFrameExtraction
             READ threadedFrameExtraction
             WRITE setThreadedFrameExtraction
             NOTIFY threadedFrameExtractionChanged)
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
  // clang-format on

signals:
  void driverChanged();
  void pausedChanged();
  void busTypeChanged();
  void busListChanged();
  void connectedChanged();
  void contextsRebuilt();
  void writeEnabledChanged();
  void configurationChanged();
  void startSequenceChanged();
  void deviceListRefreshed();
  void finishSequenceChanged();
  void checksumAlgorithmChanged();
  void threadedFrameExtractionChanged();

private:
  explicit ConnectionManager();
  ConnectionManager(ConnectionManager&&)                 = delete;
  ConnectionManager(const ConnectionManager&)            = delete;
  ConnectionManager& operator=(ConnectionManager&&)      = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;

  ~ConnectionManager();

public:
  static ConnectionManager& instance();

  [[nodiscard]] bool paused() const noexcept;
  [[nodiscard]] bool readOnly() const;
  [[nodiscard]] bool readWrite() const;
  [[nodiscard]] bool isConnected() const;
  [[nodiscard]] bool configurationOk() const;
  [[nodiscard]] bool threadedFrameExtraction() const noexcept;

  [[nodiscard]] SerialStudio::BusType busType() const noexcept;

  [[nodiscard]] const QByteArray& startSequence() const noexcept;
  [[nodiscard]] const QByteArray& finishSequence() const noexcept;
  [[nodiscard]] const QString& checksumAlgorithm() const noexcept;

  [[nodiscard]] QStringList availableBuses() const;

  [[nodiscard]] HAL_Driver* driver(int deviceId = 0) const;
  [[nodiscard]] HAL_Driver* driverForEditing(int deviceId);
  [[nodiscard]] HAL_Driver* activeUiDriver() const noexcept;

  [[nodiscard]] IO::Drivers::UART* uart() const noexcept;
  [[nodiscard]] IO::Drivers::Network* network() const noexcept;
  [[nodiscard]] IO::Drivers::BluetoothLE* bluetoothLE() const noexcept;
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] IO::Drivers::Audio* audio() const noexcept;
  [[nodiscard]] IO::Drivers::CANBus* canBus() const noexcept;
  [[nodiscard]] IO::Drivers::HID* hid() const noexcept;
  [[nodiscard]] IO::Drivers::Modbus* modbus() const noexcept;
  [[nodiscard]] IO::Drivers::Process* process() const noexcept;
  [[nodiscard]] IO::Drivers::USB* usb() const noexcept;
#endif

  Q_INVOKABLE qint64 writeData(const QByteArray& data);
  Q_INVOKABLE qint64 writeDataToDevice(int deviceId, const QByteArray& data);
  Q_INVOKABLE void processPayload(const QByteArray& payload);

public slots:
  void connectDevice();
  void toggleConnection();
  void disconnectDevice();
  void resetFrameReader();
  void setupExternalConnections();
  void connectAllDevices();
  void disconnectAllDevices();
  void connectDevice(int deviceId);
  void disconnectDevice(int deviceId);
  void setPaused(bool paused);
  void setWriteEnabled(bool enabled);
  void setStartSequence(const QByteArray& sequence);
  void setFinishSequence(const QByteArray& sequence);
  void setChecksumAlgorithm(const QString& algorithm);
  void setThreadedFrameExtraction(bool enabled);
  void setBusType(SerialStudio::BusType type);
  void setUiDriverProperty(const QString& key, const QVariant& value);

private slots:
  void rebuildDevices();
  void syncUiDriverToLive();
  void syncUiDriverFromSource0();
  void onUiDriverConfigurationChanged();
  void onFrameReady(int deviceId, const QByteArray& frame);
  void onRawDataReceived(int deviceId, const IO::ByteArrayPtr& data);

private:
  [[nodiscard]] FrameConfig buildFrameConfig(int deviceId) const;
  [[nodiscard]] std::unique_ptr<HAL_Driver> createDriver(SerialStudio::BusType type) const;
  [[nodiscard]] HAL_Driver* uiDriverForBusType(SerialStudio::BusType type) const noexcept;
  void wireDevice(DeviceManager* dm);

private:
  std::atomic<bool> m_paused;
  bool m_writeEnabled;
  bool m_thrFrameExtr;
  bool m_syncingFromProject;
  SerialStudio::BusType m_busType;

  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QString m_checksumAlgorithm;

  QSettings m_settings;

  std::unordered_map<int, DeviceManager*> m_devices;

  // UI-config driver instances (one per type; never used for live connections)
  std::unique_ptr<IO::Drivers::UART> m_uartUi;
  std::unique_ptr<IO::Drivers::Network> m_networkUi;
  std::unique_ptr<IO::Drivers::BluetoothLE> m_bluetoothLEUi;
#ifdef BUILD_COMMERCIAL
  std::unique_ptr<IO::Drivers::Audio> m_audioUi;
  std::unique_ptr<IO::Drivers::CANBus> m_canBusUi;
  std::unique_ptr<IO::Drivers::HID> m_hidUi;
  std::unique_ptr<IO::Drivers::Modbus> m_modbusUi;
  std::unique_ptr<IO::Drivers::Process> m_processUi;
  std::unique_ptr<IO::Drivers::USB> m_usbUi;
#endif
};

}  // namespace IO
