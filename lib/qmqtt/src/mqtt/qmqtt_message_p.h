/*
 * qmqtt_message.h - qmqtt message private header
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
#ifndef QMQTT_MESSAGE_P_H
#define QMQTT_MESSAGE_P_H

#include <QSharedData>
#include <QString>
#include <QByteArray>

namespace QMQTT
{

class MessagePrivate : public QSharedData
{
public:
  inline MessagePrivate()
    : QSharedData()
    , id(0)
    , qos(0)
    , retain(false)
    , dup(false)
  {
  }

  inline MessagePrivate(const MessagePrivate &other)
    : QSharedData(other)
    , id(other.id)
    , qos(other.qos)
    , retain(other.retain)
    , dup(other.dup)
    , topic(other.topic)
    , payload(other.payload)
  {
  }

  inline MessagePrivate(quint16 id, const QString &topic,
                        const QByteArray &payload, quint8 qos, bool retain,
                        bool dup)
    : QSharedData()
    , id(id)
    , qos(qos)
    , retain(retain)
    , dup(dup)
    , topic(topic)
    , payload(payload)
  {
  }

  quint16 id;
  quint8 qos : 2;
  quint8 retain : 1;
  quint8 dup : 1;
  QString topic;
  QByteArray payload;
};

} // namespace QMQTT

#endif // QMQTT_MESSAGE_P_H
