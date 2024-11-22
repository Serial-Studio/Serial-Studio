#include "TestLog.h"
#include "QtTestUtil/QtTestUtil.h"
#include "QsLog.h"
#include "QsLogDest.h"
#include "QsLogDestFile.h"
#include "QsLogDestConsole.h"
#include "QsLogDestFile.h"
#include "QsLogDestFunctor.h"
#include <QTest>
#include <QSharedPointer>
#include <QtGlobal>

const QLatin1String destinationName1("mock1");
const QLatin1String destinationName2("mock2");
using MockDestinationPtrU = std::unique_ptr<MockDestination>;

// Needed to convert from removeDestination return value to the type that we initially added.
MockDestinationPtrU unique_cast(QsLogging::DestinationPtrU&& d)
{
    MockDestinationPtrU md(dynamic_cast<MockDestination*>(d.release()));
    Q_ASSERT(md.get());
    return md;
}

void DummyLogFunction(const QsLogging::LogMessage&)
{
}

// Autotests for QsLog.
// These tests are based on using a mock destination to check what was logged. Destinations are
// owned by the log, that's why the add/remove ping-pong is needed in the tests.
class TestLog : public QObject
{
    Q_OBJECT
public:
    TestLog()
    {
    }

private slots:
    void initTestCase();
    void testAllLevels();
    void testMessageText();
    void testLevelChanges();
    void testLevelParsing();
    void testDestinationRemove();
    void testWillRotate();
    void testRotation_data();
    void testRotation();
    void testRotationNoBackup();
    void testDestinationType();

private:
};

void TestLog::initTestCase()
{
    using namespace QsLogging;
    QCOMPARE(Logger::instance().loggingLevel(), InfoLevel);
    Logger::instance().setLoggingLevel(TraceLevel);
    QCOMPARE(Logger::instance().loggingLevel(), TraceLevel);
}

void TestLog::testAllLevels()
{
    using namespace QsLogging;
    MockDestinationPtrU mockDest(new MockDestination(destinationName1));
    Logger::instance().addDestination(std::move(mockDest));

    QLOG_TRACE() << "trace level";
    QLOG_DEBUG() << "debug level";
    QLOG_INFO() << "info level";
    QLOG_WARN() << "warn level";
    QLOG_ERROR() << "error level";
    QLOG_FATAL() << "fatal level";

    mockDest = unique_cast(Logger::instance().removeDestination(destinationName1));
    QCOMPARE(mockDest->messageCount(), 6);
    QCOMPARE(mockDest->messageCountForLevel(TraceLevel), 1);
    QCOMPARE(mockDest->messageCountForLevel(DebugLevel), 1);
    QCOMPARE(mockDest->messageCountForLevel(InfoLevel), 1);
    QCOMPARE(mockDest->messageCountForLevel(WarnLevel), 1);
    QCOMPARE(mockDest->messageCountForLevel(ErrorLevel), 1);
    QCOMPARE(mockDest->messageCountForLevel(FatalLevel), 1);
}

void TestLog::testMessageText()
{
    using namespace QsLogging;
    MockDestinationPtrU mockDest(new MockDestination(destinationName1));
    Logger::instance().addDestination(std::move(mockDest));

    QLOG_DEBUG() << "foobar";
    QLOG_WARN() << "eiszeit";
    QLOG_FATAL() << "ruh-roh!";

    mockDest = unique_cast(Logger::instance().removeDestination(destinationName1));
    QVERIFY(mockDest->hasMessage("foobar", DebugLevel));
    QVERIFY(mockDest->hasMessage("eiszeit", WarnLevel));
    QVERIFY(mockDest->hasMessage("ruh-roh!", FatalLevel));
    QCOMPARE(mockDest->messageCount(), 3);
}

