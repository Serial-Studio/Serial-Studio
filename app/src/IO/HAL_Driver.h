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
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool ignoreDataDelimeters
             READ ignoreDataDelimeters
             WRITE setIgnoreDataDelimeters
             NOTIFY ignoreDataDelimetersChanged)
  // clang-format on

signals:
  /**
   * @brief Emitted when the driver's configuration changes.
   */
  void configurationChanged();

  /**
   * @brief Emitted when the `ignoreDataDelimeters` property changes.
   */
  void ignoreDataDelimetersChanged();

  /**
   * @brief Emitted when data is successfully written to the device.
   */
  void dataSent(const QByteArray &data);

  /**
   * @brief Emitted when data is received and should be appended to a buffer.
   */
  void dataReceived(const QByteArray &data);

  /**
   * @brief Emitted when data is received and should be processed directly.
   */
  void payloadReceived(const QByteArray &data);

public:
  /**
   * @brief Close the device connection.
   */
  virtual void close() = 0;

  /**
   * @brief Check if the device is open.
   */
  [[nodiscard]] virtual bool isOpen() const = 0;

  /**
   * @brief Check if the device is ready for reading.
   */
  [[nodiscard]] virtual bool isReadable() const = 0;

  /**
   * @brief Check if the device is ready for writing.
   */
  [[nodiscard]] virtual bool isWritable() const = 0;

  /**
   * @brief Check if the device configuration is valid.
   */
  [[nodiscard]] virtual bool configurationOk() const = 0;

  /**
   * @brief Write data to the device. Returns the number of bytes written.
   */
  [[nodiscard]] virtual quint64 write(const QByteArray &data) = 0;

  /**
   * @brief Open the device in the specified mode.
   */
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode) = 0;

  /**
   * Get the current value of the `ignoreDataDelimeters` property.
   */
  [[nodiscard]] bool ignoreDataDelimeters() const
  {
    return m_ignoreDataDelimeters;
  }

public slots:
  /**
   * Set the `ignoreDataDelimeters` property.
   * @param ignore If true, received data is processed as payload directly.
   */
  void setIgnoreDataDelimeters(const bool ignore)
  {
    m_ignoreDataDelimeters = ignore;
    Q_EMIT ignoreDataDelimetersChanged();
  }

protected:
  /**
   * Process incoming data.
   * @param data The received data to process.
   * Emits either `dataReceived` or `payloadReceived` based on the
   * `ignoreDataDelimeters` property.
   */
  void processData(const QByteArray &data)
  {
    if (!ignoreDataDelimeters())
    {
      QMetaObject::invokeMethod(
          this, [=] { Q_EMIT dataReceived(data); }, Qt::QueuedConnection);
    }
    else
    {
      QMetaObject::invokeMethod(
          this, [=] { Q_EMIT payloadReceived(data); }, Qt::QueuedConnection);
    }
  }

private:
  bool m_ignoreDataDelimeters = false;
};
} // namespace IO
