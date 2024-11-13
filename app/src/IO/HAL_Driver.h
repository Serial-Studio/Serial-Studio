/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QObject>
#include <QIODevice>

namespace IO
{
/**
 * @class HAL_Driver
 * @brief Abstract base class for hardware abstraction layer (HAL) drivers.
 *
 * The `HAL_Driver` class provides a unified interface for I/O operations and
 * data access across different types of devices (e.g., serial ports, network
 * connections, Bluetooth). Subclasses implement protocol-specific
 * functionality, allowing the rest of the I/O module to interact with devices
 * without requiring knowledge of protocol details.
 *
 * Key Features:
 * - Abstracts protocol-specific implementation details.
 * - Provides a common interface for opening, closing, and writing to devices.
 * - Supports signals for configuration changes, data sent, and data received.
 * - Ensures flexibility and extensibility for supporting new device types.
 *
 * Subclasses should override all pure virtual functions to provide
 * device-specific implementations.
 *
 * @signals
 * - `configurationChanged()`: Emitted when the driver's configuration changes.
 * - `dataSent(const QByteArray &data)`: Emitted when data is successfully
 *   written to the device.
 * - `dataReceived(const QByteArray &data)`: Emitted when data is received from
 *   the device.
 */
class HAL_Driver : public QObject
{
  Q_OBJECT

signals:
  void configurationChanged();
  void dataSent(const QByteArray &data);
  void dataReceived(const QByteArray &data);

public:
  virtual void close() = 0;
  virtual bool isOpen() const = 0;
  virtual bool isReadable() const = 0;
  virtual bool isWritable() const = 0;
  virtual bool configurationOk() const = 0;
  virtual quint64 write(const QByteArray &data) = 0;
  virtual bool open(const QIODevice::OpenMode mode) = 0;
};
} // namespace IO
