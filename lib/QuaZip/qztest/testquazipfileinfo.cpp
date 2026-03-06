#include "testquazipfileinfo.h"

#include "qztest.h"

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QPair>

#include <QtTest/QtTest>

#include <quazip.h>
#include <quazipfile.h>
#include <quazipfileinfo.h>
#include <quazipnewinfo.h>
#include <quazip_qt_compat.h>

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QList<qint32>);
Q_DECLARE_METATYPE(QuaExtraFieldHash);
#endif

void TestQuaZipFileInfo::getNTFSTime()
{
    QFETCH(QString, zipName);
    QFETCH(bool, useMTime);
    QFETCH(bool, useATime);
    QFETCH(bool, useCTime);
    QStringList testFiles;
    testFiles << "test.txt";
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(testFiles)) {
        QFAIL("Can't create test file");
    }
    QDateTime base(QDate(1601, 1, 1), QTime(0, 0), QTimeZone::utc());
    quint64 mTicks, aTicks, cTicks;
    QFileInfo fileInfo("tmp/test.txt");
    {
        // create
        QuaZip zip(zipName);
        QVERIFY(zip.open(QuaZip::mdCreate));
        QuaZipFile zipFile(&zip);
        QDateTime lm = fileInfo.lastModified().toUTC();
        QDateTime lr = fileInfo.lastRead().toUTC();
        QDateTime cr = quazip_ctime(fileInfo).toUTC();
        mTicks = (static_cast<qint64>(base.date().daysTo(lm.date()))
                * Q_UINT64_C(86400000)
                + static_cast<qint64>(base.time().msecsTo(lm.time())))
            * Q_UINT64_C(10000);
        aTicks = (static_cast<qint64>(base.date().daysTo(lr.date()))
                * Q_UINT64_C(86400000)
                + static_cast<qint64>(base.time().msecsTo(lr.time())))
            * Q_UINT64_C(10000);
        cTicks = (static_cast<qint64>(base.date().daysTo(cr.date()))
                * Q_UINT64_C(86400000)
                + static_cast<qint64>(base.time().msecsTo(cr.time())))
            * Q_UINT64_C(10000);
        QuaZipNewInfo newInfo("test.txt", "tmp/test.txt");
        QByteArray extra(36, 0);
        extra[0] = 0x0A; // magic
        extra[1] = 0x00;
        extra[2] = 32; // size
        extra[3] = 0;
        extra[4] = extra[5] = extra[6] = extra[7] = 0; // reserved
        extra[8] = 0x01; // time tag
        extra[9] = 0x00;
        extra[10] = 24; // time tag size
        extra[11] = 0;
        for (int i = 12; i < 36; i += 8) {
            quint64 ticks;
            if (i == 12) {
                ticks = useMTime ? mTicks : 0;
            } else if (i == 20) {
                ticks = useATime ? aTicks : 0;
            } else if (i == 28) {
                ticks = useCTime ? cTicks : 0;
            } else {
                QFAIL("Stupid programming bug here");
            }
            extra[i] = static_cast<char>(ticks);
            extra[i + 1] = static_cast<char>(ticks >> 8);
            extra[i + 2] = static_cast<char>(ticks >> 16);
            extra[i + 3] = static_cast<char>(ticks >> 24);
            extra[i + 4] = static_cast<char>(ticks >> 32);
            extra[i + 5] = static_cast<char>(ticks >> 40);
            extra[i + 6] = static_cast<char>(ticks >> 48);
            extra[i + 7] = static_cast<char>(ticks >> 56);
        }
        newInfo.extraLocal = extra;
        newInfo.extraGlobal = extra;
        QVERIFY(zipFile.open(QIODevice::WriteOnly, newInfo));
        zipFile.close();
        zip.close();
    }
    {
        // check
        QuaZip zip(zipName);
        QVERIFY(zip.open(QuaZip::mdUnzip));
        zip.goToFirstFile();
        QuaZipFileInfo64 zipFileInfo;
        QVERIFY(zip.getCurrentFileInfo(&zipFileInfo));
        zip.close();
        QCOMPARE(zipFileInfo.getNTFSmTime(), useMTime ? fileInfo.lastModified() : QDateTime());
        QCOMPARE(zipFileInfo.getNTFSaTime(), useATime ? fileInfo.lastRead() : QDateTime());
        QCOMPARE(zipFileInfo.getNTFScTime(), useCTime ? quazip_ctime(fileInfo) : QDateTime());
    }
    removeTestFiles(testFiles);
    curDir.remove(zipName);
}

void TestQuaZipFileInfo::getNTFSTime_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<bool>("useMTime");
    QTest::addColumn<bool>("useATime");
    QTest::addColumn<bool>("useCTime");
    QTest::newRow("no times") << QString::fromUtf8("noTimes") << false << false << false;
    QTest::newRow("all times") << QString::fromUtf8("allTimes") << true << true << true;
    QTest::newRow("no CTime") << QString::fromUtf8("noCTime") << true << true << false;
}

