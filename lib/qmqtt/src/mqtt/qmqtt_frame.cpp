/*
 * qmqtt_frame.cpp - qmqtt frame
 *
 * Copyright (c) 2013  Ery Lee <ery.lee at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of mqttc nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "qmqtt_frame.h"

#include <QLoggingCategory>
#include <QDataStream>

namespace QMQTT
{

Q_LOGGING_CATEGORY(frame, "qmqtt.frame")

Frame::Frame()
  : _header(0)
  , _data(QByteArray())
{
}

Frame::Frame(const quint8 header)
  : _header(header)
  , _data(QByteArray())
{
}

Frame::Frame(const quint8 header, const QByteArray &data)
  : _header(header)
  , _data(data)
{
}

Frame::Frame(const Frame &other)
{
  _header = other._header;
  _data = other._data;
}

Frame &Frame::operator=(const Frame &other)
{
  _header = other._header;
  _data = other._data;
  return *this;
}

bool Frame::operator==(const Frame &other) const
{
  return _header == other._header && _data == other._data;
}

Frame::~Frame() {}

quint8 Frame::header() const
{
  return _header;
}

QByteArray Frame::data() const
{
  return _data;
}

quint8 Frame::readChar()
{
  char c = _data.at(0);
  _data.remove(0, 1);
  return c;
}

quint16 Frame::readInt()
{
  quint8 msb = static_cast<quint8>(_data.at(0));
  quint8 lsb = static_cast<quint8>(_data.at(1));
  _data.remove(0, 2);
  return (msb << 8) | lsb;
}

QByteArray Frame::readByteArray()
{
  quint16 len = readInt();
  QByteArray data = _data.left(len);
  _data.remove(0, len);
  return data;
}

QString Frame::readString()
{
  quint16 len = readInt();
  QString s = QString::fromUtf8(_data.left(len));
  _data.remove(0, len);
  return s;
}

void Frame::writeInt(const quint16 i)
{
  _data.append(MSB(i));
  _data.append(LSB(i));
}

void Frame::writeByteArray(const QByteArray &data)
{
  if (data.size() > (int)USHRT_MAX)
  {
    qCritical("qmqtt: Binary data size bigger than %u bytes, truncate it!",
              USHRT_MAX);
    writeInt(USHRT_MAX);
    _data.append(data.left(USHRT_MAX));
    return;
  }

  writeInt(data.size());
  _data.append(data);
}

void Frame::writeString(const QString &string)
{
  QByteArray data = string.toUtf8();
  if (data.size() > (int)USHRT_MAX)
  {
    qCritical("qmqtt: String size bigger than %u bytes, truncate it!",
              USHRT_MAX);
    data.resize(USHRT_MAX);
  }
  writeInt(data.size());
  _data.append(data);
}

void Frame::writeChar(const quint8 c)
{
  _data.append(c);
}

void Frame::writeRawData(const QByteArray &data)
{
  _data.append(data);
}

void Frame::write(QDataStream &stream) const
{
  QByteArray lenbuf;

  if (!encodeLength(lenbuf, _data.size()))
  {
    qCritical("qmqtt: Control packet bigger than 256 MB, dropped!");
    return;
  }

  stream << (quint8)_header;
  if (_data.size() == 0)
  {
    stream << (quint8)0;
    return;
  }
  if (stream.writeRawData(lenbuf.data(), lenbuf.size()) != lenbuf.size())
  {
    qCritical("qmqtt: Control packet write error!");
    return;
  }
  if (stream.writeRawData(_data.data(), _data.size()) != _data.size())
  {
    qCritical("qmqtt: Control packet write error!");
  }
}

bool Frame::encodeLength(QByteArray &lenbuf, int length) const
{
  lenbuf.clear();
  quint8 d;
  do
  {
    d = length % 128;
    length /= 128;
    if (length > 0)
    {
      d |= 0x80;
    }
    lenbuf.append(d);
  } while (length > 0);

  return lenbuf.size() <= 4;
}

} // namespace QMQTT
