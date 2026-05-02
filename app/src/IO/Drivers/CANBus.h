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

#include <QByteArray>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QObject>
#include <QSettings>
#include <QString>

#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {
/**
 * @brief HAL driver for CAN and CAN FD interfaces via Qt QCanBus.
 */
class CANBus : public HAL_Driver {
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
  void connectionError(const QString& error);

public:
  explicit CANBus();
  ~CANBus();

  CANBus(CANBus&&)                 = delete;
  CANBus(const CANBus&)            = delete;
  CANBus& operator=(CANBus&&)      = delete;
  CANBus& operator=(const CANBus&) = delete;

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  bool selectByIdentifier(const QJsonObject& id) override;
  [[nodiscard]] QJsonObject deviceIdentifier() const override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] bool canFD() const;
  [[nodiscard]] quint8 pluginIndex() const;
  [[nodiscard]] quint8 interfaceIndex() const;
  [[nodiscard]] quint32 bitrate() const;

  [[nodiscard]] QStringList pluginList() const;
  [[nodiscard]] QStringList interfaceList() const;
  [[nodiscard]] QStringList bitrateList() const;
  [[nodiscard]] QString interfaceError() const;

  [[nodiscard]] Q_INVOKABLE QString pluginDisplayName(const QString& plugin) const;

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;
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
  void doClose();
  void refreshInterfaces();
  [[nodiscard]] bool canSupportAvailable() const;
  [[nodiscard]] QString noInterfacesHint(const QString& plugin) const;

private:
  QCanBusDevice* m_device;

  bool m_canFD;
  quint8 m_pluginIndex;
  quint8 m_interfaceIndex;
  quint32 m_bitrate;

  QSettings m_settings;
  QString m_interfaceError;
  QStringList m_pluginList;
  QStringList m_interfaceList;
};
}  // namespace Drivers
}  // namespace IO
