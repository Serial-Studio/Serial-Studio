/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QIODevice>

namespace IO
{
/**
 * @class HAL_Driver
 * @brief Base class for hardware abstraction layer (HAL) drivers.
 *
 * Provides a common interface for device I/O operations and data access.
 * Subclasses must implement all pure virtual methods to handle
 * protocol-specific functionality.
 *
 * Signals are available for configuration changes, data transmission, and data
 * reception.
 */
class HAL_Driver : public QObject
{
  Q_OBJECT

signals:
  void configurationChanged();
  void dataSent(const QByteArray &data);
  void dataReceived(const QByteArray &data);

public:
  /**
   * @brief Close the device connection.
   */
  virtual void close() = 0;

  [[nodiscard]] virtual bool isOpen() const = 0;
  [[nodiscard]] virtual bool isReadable() const = 0;
  [[nodiscard]] virtual bool isWritable() const = 0;
  [[nodiscard]] virtual bool configurationOk() const = 0;
  [[nodiscard]] virtual quint64 write(const QByteArray &data) = 0;
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode) = 0;

protected:
  void processData(const QByteArray &data)
  {
    QByteArray dataCopy(data);
    QMetaObject::invokeMethod(
        this, [=] { Q_EMIT dataReceived(dataCopy); }, Qt::QueuedConnection);
  }
};
} // namespace IO
