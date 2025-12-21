/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QByteArray>
#include <QCanBusDevice>
#include <QCanBusFrame>

#include "IO/HAL_Driver.h"

namespace IO
{
namespace Drivers
{
/**
 * @class CANBus
 * @brief Serial Studio driver for CAN bus communication
 *
 * This class provides a complete implementation of the CAN bus protocol driver
 * for Serial Studio, supporting various CAN hardware interfaces through Qt's
 * QCanBus plugin system.
 *
 * ## Supported Hardware
 * Platform-specific CAN interface support:
 * - **Linux**: SocketCAN (kernel module)
 * - **Windows**: PEAK PCAN, Vector CAN, Systec CAN
 * - **macOS**: Limited support (requires third-party drivers)
 *
 * ## CAN Frame Format
 * The driver handles both standard and extended CAN frames:
 * - **Standard CAN**: 11-bit identifier (0x000-0x7FF)
 * - **Extended CAN (CAN FD)**: 29-bit identifier (0x00000000-0x1FFFFFFF)
 * - **Data Length**: 0-8 bytes (standard) or 0-64 bytes (CAN FD)
 *
 * ## Connection Lifecycle
 * The driver implements an event-driven model:
 * 1. Discovers available CAN plugins (socketcan, peakcan, etc.)
 * 2. User selects plugin and interface (can0, can1, etc.)
 * 3. `open()` creates and configures CAN device
 * 4. Device connects and transitions to Connected state
 * 5. `framesReceived` signal triggers data reception
 *
 * ## Data Flow
 * - Receives CAN frames via `onFramesReceived()` event handler
 * - Converts CAN frames to byte arrays: [ID_high, ID_low, DLC, data...]
 * - Emits `dataReceived()` signal for Serial Studio's frame parser
 * - Supports bidirectional communication (read/write)
 *
 * ## Platform Detection
 * The driver automatically detects available CAN plugins and provides
 * platform-specific error messages when hardware support is unavailable.
 *
 * ## Signal Integration
 * - `configurationChanged()`: Emitted when connection state or config changes
 * - `dataReceived()`: Emitted when CAN frames are received
 * - `connectionError()`: Emitted when connection or frame errors occur
 * - `availablePluginsChanged()`: Emitted when plugin list updates
 * - `availableInterfacesChanged()`: Emitted when interface list updates
 *
 * @see HAL_Driver
 * @see IO::Manager
 * @see QCanBusDevice
 */
class CANBus : public HAL_Driver
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(quint8 pluginIndex
             READ pluginIndex
             WRITE setPluginIndex
             NOTIFY pluginIndexChanged)
  Q_PROPERTY(quint8 interfaceIndex
             READ interfaceIndex
             WRITE setInterfaceIndex
             NOTIFY interfaceIndexChanged)
  Q_PROPERTY(quint32 bitrate
             READ bitrate
             WRITE setBitrate
             NOTIFY bitrateChanged)
  Q_PROPERTY(bool canFD
             READ canFD
             WRITE setCanFD
             NOTIFY canFDChanged)
  Q_PROPERTY(QStringList pluginList
             READ pluginList
             NOTIFY availablePluginsChanged)
  Q_PROPERTY(QStringList interfaceList
             READ interfaceList
             NOTIFY availableInterfacesChanged)
  Q_PROPERTY(QStringList bitrateList
             READ bitrateList
             CONSTANT)
  Q_PROPERTY(QString interfaceError
             READ interfaceError
             NOTIFY interfaceErrorChanged)
  // clang-format on

signals:
  void canFDChanged();
  void bitrateChanged();
  void pluginIndexChanged();
  void interfaceIndexChanged();
  void availablePluginsChanged();
  void availableInterfacesChanged();
  void interfaceErrorChanged();
  void connectionError(const QString &error);

private:
  explicit CANBus();
  CANBus(CANBus &&) = delete;
  CANBus(const CANBus &) = delete;
  CANBus &operator=(CANBus &&) = delete;
  CANBus &operator=(const CANBus &) = delete;

  ~CANBus();

public:
  static CANBus &instance();

  void close() override;

  [[nodiscard]] bool isOpen() const override;
  [[nodiscard]] bool isReadable() const override;
  [[nodiscard]] bool isWritable() const override;
  [[nodiscard]] bool configurationOk() const override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] bool canFD() const;
  [[nodiscard]] quint8 pluginIndex() const;
  [[nodiscard]] quint8 interfaceIndex() const;
  [[nodiscard]] quint32 bitrate() const;

  [[nodiscard]] QStringList pluginList() const;
  [[nodiscard]] QStringList interfaceList() const;
  [[nodiscard]] QStringList bitrateList() const;
  [[nodiscard]] QString interfaceError() const;

  Q_INVOKABLE QString pluginDisplayName(const QString &plugin) const;

public slots:
  void setupExternalConnections();
  void setCanFD(const bool enabled);
  void setBitrate(const quint32 bitrate);
  void setPluginIndex(const quint8 index);
  void setInterfaceIndex(const quint8 index);

private slots:
  void refreshPlugins();
  void onFramesReceived();
  void onStateChanged(QCanBusDevice::CanBusDeviceState state);
  void onErrorOccurred(QCanBusDevice::CanBusError error);

private:
  void refreshInterfaces();
  [[nodiscard]] bool canSupportAvailable() const;

private:
  QCanBusDevice *m_device;

  bool m_canFD;
  quint8 m_pluginIndex;
  quint8 m_interfaceIndex;
  quint32 m_bitrate;

  QSettings m_settings;
  QString m_interfaceError;
  QStringList m_pluginList;
  QStringList m_interfaceList;
};
} // namespace Drivers
} // namespace IO