void TestQuaZipFileInfo::getExtTime_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<quint8>("flags");
    QTest::addColumn<quint16>("sizeLocal");
    QTest::addColumn< QList<qint32> >("timesLocal");
    QTest::addColumn<quint16>("sizeGlobal");
    QTest::addColumn< QList<qint32> >("timesGlobal");
    QTest::addColumn<QDateTime>("expectedModTime");
    QTest::addColumn<QDateTime>("expectedAcTime");
    QTest::addColumn<QDateTime>("expectedCrTime");
    QTest::newRow("no times") << QString::fromUtf8("noTimes")
                              << static_cast<quint8>(0)
                              << static_cast<quint16>(1)
                              << QList<qint32>()
                              << static_cast<quint16>(1)
                              << QList<qint32>()
                              << QDateTime()
                              << QDateTime()
                              << QDateTime();
    QTest::newRow("all times") << QString::fromUtf8("allTimes")
                              << static_cast<quint8>(7)
                              << static_cast<quint16>(13)
                              << (QList<qint32>() << 1 << 2 << 3)
                              << static_cast<quint16>(5)
                              << (QList<qint32>() << 1)
                              << QDateTime(QDate(1970, 1, 1), QTime(0, 0, 1), QTimeZone::utc())
                              << QDateTime(QDate(1970, 1, 1), QTime(0, 0, 2), QTimeZone::utc())
                              << QDateTime(QDate(1970, 1, 1), QTime(0, 0, 3), QTimeZone::utc());
    QTest::newRow("no ac time") << QString::fromUtf8("noAcTime")
                              << static_cast<quint8>(5)
                              << static_cast<quint16>(9)
                              << (QList<qint32>() << 1 << 3)
                              << static_cast<quint16>(5)
                              << (QList<qint32>() << 1)
                              << QDateTime(QDate(1970, 1, 1), QTime(0, 0, 1), QTimeZone::utc())
                              << QDateTime()
                              << QDateTime(QDate(1970, 1, 1), QTime(0, 0, 3), QTimeZone::utc());
    QTest::newRow("negativeTime") << QString::fromUtf8("negativeTime")
                              << static_cast<quint8>(1)
                              << static_cast<quint16>(5)
                              << (QList<qint32>() << -1)
                              << static_cast<quint16>(5)
                              << (QList<qint32>() << -1)
                              << QDateTime(QDate(1969, 12, 31), QTime(23, 59, 59), QTimeZone::utc())
                              << QDateTime()
                              << QDateTime();
}

static QByteArray makeExtTimeField(quint16 size, quint8 flags, const QList<qint32> &times)
{
    if (size == 0)
        return QByteArray();
    const quint16 EXT_TIME_MAGIC = 0x5455;
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    QDataStream localStream(&buffer);
    localStream.setByteOrder(QDataStream::LittleEndian);
    localStream << EXT_TIME_MAGIC;
    localStream << size;
    localStream << flags;
    Q_FOREACH(qint32 time, times) {
        localStream << time;
    }
    return buffer.buffer();
}

void TestQuaZipFileInfo::getExtTime()
{
    QFETCH(QString, zipName);
    QFETCH(quint8, flags);
    QFETCH(quint16, sizeLocal);
    QFETCH(QList<qint32>, timesLocal);
    QFETCH(quint16, sizeGlobal);
    QFETCH(QList<qint32>, timesGlobal);
    QFETCH(QDateTime, expectedModTime);
    QFETCH(QDateTime, expectedAcTime);
    QFETCH(QDateTime, expectedCrTime);
    QStringList fileNames;
    QString fileName = QString::fromLatin1("aFile.txt");
    fileNames << fileName;
    {
        createTestFiles(fileNames);
        QuaZip testZip("tmp/" + zipName);
        testZip.open(QuaZip::mdCreate);
        QuaZipFile file(&testZip);
        QuaZipNewInfo newInfo(fileName, "tmp/" + fileName);
        newInfo.extraLocal = makeExtTimeField(sizeLocal, flags, timesLocal);
        newInfo.extraGlobal = makeExtTimeField(sizeGlobal, flags, timesGlobal);
        file.open(QIODevice::WriteOnly, newInfo);
        file.close();
        testZip.close();
    }
    removeTestFiles(fileNames);
    QuaZip zip("tmp/" + zipName);
    QVERIFY(zip.open(QuaZip::mdUnzip));
    QVERIFY(zip.goToFirstFile());
    QuaZipFileInfo64 fileInfo;
    QVERIFY(zip.getCurrentFileInfo(&fileInfo));
    QuaZipFile zipFile(&zip);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    QDateTime actualGlobalModTime = fileInfo.getExtModTime();
    QDateTime actualLocalModTime = zipFile.getExtModTime();
    QDateTime actualLocalAcTime = zipFile.getExtAcTime();
    QDateTime actualLocalCrTime = zipFile.getExtCrTime();
    zipFile.close();
    QCOMPARE(actualGlobalModTime, expectedModTime);
    QCOMPARE(actualLocalModTime, expectedModTime);
    QCOMPARE(actualLocalAcTime, expectedAcTime);
    QCOMPARE(actualLocalCrTime, expectedCrTime);
    zip.close();
    QDir("tmp").remove(zipName);
}

