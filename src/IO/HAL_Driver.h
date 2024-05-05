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
 * @brief The HAL_Driver class
 *
 * Abstract class that defines common I/O and data access functions for
 * different types of devices. I/O drivers (e.g. serial port & network) are
 * sub-classes of this class.
 *
 * This allows the rest of the I/O module to interact with a wide range of
 * devices and protocols without the need of understanding protocol-specific
 * implementation details.
 */
class HAL_Driver : public QObject
{
  Q_OBJECT

Q_SIGNALS:
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
