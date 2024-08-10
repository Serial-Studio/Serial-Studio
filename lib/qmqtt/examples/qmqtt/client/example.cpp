/*
 * example.cpp - qmqtt example
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
#include <qmqtt.h>
#include <QCoreApplication>
#include <QTimer>

const QHostAddress EXAMPLE_HOST = QHostAddress::LocalHost;
const quint16 EXAMPLE_PORT = 1883;
const QString EXAMPLE_TOPIC = "qmqtt/exampletopic";

class Publisher : public QMQTT::Client
{
  Q_OBJECT
public:
  explicit Publisher(const QHostAddress &host = EXAMPLE_HOST,
                     const quint16 port = EXAMPLE_PORT,
                     QObject *parent = nullptr)
    : QMQTT::Client(host, port, parent)
    , _number(0)
  {
    connect(this, &Publisher::connected, this, &Publisher::onConnected);
    connect(&_timer, &QTimer::timeout, this, &Publisher::onTimeout);
    connect(this, &Publisher::disconnected, this, &Publisher::onDisconnected);
  }
  virtual ~Publisher() {}

  QTimer _timer;
  quint16 _number;

public slots:
  void onConnected()
  {
    subscribe(EXAMPLE_TOPIC, 0);
    _timer.start(1000);
  }

  void onTimeout()
  {
    QMQTT::Message message(_number, EXAMPLE_TOPIC,
                           QString("Number is %1").arg(_number).toUtf8());
    publish(message);
    _number++;
    if (_number >= 10)
    {
      _timer.stop();
      disconnectFromHost();
    }
  }

  void onDisconnected()
  {
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
  }
};

class Subscriber : public QMQTT::Client
{
  Q_OBJECT
public:
  explicit Subscriber(const QHostAddress &host = EXAMPLE_HOST,
                      const quint16 port = EXAMPLE_PORT,
                      QObject *parent = nullptr)
    : QMQTT::Client(host, port, parent)
    , _qout(stdout)
  {
    connect(this, &Subscriber::connected, this, &Subscriber::onConnected);
    connect(this, &Subscriber::subscribed, this, &Subscriber::onSubscribed);
    connect(this, &Subscriber::received, this, &Subscriber::onReceived);
  }
  virtual ~Subscriber() {}

  QTextStream _qout;

public slots:
  void onConnected()
  {
    _qout << "connected" << endl;
    subscribe(EXAMPLE_TOPIC, 0);
  }

  void onSubscribed(const QString &topic)
  {
    _qout << "subscribed " << topic << endl;
  }

  void onReceived(const QMQTT::Message &message)
  {
    _qout << "publish received: \"" << QString::fromUtf8(message.payload())
          << "\"" << endl;
  }
};

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  Subscriber subscriber;
  subscriber.connectToHost();
  Publisher publisher;
  publisher.connectToHost();
  return app.exec();
}

#include "example.moc"
