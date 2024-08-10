/*
 * qmqtt_frame.h - qmqtt frame heaer
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
#ifndef QMQTT_FRAME_H
#define QMQTT_FRAME_H

#include <qmqtt_global.h>

#include <QMetaType>
#include <QByteArray>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QDataStream)

#define PROTOCOL_MAGIC_3_1_0 "MQIsdp"
#define PROTOCOL_MAGIC_3_1_1 "MQTT"

#define RANDOM_CLIENT_PREFIX "QMQTT-"

#define CONNECT 0x10
#define CONNACK 0x20
#define PUBLISH 0x30
#define PUBACK 0x40
#define PUBREC 0x50
#define PUBREL 0x60
#define PUBCOMP 0x70
#define SUBSCRIBE 0x80
#define SUBACK 0x90
#define UNSUBSCRIBE 0xA0
#define UNSUBACK 0xB0
#define PINGREQ 0xC0
#define PINGRESP 0xD0
#define DISCONNECT 0xE0

#define LSB(A) quint8(A & 0x00FF)
#define MSB(A) quint8((A & 0xFF00) >> 8)

/*
|--------------------------------------
| 7 6 5 4 |     3    |  2 1  | 0      |
|  Type   | DUP flag |  QoS  | RETAIN |
|--------------------------------------
*/
#define GETTYPE(HDR) (HDR & 0xF0)
#define SETQOS(HDR, Q) (HDR | ((Q) << 1))
#define GETQOS(HDR) ((HDR & 0x06) >> 1)
#define SETDUP(HDR, D) (HDR | ((D) << 3))
#define GETDUP(HDR) ((HDR & 0x08) >> 3)
#define SETRETAIN(HDR, R) (HDR | (R))
#define GETRETAIN(HDR) (HDR & 0x01)

/*
|----------------------------------------------------------------------------------
|     7    |    6     |      5     |  4   3  |     2    |       1      |     0 |
| username | password | willretain | willqos | willflag | cleansession |
reserved |
|----------------------------------------------------------------------------------
*/
#define FLAG_CLEANSESS(F, C) (F | ((C) << 1))
#define FLAG_WILL(F, W) (F | ((W) << 2))
#define FLAG_WILLQOS(F, Q) (F | ((Q) << 3))
#define FLAG_WILLRETAIN(F, R) (F | ((R) << 5))
#define FLAG_PASSWD(F, P) (F | ((P) << 6))
#define FLAG_USERNAME(F, U) (F | ((U) << 7))

namespace QMQTT
{

class Q_MQTT_EXPORT Frame
{
public:
  explicit Frame();
  explicit Frame(const quint8 header);
  explicit Frame(const quint8 header, const QByteArray &data);
  virtual ~Frame();

  Frame(const Frame &other);
  Frame &operator=(const Frame &other);

  bool operator==(const Frame &other) const;
  inline bool operator!=(const Frame &other) const
  {
    return !operator==(other);
  }

  quint8 header() const;
  QByteArray data() const;

  quint16 readInt();
  quint8 readChar();
  QByteArray readByteArray();
  QString readString();

  void writeInt(const quint16 i);
  void writeChar(const quint8 c);
  void writeByteArray(const QByteArray &data);
  void writeString(const QString &string);
  void writeRawData(const QByteArray &data);

  // TODO: FIXME LATER
  void write(QDataStream &stream) const;
  bool encodeLength(QByteArray &lenbuf, int length) const;

private:
  quint8 _header;
  QByteArray _data;
};

} // namespace QMQTT

Q_DECLARE_METATYPE(QMQTT::Frame)

#endif // QMQTT_FRAME_H
