#include <qtest.h>

#include <qmqtt.h>

class tst_message : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void emptyMessage();
  void message();
  void equality_data();
  void equality();
  void copy_data();
  void copy();
  void write_data();
  void write();
  void copyOnWrite_data();
  void copyOnWrite();
};

void tst_message::emptyMessage()
{
  quint8 qos;
  bool retain;
  bool dup;
  QString topic;
  QByteArray payload;

  QBENCHMARK
  {
    QMQTT::Message message;
    qos = message.qos();
    retain = message.retain();
    dup = message.dup();
    topic = message.topic();
    payload = message.payload();
  }

  QCOMPARE(qos, quint8(0));
  QCOMPARE(retain, false);
  QCOMPARE(dup, false);
  QCOMPARE(topic, QString());
  QCOMPARE(payload, QByteArray());
}

void tst_message::message()
{
  quint8 qos;
  bool retain;
  bool dup;
  QString topic;
  QByteArray payload;

  QBENCHMARK
  {
    QMQTT::Message message(0, QStringLiteral("test/test"),
                           QByteArrayLiteral("test"), 1, true, false);
    qos = message.qos();
    retain = message.retain();
    dup = message.dup();
    topic = message.topic();
    payload = message.payload();
  }

  QCOMPARE(qos, quint8(1));
  QCOMPARE(retain, true);
  QCOMPARE(dup, false);
  QCOMPARE(topic, QStringLiteral("test/test"));
  QCOMPARE(payload, QByteArrayLiteral("test"));
}

void tst_message::equality_data()
{
  QTest::addColumn<bool>("firstRun");

  QTest::newRow("construction + first run") << true;
  QTest::newRow("subsequent runs") << false;
}

void tst_message::equality()
{
  QFETCH(bool, firstRun);

  if (firstRun)
  {
    bool equals;
    QBENCHMARK
    {
      QMQTT::Message message1(0, QStringLiteral("test/1"),
                              QByteArrayLiteral("1"), 1, true, false);
      QMQTT::Message message2(0, QStringLiteral("test/2"),
                              QByteArrayLiteral("2"), 2, false, true);
      equals = message1 == message2;
    }
    QCOMPARE(equals, false);
  }
  else
  {
    bool equals;
    QMQTT::Message message1(0, QStringLiteral("test/1"), QByteArrayLiteral("1"),
                            1, true, false);
    QMQTT::Message message2(0, QStringLiteral("test/2"), QByteArrayLiteral("2"),
                            2, false, true);
    QBENCHMARK
    {
      equals = message1 == message2;
    }
    QCOMPARE(equals, false);
  }
}

void tst_message::copy_data()
{
  QTest::addColumn<bool>("firstRun");

  QTest::newRow("construction + first run") << true;
  QTest::newRow("subsequent runs") << false;
}

void tst_message::copy()
{
  QFETCH(bool, firstRun);

  if (firstRun)
  {
    QBENCHMARK
    {
      QMQTT::Message message(0, QStringLiteral("test/test"),
                             QByteArrayLiteral("test"), 1, true, false);
      QMQTT::Message tmp;
      message = tmp;
    }
  }
  else
  {
    QMQTT::Message message(0, QStringLiteral("test/test"),
                           QByteArrayLiteral("test"), 1, true, false);
    QMQTT::Message tmp;
    QBENCHMARK
    {
      message = tmp;
    }
  }
}

void tst_message::write_data()
{
  QTest::addColumn<bool>("firstRun");

  QTest::newRow("construction + first run") << true;
  QTest::newRow("subsequent runs") << false;
}

void tst_message::write()
{
  QFETCH(bool, firstRun);

  if (firstRun)
  {
    QBENCHMARK
    {
      QMQTT::Message message(0, QStringLiteral("test/test"),
                             QByteArrayLiteral("test"), 1, true, false);
      message.setRetain(true);
    }
  }
  else
  {
    QMQTT::Message message(0, QStringLiteral("test/test"),
                           QByteArrayLiteral("test"), 1, true, false);
    QBENCHMARK
    {
      message.setRetain(true);
    }
  }
}

void tst_message::copyOnWrite_data()
{
  QTest::addColumn<bool>("firstRun");

  QTest::newRow("construction + first run") << true;
  QTest::newRow("subsequent runs") << false;
}

void tst_message::copyOnWrite()
{
  QFETCH(bool, firstRun);

  if (firstRun)
  {
    QBENCHMARK
    {
      QMQTT::Message message1(0, QStringLiteral("test/1"),
                              QByteArrayLiteral("1"), 1, true, false);
      QMQTT::Message message2(0, QStringLiteral("test/2"),
                              QByteArrayLiteral("2"), 2, false, true);
      QMQTT::Message message = message1;
      message.setRetain(false);
      message = message2;
      message.setDup(false);
    }
  }
  else
  {
    QMQTT::Message message1(0, QStringLiteral("test/1"), QByteArrayLiteral("1"),
                            1, true, false);
    QMQTT::Message message2(0, QStringLiteral("test/2"), QByteArrayLiteral("2"),
                            2, false, true);
    QBENCHMARK
    {
      QMQTT::Message message = message1;
      message.setRetain(false);
      message = message2;
      message.setDup(false);
    }
  }
}

QTEST_MAIN(tst_message)

#include "main.moc"
