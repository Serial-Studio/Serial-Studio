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
#include <QMap>
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
  void driverChanged();             ///< Active driver type changed (bus type switch or rebuild)
  void pausedChanged();             ///< Streaming paused/resumed (device stays connected)
  void busTypeChanged();            ///< Primary device bus type changed
  void busListChanged();            ///< Available bus list changed (language switch)
  void connectedChanged();          ///< Any device connected or disconnected
  void contextsRebuilt();           ///< rebuildDevices() completed (all DeviceManagers recreated)
  void writeEnabledChanged();       ///< Write capability toggled
  void configurationChanged();      ///< UI driver config changed (triggers configurationOk re-eval)
  void startSequenceChanged();      ///< Frame start delimiter changed
  void deviceListRefreshed();       ///< Hardware device enumeration updated (UART ports, BLE, etc.)
  void finishSequenceChanged();     ///< Frame end delimiter changed
  void checksumAlgorithmChanged();  ///< Checksum algorithm changed

private:
  explicit ConnectionManager();
  ConnectionManager(ConnectionManager&&)                 = delete;
  ConnectionManager(const ConnectionManager&)            = delete;
  ConnectionManager& operator=(ConnectionManager&&)      = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;

  ~ConnectionManager();

public:
  [[nodiscard]] static ConnectionManager& instance();

  // Status queries
  [[nodiscard]] bool paused() const noexcept;
  [[nodiscard]] bool readOnly() const;
  [[nodiscard]] bool readWrite() const;
  [[nodiscard]] bool isConnected() const;
  [[nodiscard]] bool configurationOk() const;
  [[nodiscard]] int connectedDeviceCount() const;

  // Primary device configuration
  [[nodiscard]] SerialStudio::BusType busType() const noexcept;

  // Frame delimiter / checksum configuration
  [[nodiscard]] const QByteArray& startSequence() const noexcept;
  [[nodiscard]] const QByteArray& finishSequence() const noexcept;
  [[nodiscard]] const QString& checksumAlgorithm() const noexcept;

  // Bus type names for QML combo box
  [[nodiscard]] QStringList availableBuses() const;

  // Driver access — live drivers (from DeviceManagers) and UI-config drivers
  [[nodiscard]] HAL_Driver* driver(int deviceId = 0) const;
  [[nodiscard]] HAL_Driver* driverForEditing(int deviceId);
  [[nodiscard]] HAL_Driver* activeUiDriver() const noexcept;
  [[nodiscard]] HAL_Driver* uiDriverForBusType(SerialStudio::BusType type) const noexcept;

  // Per-type UI-config driver accessors (used by QML context properties)
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

  // Data transmission and payload injection
  Q_INVOKABLE qint64 writeData(const QByteArray& data);
  Q_INVOKABLE qint64 writeDataToDevice(int deviceId, const QByteArray& data);
  Q_INVOKABLE void processPayload(const QByteArray& payload);
  void processMultiSourcePayload(const QByteArray& fullPayload,
                                 const QMap<int, QByteArray>& sourcePayloads);

public slots:
  // Connection lifecycle
  void connectDevice();
  void toggleConnection();
  void disconnectDevice();
  void connectAllDevices();
  void disconnectAllDevices();
  void connectDevice(int deviceId);
  void disconnectDevice(int deviceId);

  // Initialization and reconfiguration
  void resetFrameReader();
  void setupExternalConnections();

  // Property setters
  void setPaused(bool paused);
  void setWriteEnabled(bool enabled);
  void setStartSequence(const QByteArray& sequence);
  void setFinishSequence(const QByteArray& sequence);
  void setChecksumAlgorithm(const QString& algorithm);
  void setBusType(SerialStudio::BusType type);
  void setUiDriverProperty(const QString& key, const QVariant& value);

private slots:
  void rebuildDevices();                  ///< Recreates DeviceManagers from project sources
  void syncUiDriverToLive();              ///< Copies UI driver properties → live driver (device 0)
  void syncUiDriverFromSource0();         ///< Copies source[0] settings → UI driver (project load)
  void onUiDriverConfigurationChanged();  ///< Saves UI driver state → source[0].connectionSettings
  void onFrameReady(int deviceId, const QByteArray& frame);
  void onRawDataReceived(int deviceId, const IO::ByteArrayPtr& data);

private:
  // Builds FrameConfig from project settings or global delimiters
  [[nodiscard]] FrameConfig buildFrameConfig(int deviceId) const;

  // Creates a fresh driver instance (never a singleton) for live connections
  [[nodiscard]] std::unique_ptr<HAL_Driver> createDriver(SerialStudio::BusType type) const;

  // Connects DeviceManager signals (frameReady, rawDataReceived) to routing slots
  void wireDevice(DeviceManager* dm);

private:
  // State flags
  std::atomic<bool> m_paused;       ///< True = streaming paused, device stays open
  bool m_writeEnabled;              ///< False = read-only mode (no TX)
  bool m_syncingFromProject;        ///< Re-entrancy guard for project → UI sync
  SerialStudio::BusType m_busType;  ///< Active bus type for device 0

  // Frame detection parameters (global, used in non-ProjectFile modes)
  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QString m_checksumAlgorithm;

  QSettings m_settings;

  // Live devices: deviceId → DeviceManager (owns driver + thread + FrameReader)
  std::unordered_map<int, std::unique_ptr<DeviceManager>> m_devices;

  // UI-config driver instances (one per bus type, never used for live I/O).
  // QML context properties (Cpp_IO_Serial, etc.) point to these for
  // port/baud/device enumeration. On connect, a FRESH driver is created
  // via createDriver() and the UI driver's properties are copied into it.
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