void TestLog::testLevelChanges()
{
    using namespace QsLogging;
    MockDestinationPtrU mockDest1(new MockDestination(destinationName1));
    MockDestinationPtrU mockDest2(new MockDestination(destinationName2));
    Logger::instance().addDestination(std::move(mockDest1));
    Logger::instance().addDestination(std::move(mockDest2));

    using namespace QsLogging;
    Logger::instance().setLoggingLevel(WarnLevel);
    QCOMPARE(Logger::instance().loggingLevel(), WarnLevel);

    QLOG_TRACE() << "one";
    QLOG_DEBUG() << "two";
    QLOG_INFO() << "three";

    mockDest1 = unique_cast(Logger::instance().removeDestination(destinationName1));
    mockDest2 = unique_cast(Logger::instance().removeDestination(destinationName2));

    QCOMPARE(mockDest1->messageCount(), 0);
    QCOMPARE(mockDest2->messageCount(), 0);

    Logger::instance().addDestination(std::move(mockDest1));
    Logger::instance().addDestination(std::move(mockDest2));

    QLOG_WARN() << "warning";
    QLOG_ERROR() << "error";
    QLOG_FATAL() << "fatal";

    mockDest1 = unique_cast(Logger::instance().removeDestination(destinationName1));
    mockDest2 = unique_cast(Logger::instance().removeDestination(destinationName2));

    QCOMPARE(mockDest1->messageCountForLevel(WarnLevel), 1);
    QCOMPARE(mockDest1->messageCountForLevel(ErrorLevel), 1);
    QCOMPARE(mockDest1->messageCountForLevel(FatalLevel), 1);
    QCOMPARE(mockDest1->messageCount(), 3);
    QCOMPARE(mockDest2->messageCountForLevel(WarnLevel), 1);
    QCOMPARE(mockDest2->messageCountForLevel(ErrorLevel), 1);
    QCOMPARE(mockDest2->messageCountForLevel(FatalLevel), 1);
    QCOMPARE(mockDest2->messageCount(), 3);
}

void TestLog::testLevelParsing()
{
    using namespace QsLogging;
    MockDestinationPtrU mockDest(new MockDestination(destinationName1));
    Logger::instance().addDestination(std::move(mockDest));

    QLOG_TRACE() << "one";
    QLOG_DEBUG() << "two";
    QLOG_INFO() << "three";
    QLOG_WARN() << "warning";
    QLOG_ERROR() << "error";
    QLOG_FATAL() << "fatal";

    mockDest = unique_cast(Logger::instance().removeDestination(destinationName1));

    using namespace QsLogging;
    for(int i = 0;i < mockDest->messageCount();++i) {
        bool conversionOk = false;
        const MockDestination::Message& m = mockDest->messageAt(i);
        QCOMPARE(Logger::levelFromLogMessage(m.text, &conversionOk), m.level);
        QCOMPARE(Logger::levelFromLogMessage(m.text), m.level);
        QCOMPARE(conversionOk, true);
    }
}

void TestLog::testDestinationRemove()
{
    using namespace QsLogging;
    MockDestinationPtrU mockDest(new MockDestination(destinationName1));
    MockDestinationPtrU toAddAndRemove(new MockDestination(destinationName2));
    Logger::instance().addDestination(std::move(mockDest));
    Logger::instance().addDestination(std::move(toAddAndRemove));
    Logger::instance().setLoggingLevel(DebugLevel);
    QLOG_INFO() << "one for all";

    mockDest = unique_cast(Logger::instance().removeDestination(destinationName1));
    toAddAndRemove = unique_cast(Logger::instance().removeDestination(destinationName2));
    QCOMPARE(mockDest->messageCount(), 1);
    QCOMPARE(toAddAndRemove->messageCount(), 1);

    Logger::instance().addDestination(std::move(mockDest));
    QLOG_INFO() << "one for (almost) all";

    mockDest = unique_cast(Logger::instance().removeDestination(destinationName1));
    QCOMPARE(mockDest->messageCount(), 2);
    QCOMPARE(toAddAndRemove->messageCount(), 1);
    QCOMPARE(toAddAndRemove->messageCountForLevel(InfoLevel), 1);

    Logger::instance().addDestination(std::move(mockDest));
    Logger::instance().addDestination(std::move(toAddAndRemove));
    QLOG_INFO() << "another one for all";

    mockDest = unique_cast(Logger::instance().removeDestination(destinationName1));
    toAddAndRemove = unique_cast(Logger::instance().removeDestination(destinationName2));
    QCOMPARE(mockDest->messageCount(), 3);
    QCOMPARE(toAddAndRemove->messageCount(), 2);
    QCOMPARE(toAddAndRemove->messageCountForLevel(InfoLevel), 2);
}