void TestQuaZipFileInfo::getExtTime_issue43()
{
    // Test original GitHub issue, just in case.
    // (The test above relies on manual ZIP generation, which isn't perfect.)
    QuaZip zip(":/test_files/issue43_cant_get_dates.zip");
    QVERIFY(zip.open(QuaZip::mdUnzip));
    zip.goToFirstFile();
    QuaZipFileInfo64 zipFileInfo;
    QVERIFY(zip.getCurrentFileInfo(&zipFileInfo));
    zip.goToFirstFile();
    QuaZipFile zipFile(&zip);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    QDateTime actualGlobalModTime = zipFileInfo.getExtModTime();
    QDateTime actualLocalModTime = zipFile.getExtModTime();
    QDateTime actualLocalAcTime = zipFile.getExtAcTime();
    QDateTime actualLocalCrTime = zipFile.getExtCrTime();
    zip.close();
    QDateTime extModTime(QDate(2019, 7, 2), QTime(15, 43, 47), QTimeZone::utc());
    QDateTime extAcTime = extModTime;
    QDateTime extCrTime = extModTime;
    QCOMPARE(actualGlobalModTime, extModTime);
    QCOMPARE(actualLocalModTime, extModTime);
    QCOMPARE(actualLocalAcTime, extAcTime);
    QCOMPARE(actualLocalCrTime, extCrTime);
}

void TestQuaZipFileInfo::parseExtraField_data()
{
    QTest::addColumn<QByteArray>("extraField");
    QTest::addColumn<QuaExtraFieldHash>("expected");
    QTest::newRow("empty")
            << QByteArray()
            << QuaExtraFieldHash();
    {
    QuaExtraFieldHash oneZeroIdWithNoData;
    oneZeroIdWithNoData[0] = QList<QByteArray>() << QByteArray();
    QTest::newRow("one zero ID with no data")
            << QByteArray("\x00\x00\x00\x00", 4)
            << oneZeroIdWithNoData;
    }
    {
    QuaExtraFieldHash oneZeroIdWithData;
    oneZeroIdWithData[0] = QList<QByteArray>() << QByteArray("DATA", 4);
    QTest::newRow("one zero ID with data")
            << QByteArray("\x00\x00\x04\x00" "DATA", 8)
            << oneZeroIdWithData;
    }
    {
    QuaExtraFieldHash oneNonZeroIdWithData;
    oneNonZeroIdWithData[1] = QList<QByteArray>() << QByteArray("DATA", 4);
    QTest::newRow("one non zero ID with data")
            << QByteArray("\x01\x00\x04\x00" "DATA", 8)
            << oneNonZeroIdWithData;
    }
    {
    QuaExtraFieldHash severalDifferentIDs;
    severalDifferentIDs[0] = QList<QByteArray>() << QByteArray("DATA0", 5);
    severalDifferentIDs[1] = QList<QByteArray>() << QByteArray("DATA1", 5);
    QTest::newRow("two IDs")
            << QByteArray("\x00\x00\x05\x00" "DATA0" "\x01\x00\x05\x00" "DATA1", 18)
            << severalDifferentIDs;
    }
    {
    QuaExtraFieldHash repeatedID;
    repeatedID[0] = QList<QByteArray>() << QByteArray("DATA0", 5) << QByteArray("DATA1", 5);
    QTest::newRow("repeated ID")
            << QByteArray("\x00\x00\x05\x00" "DATA0" "\x00\x00\x05\x00" "DATA1", 18)
            << repeatedID;
    }
    {
    QuaExtraFieldHash largeID;
    largeID[0x0102] = QList<QByteArray>() << QByteArray("DATA", 4);
    QTest::newRow("large ID")
            << QByteArray("\x02\x01\x04\x00" "DATA", 8)
            << largeID;
    }
    {
    QuaExtraFieldHash largeData;
    largeData[0] = QList<QByteArray>() << QByteArray(65535, 'x');
    QTest::newRow("large ID")
            << (QByteArray("\x00\x00\xff\xff", 4) + QByteArray(65535, 'x'))
            << largeData;
    }
    QTest::newRow("only 1 byte") << QByteArray("\x00", 1) << QuaExtraFieldHash();
    QTest::newRow("only 2 bytes") << QByteArray("\x00\x00", 2) << QuaExtraFieldHash();
    QTest::newRow("only 3 bytes") << QByteArray("\x00\x00\x00", 3) << QuaExtraFieldHash();
    QTest::newRow("truncated data") << QByteArray("\x00\x00\x02\x00\x00", 5) << QuaExtraFieldHash();
}

void TestQuaZipFileInfo::parseExtraField()
{
    QFETCH(QByteArray, extraField);
    QFETCH(QuaExtraFieldHash, expected);
    QuaExtraFieldHash actual = QuaZipFileInfo64::parseExtraField(extraField);
    QCOMPARE(actual, expected);
}
