/*
 * qmqtt_router.h - qmqtt router
 *
 * Copyright (c) 2013  Ery Lee <ery.lee at gmail dot com>
 * Router added by Niklas Wulf <nwulf at geenen-it-systeme dot de>
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
#ifndef QMQTT_ROUTESUBSCRIPTION_H
#define QMQTT_ROUTESUBSCRIPTION_H

#include <qmqtt_global.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QRegularExpression>
#include <QStringList>

namespace QMQTT
{

class Client;
class Message;
class RoutedMessage;
class Router;

class Q_MQTT_EXPORT RouteSubscription : public QObject
{
  Q_OBJECT
public:
  ~RouteSubscription();

  QString route() const;

Q_SIGNALS:
  void received(const RoutedMessage &message);

private slots:
  void routeMessage(const Message &message);

private:
  friend class Router;

  explicit RouteSubscription(Router *parent = nullptr);
  void setRoute(const QString &route);

  QPointer<Client> _client;
  QString _topic;
  QRegularExpression _regularExpression;
  QStringList _parameterNames;
};

} // namespace QMQTT

#endif // QMQTT_ROUTESUBSCRIPTION_H
