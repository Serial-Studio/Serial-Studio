/*
Copyright (C) 2005-2014 Sergey A. Tachenov

This file is part of QuaZip test suite.

QuaZip is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

QuaZip is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QuaZip.  If not, see <http://www.gnu.org/licenses/>.

See COPYING file for the full LGPL text.

Original ZIP package is copyrighted by Gilles Vollant and contributors,
see quazip/(un)zip.h files for details. Basically it's the zlib license.
*/

#include "testquazip.h"

#include "qztest.h"

#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#ifdef QUAZIP_TEST_QSAVEFILE
#include <QtCore/QSaveFile>
#endif
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <quazip_qt_compat.h>

#include <QtTest/QtTest>

#include <quazip.h>
#include <JlCompress.h>

void TestQuaZip::getFileList_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::newRow("simple") << "qzfilelist.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt");
    QTest::newRow("russian") << QString::fromUtf8("файл.zip") << (
        QStringList() << QString::fromUtf8("test0.txt") << QString::fromUtf8("test1/test1.txt")
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt");
    QTest::newRow("japanese") << QString::fromUtf8("テスト.zip") << (
        QStringList() << QString::fromUtf8("test.txt"));
    QTest::newRow("hebrew") << QString::fromUtf8("פתח תקווה.zip") << (
        QStringList() << QString::fromUtf8("test.txt"));
}

void TestQuaZip::getFileList()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    fileNames.sort();
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Can't create test file");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Can't create test archive");
    }
    QuaZip testZip(zipName);
    QVERIFY(testZip.open(QuaZip::mdUnzip));
    QVERIFY(testZip.goToFirstFile());
    QString firstFile = testZip.getCurrentFileName();
    QStringList fileList = testZip.getFileNameList();
    fileList.sort();
    QCOMPARE(fileList, fileNames);
    QHash<QString, QFileInfo> srcInfo;
    foreach (QString fileName, fileNames) {
        srcInfo[fileName] = QFileInfo("tmp/" + fileName);
    }
    QList<QuaZipFileInfo> destList = testZip.getFileInfoList();
    QCOMPARE(destList.size(), srcInfo.size());
    for (const auto& dest : destList) {
        QCOMPARE(static_cast<qint64>(dest.uncompressedSize),
                srcInfo[dest.name].size());
    }
    // Now test zip64
    QList<QuaZipFileInfo64> destList64 = testZip.getFileInfoList64();
    QCOMPARE(destList64.size(), srcInfo.size());
    for (const auto& dest : destList64) {
        QCOMPARE(static_cast<qint64>(dest.uncompressedSize),
                srcInfo[dest.name].size());
    }
    // test that we didn't mess up the current file
    QCOMPARE(testZip.getCurrentFileName(), firstFile);
    testZip.close();
    // clean up
    removeTestFiles(fileNames);
    curDir.remove(zipName);
}

void TestQuaZip::add_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QStringList>("fileNamesToAdd");
    QTest::newRow("simple") << "qzadd.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << (QStringList() << "testAdd.txt");
}

void TestQuaZip::add()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QStringList, fileNamesToAdd);
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Can't create test file");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Can't create test archive");
    }
    if (!createTestFiles(fileNamesToAdd)) {
        QFAIL("Can't create test files to add");
    }
    QuaZip testZip(zipName);
    QVERIFY(testZip.open(QuaZip::mdUnzip));
    // according to the bug #3485459 the global is lost, so we test it
    QString globalComment = testZip.getComment();
    testZip.close();
    QVERIFY(testZip.open(QuaZip::mdAdd));
    foreach (QString fileName, fileNamesToAdd) {
        QuaZipFile testFile(&testZip);
        QVERIFY(testFile.open(QIODevice::WriteOnly,
            QuaZipNewInfo(fileName, "tmp/" + fileName)));
        QFile inFile("tmp/" + fileName);
        QVERIFY(inFile.open(QIODevice::ReadOnly));
        testFile.write(inFile.readAll());
        inFile.close();
        testFile.close();
    }
    testZip.close();
    QVERIFY(testZip.open(QuaZip::mdUnzip));
    QStringList allNames = fileNames + fileNamesToAdd;
    QCOMPARE(testZip.getEntriesCount(), allNames.size());
    QCOMPARE(testZip.getFileNameList(), allNames);
    QCOMPARE(testZip.getComment(), globalComment);
    testZip.close();
    removeTestFiles(fileNames);
    removeTestFiles(fileNamesToAdd);
    curDir.remove(zipName);
}

void TestQuaZip::setFileNameCodec_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QByteArray>("encoding");
    QTest::newRow("russian") << QString::fromUtf8("russian.zip") << (
        QStringList() << QString::fromUtf8("тест.txt")) << QByteArray("IBM866");
}

