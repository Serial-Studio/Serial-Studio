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
