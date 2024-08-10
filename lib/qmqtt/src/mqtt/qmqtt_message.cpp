/*
 * qmqtt_message.cpp - qmqtt message
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

#include "qmqtt_message.h"
#include "qmqtt_message_p.h"

namespace QMQTT
{

Message::Message()
  : d(new MessagePrivate)
{
}

Message::Message(const quint16 id, const QString &topic,
                 const QByteArray &payload, const quint8 qos, const bool retain,
                 const bool dup)
  : d(new MessagePrivate(id, topic, payload, qos, retain, dup))
{
}

Message::Message(const Message &other)
  : d(other.d)
{
}

Message::~Message() {}

Message &Message::operator=(const Message &other)
{
  d = other.d;
  return *this;
}

bool Message::operator==(const Message &other) const
{
  if (d == other.d)
    return true;
  return d->id == other.d->id && d->qos == other.d->qos
         && d->retain == other.d->retain && d->dup == other.d->dup
         && d->topic == other.d->topic && d->payload == other.d->payload;
}

quint16 Message::id() const
{
  return d->id;
}

void Message::setId(const quint16 id)
{
  d->id = id;
}

quint8 Message::qos() const
{
  return d->qos;
}

void Message::setQos(const quint8 qos)
{
  d->qos = qos;
}

bool Message::retain() const
{
  return d->retain;
}

void Message::setRetain(const bool retain)
{
  d->retain = retain;
}

bool Message::dup() const
{
  return d->dup;
}

void Message::setDup(const bool dup)
{
  d->dup = dup;
}

QString Message::topic() const
{
  return d->topic;
}

void Message::setTopic(const QString &topic)
{
  d->topic = topic;
}

QByteArray Message::payload() const
{
  return d->payload;
}

void Message::setPayload(const QByteArray &payload)
{
  d->payload = payload;
}

} // namespace QMQTT
