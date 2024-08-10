/*
 * qmqtt_message.h - qmqtt message header
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
#ifndef QMQTT_MESSAGE_H
#define QMQTT_MESSAGE_H

#include <qmqtt_global.h>

#include <QMetaType>
#include <QString>
#include <QByteArray>
#include <QSharedDataPointer>

namespace QMQTT
{

class MessagePrivate;

class Q_MQTT_EXPORT Message
{
public:
  Message();
  explicit Message(const quint16 id, const QString &topic,
                   const QByteArray &payload, const quint8 qos = 0,
                   const bool retain = false, const bool dup = false);
  Message(const Message &other);
  ~Message();

  Message &operator=(const Message &other);
#ifdef Q_COMPILER_RVALUE_REFS
  inline Message &operator=(Message &&other) Q_DECL_NOTHROW
  {
    swap(other);
    return *this;
  }
#endif

  bool operator==(const Message &other) const;
  inline bool operator!=(const Message &other) const
  {
    return !operator==(other);
  }

  inline void swap(Message &other) Q_DECL_NOTHROW { qSwap(d, other.d); }

  quint16 id() const;
  void setId(const quint16 id);

  quint8 qos() const;
  void setQos(const quint8 qos);

  bool retain() const;
  void setRetain(const bool retain);

  bool dup() const;
  void setDup(const bool dup);

  QString topic() const;
  void setTopic(const QString &topic);

  QByteArray payload() const;
  void setPayload(const QByteArray &payload);

private:
  QSharedDataPointer<MessagePrivate> d;
};

} // namespace QMQTT

Q_DECLARE_SHARED(QMQTT::Message)

Q_DECLARE_METATYPE(QMQTT::Message)

#endif // QMQTT_MESSAGE_H