void TestQuaZip::setFileNameCodec()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QByteArray, encoding);
    fileNames.sort();
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Can't create test file");
    }
    if (!createTestArchive(zipName, fileNames,
                           QTextCodec::codecForName(encoding))) {
        QFAIL("Can't create test archive");
    }
    QuaZip testZip(zipName);
    QVERIFY(testZip.open(QuaZip::mdUnzip));
    QStringList fileList = testZip.getFileNameList();
    fileList.sort();
    QVERIFY(fileList[0] != fileNames[0]);
    testZip.close();
    testZip.setFileNameCodec(encoding);
    QVERIFY(testZip.open(QuaZip::mdUnzip));
    fileList = testZip.getFileNameList();
    fileList.sort();
    QCOMPARE(fileList, fileNames);
    testZip.close();
    // clean up
    removeTestFiles(fileNames);
    curDir.remove(zipName);
}

void TestQuaZip::setOsCode_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<uint>("osCode");
    QTest::newRow("unix") << "unix.zip" << 3u;
    QTest::newRow("dos") << "dos.zip" << 0u;
}

void TestQuaZip::setOsCode()
{
    QFETCH(QString, zipName);
    QFETCH(uint, osCode);
    QuaZip testZip(zipName);
    testZip.setOsCode(osCode);
    testZip.open(QuaZip::mdCreate);
    QCOMPARE(testZip.getOsCode(), osCode);
    QuaZipFile testZipFile(&testZip);
    testZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo("test.txt"));
    testZipFile.close();
    testZip.close();
    QuaZip checkZip(zipName);
    checkZip.open(QuaZip::mdUnzip);
    checkZip.goToFirstFile();
    QuaZipFileInfo64 fi;
    QVERIFY(checkZip.getCurrentFileInfo(&fi));
    QCOMPARE(static_cast<uint>(fi.versionCreated) >> 8, osCode);
    checkZip.close();
    QDir().remove(zipName);
}

void TestQuaZip::setDataDescriptorWritingEnabled()
{
    QString zipName = "zip10.zip";
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    QuaZip testZip(zipName);
    testZip.setDataDescriptorWritingEnabled(false);
    QVERIFY(testZip.open(QuaZip::mdCreate));
    QuaZipFile testZipFile(&testZip);
    QVERIFY(testZipFile.open(QIODevice::WriteOnly,
                             QuaZipNewInfo("vegetation_info.xml"), NULL, 0, 0));
    QByteArray contents = "<vegetation_info version=\"4096\" />\n";
    testZipFile.write(contents);
    testZipFile.close();
    testZip.close();
    QuaZipFile readZipFile(zipName, "vegetation_info.xml");
    QVERIFY(readZipFile.open(QIODevice::ReadOnly));
    // Test that file is not compressed.
    QCOMPARE(readZipFile.csize(), static_cast<qint64>(contents.size()));
    readZipFile.close();
    QCOMPARE(QFileInfo(zipName).size(), static_cast<qint64>(171));
    QFile zipFile(zipName);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    QDataStream zipData(&zipFile);
    zipData.setByteOrder(QDataStream::LittleEndian);
    quint32 magic = 0;
    quint16 versionNeeded = 0;
    zipData >> magic;
    zipData >> versionNeeded;
    QCOMPARE(magic, static_cast<quint32>(0x04034b50));
    QCOMPARE(versionNeeded, static_cast<quint16>(10));
    zipFile.close();
    curDir.remove(zipName);
    // now test 2.0
    zipName = "zip20.zip";
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    QuaZip testZip20(zipName);
    QVERIFY(testZip20.open(QuaZip::mdCreate));
    QuaZipFile testZipFile20(&testZip20);
    QVERIFY(testZipFile20.open(QIODevice::WriteOnly,
                             QuaZipNewInfo("vegetation_info.xml"), NULL, 0, 0));
    testZipFile20.write("<vegetation_info version=\"4096\" />\n");
    testZipFile20.close();
    testZip20.close();
    QCOMPARE(QFileInfo(zipName).size(),
            static_cast<qint64>(171 + 16)); // 16 bytes = data descriptor
    QFile zipFile20(zipName);
    QVERIFY(zipFile20.open(QIODevice::ReadOnly));
    QDataStream zipData20(&zipFile20);
    zipData20.setByteOrder(QDataStream::LittleEndian);
    magic = 0;
    versionNeeded = 0;
    zipData20 >> magic;
    zipData20 >> versionNeeded;
    QCOMPARE(magic, static_cast<quint32>(0x04034b50));
    QCOMPARE(versionNeeded, static_cast<quint16>(20));
    zipFile20.close();
    curDir.remove(zipName);
}

void TestQuaZip::testQIODeviceAPI()
{
    QString zipName = "qiodevice_api.zip";
    QStringList fileNames;
    fileNames << "test.txt";
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Can't create test file");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Can't create test archive");
    }
    QBuffer buffer;
    if (!createTestArchive(&buffer, fileNames, NULL)) {
        QFAIL("Can't create test archive");
    }
    QFile diskFile(zipName);
    QVERIFY(diskFile.open(QIODevice::ReadOnly));
    QByteArray bufferArray = buffer.buffer();
    QByteArray fileArray = diskFile.readAll();
    diskFile.close();
    QCOMPARE(bufferArray.size(), fileArray.size());
    QCOMPARE(bufferArray, fileArray);
    curDir.remove(zipName);
}