void TestLog::testWillRotate()
{
    using namespace QsLogging;
    MockSizeRotationStrategy rotationStrategy;
    rotationStrategy.setBackupCount(1);
    rotationStrategy.setMaximumSizeInBytes(10);
    QCOMPARE(rotationStrategy.shouldRotate(), false);

    rotationStrategy.includeMessageInCalculation(QLatin1String("12345"));
    QCOMPARE(rotationStrategy.shouldRotate(), false);

    rotationStrategy.includeMessageInCalculation(QLatin1String("12345"));
    QCOMPARE(rotationStrategy.shouldRotate(), false);

    rotationStrategy.includeMessageInCalculation(QLatin1String("1"));
    QCOMPARE(rotationStrategy.shouldRotate(), true);
}

void TestLog::testRotation_data()
{
    QTest::addColumn<int>("backupCount");

    QTest::newRow("one backup") << 1;
    QTest::newRow("three backups") << 3;
    QTest::newRow("five backups") << 5;
    QTest::newRow("ten backups") << 10;
}

void TestLog::testRotation()
{
    using namespace QsLogging;

    QFETCH(int, backupCount);
    MockSizeRotationStrategy rotationStrategy;
    rotationStrategy.setBackupCount(backupCount);
    for (int i = 0;i < backupCount;++i) {
        rotationStrategy.rotate();
        QCOMPARE(rotationStrategy.filesList().size(), 1 + i + 1); // 1 log + "rotation count" backups
    }

    rotationStrategy.rotate();
    QCOMPARE(rotationStrategy.filesList().size(), backupCount + 1); // 1 log + backup count
}

void TestLog::testRotationNoBackup()
{
    using namespace QsLogging;
    MockSizeRotationStrategy rotationStrategy;
    rotationStrategy.setBackupCount(0);
    rotationStrategy.setMaximumSizeInBytes(10);

    rotationStrategy.rotate();
    QCOMPARE(rotationStrategy.filesList().size(), 1); // log
}



void TestLog::testDestinationType()
{
    using namespace QsLogging;

    DestinationPtrU console = DestinationFactory::MakeDebugOutputDestination();
    DestinationPtrU file = DestinationFactory::MakeFileDestination("test.log",
                           LogRotationOption::DisableLogRotation, MaxSizeBytes(5000), MaxOldLogCount(1));
    DestinationPtrU func = DestinationFactory::MakeFunctorDestination(&DummyLogFunction);

    QCOMPARE(Logger::instance().hasDestinationOfType(DebugOutputDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FileDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FunctorDestination::Type), false);

    Logger::instance().addDestination(std::move(console));
    QCOMPARE(Logger::instance().hasDestinationOfType(DebugOutputDestination::Type), true);
    QCOMPARE(Logger::instance().hasDestinationOfType(FileDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FunctorDestination::Type), false);

    Logger::instance().addDestination(std::move(file));
    QCOMPARE(Logger::instance().hasDestinationOfType(DebugOutputDestination::Type), true);
    QCOMPARE(Logger::instance().hasDestinationOfType(FileDestination::Type), true);
    QCOMPARE(Logger::instance().hasDestinationOfType(FunctorDestination::Type), false);

    Logger::instance().addDestination(std::move(func));
    QCOMPARE(Logger::instance().hasDestinationOfType(DebugOutputDestination::Type), true);
    QCOMPARE(Logger::instance().hasDestinationOfType(FileDestination::Type), true);
    QCOMPARE(Logger::instance().hasDestinationOfType(FunctorDestination::Type), true);

    Logger::instance().removeDestination(DebugOutputDestination::Type);
    QCOMPARE(Logger::instance().hasDestinationOfType(DebugOutputDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FileDestination::Type), true);
    QCOMPARE(Logger::instance().hasDestinationOfType(FunctorDestination::Type), true);

    Logger::instance().removeDestination(FileDestination::Type);
    QCOMPARE(Logger::instance().hasDestinationOfType(DebugOutputDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FileDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FunctorDestination::Type), true);

    Logger::instance().removeDestination(FunctorDestination::Type);
    QCOMPARE(Logger::instance().hasDestinationOfType(DebugOutputDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FileDestination::Type), false);
    QCOMPARE(Logger::instance().hasDestinationOfType(FunctorDestination::Type), false);
}

QTTESTUTIL_REGISTER_TEST(TestLog);
#include "TestLog.moc"
