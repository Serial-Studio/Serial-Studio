#include <QtTest/QtTest>
#include <Logger.h>
#include <AbstractAppender.h>


class TestAppender : public AbstractAppender
{
  protected:
    void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line, const char* function,
                const QString& category, const QString& message) override;

  public:
    struct Record
    {
      QDateTime timeStamp;
      Logger::LogLevel logLevel;
      const char* file;
      int line;
      const char* function;
      QString category;
      QString message;
    };
    QList<Record> records;

    void clear() { records.clear(); }
};


void TestAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line, const char* function, const QString& category, const QString& message)
{
  records.append({ timeStamp, logLevel, file, line, function, category, message });
}


class BasicTest : public QObject
{
  Q_OBJECT

  private slots:
    void initTestCase();

    void testCString();
    void testQDebug();
    void testRecursiveQDebug();

    void cleanupTestCase();

  private:
    TestAppender appender;

    int testQDebugInt();
};


void BasicTest::initTestCase()
{
  cuteLogger->registerAppender(&appender);
}


void BasicTest::testCString()
{
  LOG_DEBUG("Message");

  QCOMPARE(appender.records.size(), 1);
  TestAppender::Record r = appender.records.last();
  appender.clear();

  QCOMPARE(r.logLevel, Logger::Debug);
  QCOMPARE(r.file, __FILE__);
  QVERIFY(r.line != 0);
  QCOMPARE(r.function, Q_FUNC_INFO);
  QCOMPARE(r.category, QString());
  QCOMPARE(r.message, QStringLiteral("Message"));
}


void BasicTest::testQDebug()
{
  LOG_DEBUG() << "Message" << 5;

  QCOMPARE(appender.records.size(), 1);
  TestAppender::Record r = appender.records.last();
  appender.clear();

  QCOMPARE(r.logLevel, Logger::Debug);
  QCOMPARE(r.file, __FILE__);
  QVERIFY(r.line != 0);
  QCOMPARE(r.function, Q_FUNC_INFO);
  QCOMPARE(r.category, QString());
  QCOMPARE(r.message, QStringLiteral("Message 5 "));
}


void BasicTest::testRecursiveQDebug()
{
  LOG_DEBUG() << "Message" << testQDebugInt();
  QCOMPARE(appender.records.size(), 2);

  auto r1 = appender.records.first();
  auto r2 = appender.records.last();
  appender.clear();

  QCOMPARE(r2.function, Q_FUNC_INFO);
  QCOMPARE(r1.message, QStringLiteral("Test "));
  QCOMPARE(r2.message, QStringLiteral("Message 0 "));
}


void BasicTest::cleanupTestCase()
{
  cuteLogger->removeAppender(&appender);
}


int BasicTest::testQDebugInt()
{
  LOG_DEBUG() << "Test";
  return 0;
}


QTEST_MAIN(BasicTest)
#include "basictest.moc"