void TestQuaZip::setZipName()
{
    QuaZip zip;
    zip.setZipName("testSetZipName.zip");
    zip.open(QuaZip::mdCreate);
    zip.close();
    QVERIFY(QFileInfo(zip.getZipName()).exists());
    QDir().remove(zip.getZipName());
}

void TestQuaZip::setIoDevice()
{
    QuaZip zip;
    QFile file("testSetIoDevice.zip");
    zip.setIoDevice(&file);
    QCOMPARE(zip.getIoDevice(), &file);
    zip.open(QuaZip::mdCreate);
    QVERIFY(file.isOpen());
    zip.close();
    QVERIFY(!file.isOpen());
    QVERIFY(file.exists());
    QDir().remove(file.fileName());
}

void TestQuaZip::setCommentCodec()
{
    QuaZip zip("commentCodec.zip");
    QVERIFY(zip.open(QuaZip::mdCreate));
    zip.setCommentCodec("WINDOWS-1251");
    zip.setComment(QString::fromUtf8("Вопрос"));
    QuaZipFile zipFile(&zip);
    QVERIFY(zipFile.open(QIODevice::WriteOnly, QuaZipNewInfo("test.txt")));
    zipFile.close();
    zip.close();
    QVERIFY(zip.open(QuaZip::mdUnzip));
    zip.setCommentCodec(QTextCodec::codecForName("KOI8-R"));
    QCOMPARE(zip.getComment(), QString::fromUtf8("бНОПНЯ"));
    zip.close();
    QDir().remove(zip.getZipName());
}

void TestQuaZip::setAutoClose()
{
    {
        QBuffer buf;
        QuaZip zip(&buf);
        QVERIFY(zip.isAutoClose());
        QVERIFY(zip.open(QuaZip::mdCreate));
        QVERIFY(buf.isOpen());
        zip.close();
        QVERIFY(!buf.isOpen());
        QVERIFY(zip.open(QuaZip::mdCreate));
    }
    {
        QBuffer buf;
        QuaZip zip(&buf);
        QVERIFY(zip.isAutoClose());
        zip.setAutoClose(false);
        QVERIFY(!zip.isAutoClose());
        QVERIFY(zip.open(QuaZip::mdCreate));
        QVERIFY(buf.isOpen());
        zip.close();
        QVERIFY(buf.isOpen());
        QVERIFY(zip.open(QuaZip::mdCreate));
    }
}

#ifdef QUAZIP_TEST_QSAVEFILE
void TestQuaZip::saveFileBug()
{
    QDir curDir;
    QString zipName = "testSaveFile.zip";
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName)) {
            QFAIL("Can't remove testSaveFile.zip");
        }
    }
    QuaZip zip;
    QSaveFile saveFile(zipName);
    zip.setIoDevice(&saveFile);
    QCOMPARE(zip.getIoDevice(), &saveFile);
    zip.open(QuaZip::mdCreate);
    zip.close();
    QVERIFY(QFileInfo(saveFile.fileName()).exists());
    curDir.remove(saveFile.fileName());
}
#endif

void TestQuaZip::testSequential()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress(QHostAddress::LocalHost)));
    quint16 port = server.serverPort();
    QTcpSocket socket;
    socket.connectToHost(QHostAddress(QHostAddress::LocalHost), port);
    QVERIFY(socket.waitForConnected());
    QVERIFY(server.waitForNewConnection(30000));
    QTcpSocket *client = server.nextPendingConnection();
    QuaZip zip(&socket);
    zip.setAutoClose(false);
    QVERIFY(zip.open(QuaZip::mdCreate));
    QVERIFY(socket.isOpen());
    QuaZipFile zipFile(&zip);
    QuaZipNewInfo info("test.txt");
    QVERIFY(zipFile.open(QIODevice::WriteOnly, info, NULL, 0, 0));
    QCOMPARE(zipFile.write("test"), static_cast<qint64>(4));
    zipFile.close();
    zip.close();
    QVERIFY(socket.isOpen());
    socket.disconnectFromHost();
    QVERIFY(socket.waitForDisconnected());
    QVERIFY(client->waitForReadyRead());
    QByteArray received = client->readAll();
#ifdef QUAZIP_QZTEST_QUAZIP_DEBUG_SOCKET
    QFile debug("testSequential.zip");
    debug.open(QIODevice::WriteOnly);
    debug.write(received);
    debug.close();
#endif
    client->close();
    QBuffer buffer(&received);
    QuaZip receivedZip(&buffer);
    QVERIFY(receivedZip.open(QuaZip::mdUnzip));
    QVERIFY(receivedZip.goToFirstFile());
    QuaZipFileInfo64 receivedInfo;
    QVERIFY(receivedZip.getCurrentFileInfo(&receivedInfo));
    QCOMPARE(receivedInfo.name, QString::fromLatin1("test.txt"));
    QCOMPARE(receivedInfo.uncompressedSize, static_cast<quint64>(4));
    QuaZipFile receivedFile(&receivedZip);
    QVERIFY(receivedFile.open(QIODevice::ReadOnly));
    QByteArray receivedText = receivedFile.readAll();
    QCOMPARE(receivedText, QByteArray("test"));
    receivedFile.close();
    receivedZip.close();
}
