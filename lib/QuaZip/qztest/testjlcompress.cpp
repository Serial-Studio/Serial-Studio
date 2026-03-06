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

#include "testjlcompress.h"

#include "qztest.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMetaType>
#include <quazip_qt_compat.h>

#include <QtTest/QtTest>

#include <JlCompress.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Q_DECLARE_METATYPE(JlCompress::Options::CompressionStrategy)

void TestJlCompress::compressFile_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QString>("fileName");
    QTest::newRow("simple") << "jlsimplefile.zip" << "test0.txt";
}

void TestJlCompress::compressFile()
{
    QFETCH(QString, zipName);
    QFETCH(QString, fileName);
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(QStringList() << fileName)) {
        QFAIL("Can't create test file");
    }
    QVERIFY(JlCompress::compressFile(zipName, "tmp/" + fileName));
    // get the file list and check it
    QStringList fileList = JlCompress::getFileList(zipName);
    QCOMPARE(fileList.count(), 1);
    QVERIFY(fileList[0] == fileName);
    // now test the QIODevice* overload of getFileList()
    QFile zipFile(zipName);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    fileList = JlCompress::getFileList(zipName);
    QCOMPARE(fileList.count(), 1);
    QVERIFY(fileList[0] == fileName);
    zipFile.close();
    removeTestFiles(QStringList() << fileName);
    curDir.remove(zipName);
}

void TestJlCompress::compressFileOptions_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<JlCompress::Options::CompressionStrategy>("strategy");
    QTest::addColumn<QString>("sha256sum_unix"); // Due to extra data archives are not identical
    QTest::addColumn<QString>("sha256sum_win");
    QTest::newRow("simple") << "jlsimplefile.zip"
                            << "test0.txt"
                            << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                            << JlCompress::Options::Default
                            << "5eedd83aee92cf3381155d167fee54a4ef6e43b8bc7a979c903611d9aa28610a"
                            << "cb1847dff1a5c33a805efde2558fc74024ad4c64c8607f8b12903e4d92385955";
    QTest::newRow("simple-storage") << "jlsimplefile-storage.zip"
                                    << "test0.txt"
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Storage
                                    << ""
                                    << "";
    QTest::newRow("simple-fastest") << "jlsimplefile-fastest.zip"
                                    << "test0.txt"
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Fastest
                                    << ""
                                    << "";
    QTest::newRow("simple-faster") << "jlsimplefile-faster.zip"
                                   << "test0.txt"
                                   << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                   << JlCompress::Options::Faster
                                   << ""
                                   << "";
    QTest::newRow("simple-standard") << "jlsimplefile-standard.zip"
                                     << "test0.txt"
                                     << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                     << JlCompress::Options::Standard
                                     << "5eedd83aee92cf3381155d167fee54a4ef6e43b8bc7a979c903611d9aa28610a"
                                     << "cb1847dff1a5c33a805efde2558fc74024ad4c64c8607f8b12903e4d92385955";
    QTest::newRow("simple-better") << "jlsimplefile-better.zip"
                                   << "test0.txt"
                                   << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                   << JlCompress::Options::Better
                                   << ""
                                   << "";
    QTest::newRow("simple-best") << "jlsimplefile-best.zip"
                                 << "test0.txt"
                                 << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                 << JlCompress::Options::Best
                                 << ""
                                 << "";
}

void TestJlCompress::compressFileOptions()
{
    QFETCH(QString, zipName);
    QFETCH(QString, fileName);
    QFETCH(QDateTime, dateTime);
    QFETCH(JlCompress::Options::CompressionStrategy, strategy);
    QFETCH(QString, sha256sum_unix);
    QFETCH(QString, sha256sum_win);
    QDir curDir;
    if (curDir.exists(zipName)) {
      if (!curDir.remove(zipName))
          QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(QStringList() << fileName)) {
        QFAIL("Can't create test file");
    }

    const JlCompress::Options options(dateTime, strategy);
    QVERIFY(JlCompress::compressFile(zipName, "tmp/" + fileName, options));
    // get the file list and check it
    QStringList fileList = JlCompress::getFileList(zipName);
    QCOMPARE(fileList.count(), 1);
    QVERIFY(fileList[0] == fileName);
    // now test the QIODevice* overload of getFileList()
    QFile zipFile(zipName);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    fileList = JlCompress::getFileList(zipName);
    QCOMPARE(fileList.count(), 1);
    QVERIFY(fileList[0] == fileName);
    // Hash is computed on the resulting file externally, then hardcoded in the test data
    // This should help detecting any library breakage since we compare against a well-known stable result
    QString hash = QCryptographicHash::hash(zipFile.readAll(), QCryptographicHash::Sha256).toHex();
    #ifdef Q_OS_WIN
    if (!sha256sum_win.isEmpty()) QCOMPARE(hash, sha256sum_win);
    #else
    if (!sha256sum_unix.isEmpty()) QCOMPARE(hash, sha256sum_unix);
    #endif
    zipFile.close();
    removeTestFiles(QStringList() << fileName);
    curDir.remove(zipName);
}

void TestJlCompress::compressFiles_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::newRow("simple") << "jlsimplefiles.zip" <<
        (QStringList() << "test0.txt" << "test00.txt");
    QTest::newRow("different subdirs") << "jlsubdirfiles.zip" <<
        (QStringList() << "subdir1/test1.txt" << "subdir2/test2.txt");
}

void TestJlCompress::compressFiles()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Can't create test files");
    }
    QStringList realNamesList, shortNamesList;
    foreach (QString fileName, fileNames) {
        QString realName = "tmp/" + fileName;
        realNamesList += realName;
        shortNamesList += QFileInfo(realName).fileName();
    }
    QVERIFY(JlCompress::compressFiles(zipName, realNamesList));
    // get the file list and check it
    QStringList fileList = JlCompress::getFileList(zipName);
    QCOMPARE(fileList, shortNamesList);
    removeTestFiles(fileNames);
    curDir.remove(zipName);
}

void TestJlCompress::compressDir_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QStringList>("expected");
    QTest::newRow("simple") << "jldir.zip"
        << (QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
		<< (QStringList() << "test0.txt"
			<< "testdir1/" << "testdir1/test1.txt"
            << "testdir2/" << "testdir2/test2.txt"
			<< "testdir2/subdir/" << "testdir2/subdir/test2sub.txt");
    QTest::newRow("empty dirs") << "jldir_empty.zip"
		<< (QStringList() << "testdir1/" << "testdir2/testdir3/")
        << (QStringList() << "testdir1/" << "testdir2/"
            << "testdir2/testdir3/");
    QTest::newRow("hidden files") << "jldir_hidden.zip"
        << (QStringList() << ".test0.txt" << "test1.txt")
        << (QStringList() << ".test0.txt" << "test1.txt");
}

void TestJlCompress::compressDir()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QStringList, expected);
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(fileNames, -1, "compressDir_tmp")) {
        QFAIL("Can't create test files");
    }
#ifdef Q_OS_WIN
    for (int i = 0; i < fileNames.size(); ++i) {
        if (fileNames.at(i).startsWith(".")) {
            QString fn = "compressDir_tmp\\" + fileNames.at(i);
            SetFileAttributesW(reinterpret_cast<LPCWSTR>(fn.utf16()),
                              FILE_ATTRIBUTE_HIDDEN);
        }
    }
#endif
    QVERIFY(JlCompress::compressDir(zipName, "compressDir_tmp", true, QDir::Hidden));
    // get the file list and check it
    QStringList fileList = JlCompress::getFileList(zipName);
    fileList.sort();
    expected.sort();
    QCOMPARE(fileList, expected);
    removeTestFiles(fileNames, "compressDir_tmp");
    curDir.remove(zipName);
}

void TestJlCompress::compressDirOptions_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<JlCompress::Options::CompressionStrategy>("strategy");
    QTest::addColumn<QString>("sha256sum_unix");
    QTest::addColumn<QString>("sha256sum_win");
    QTest::newRow("simple") << "jldir.zip"
                            << (QStringList() << "test0.txt" << "testdir1/test1.txt"
                                              << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
                            << (QStringList() << "test0.txt"
                                              << "testdir1/" << "testdir1/test1.txt"
                                              << "testdir2/" << "testdir2/test2.txt"
                                              << "testdir2/subdir/" << "testdir2/subdir/test2sub.txt")
                            << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                            << JlCompress::Options::Default
                            << "ed0d5921b2fc11b6b4cb214b3e43ea3ea28987d6ff8080faab54c4756de30af6"
                            << "1eba110a33718c07a4ddf3fa515d1b4c6e3f4fc912b2e29e5e32783e2cddf852";
    QTest::newRow("simple-storage") << "jldir-storage.zip"
                                    << (QStringList() << "test0.txt" << "testdir1/test1.txt"
                                                      << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
                                    << (QStringList() << "test0.txt"
                                                      << "testdir1/" << "testdir1/test1.txt"
                                                      << "testdir2/" << "testdir2/test2.txt"
                                                      << "testdir2/subdir/" << "testdir2/subdir/test2sub.txt")
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Storage
                                    << ""
                                    << "";
    QTest::newRow("simple-fastest") << "jldir-fastest.zip"
                                    << (QStringList() << "test0.txt" << "testdir1/test1.txt"
                                                      << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
                                    << (QStringList() << "test0.txt"
                                                      << "testdir1/" << "testdir1/test1.txt"
                                                      << "testdir2/" << "testdir2/test2.txt"
                                                      << "testdir2/subdir/" << "testdir2/subdir/test2sub.txt")
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Fastest
                                    << ""
                                    << "";
    QTest::newRow("simple-faster") << "jldir-faster.zip"
                                    << (QStringList() << "test0.txt" << "testdir1/test1.txt"
                                                      << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
                                    << (QStringList() << "test0.txt"
                                                      << "testdir1/" << "testdir1/test1.txt"
                                                      << "testdir2/" << "testdir2/test2.txt"
                                                      << "testdir2/subdir/" << "testdir2/subdir/test2sub.txt")
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Faster
                                    << ""
                                    << "";
    QTest::newRow("simple-standard") << "jldir-standard.zip"
                                    << (QStringList() << "test0.txt" << "testdir1/test1.txt"
                                                      << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
                                    << (QStringList() << "test0.txt"
                                                      << "testdir1/" << "testdir1/test1.txt"
                                                      << "testdir2/" << "testdir2/test2.txt"
                                                      << "testdir2/subdir/" << "testdir2/subdir/test2sub.txt")
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Standard
                                    << "ed0d5921b2fc11b6b4cb214b3e43ea3ea28987d6ff8080faab54c4756de30af6"
                                    << "1eba110a33718c07a4ddf3fa515d1b4c6e3f4fc912b2e29e5e32783e2cddf852";
    QTest::newRow("simple-better") << "jldir-better.zip"
                                    << (QStringList() << "test0.txt" << "testdir1/test1.txt"
                                                      << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
                                    << (QStringList() << "test0.txt"
                                                      << "testdir1/" << "testdir1/test1.txt"
                                                      << "testdir2/" << "testdir2/test2.txt"
                                                      << "testdir2/subdir/" << "testdir2/subdir/test2sub.txt")
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Better
                                    << ""
                                    << "";
    QTest::newRow("simple-best") << "jldir-best.zip"
                                    << (QStringList() << "test0.txt" << "testdir1/test1.txt"
                                                      << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
                                    << (QStringList() << "test0.txt"
                                                      << "testdir1/" << "testdir1/test1.txt"
                                                      << "testdir2/" << "testdir2/test2.txt"
                                                      << "testdir2/subdir/" << "testdir2/subdir/test2sub.txt")
                                    << QDateTime(QDate(2024, 9, 19), QTime(21, 0, 0), QTimeZone::utc())
                                    << JlCompress::Options::Best
                                    << ""
                                    << "";
}

void TestJlCompress::compressDirOptions()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QStringList, expected);
    QFETCH(QDateTime, dateTime);
    QFETCH(JlCompress::Options::CompressionStrategy, strategy);
    QFETCH(QString, sha256sum_unix);
    QFETCH(QString, sha256sum_win);
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(fileNames, -1, "compressDir_tmp")) {
        QFAIL("Can't create test files");
    }
#ifdef Q_OS_WIN
    for (int i = 0; i < fileNames.size(); ++i) {
        if (fileNames.at(i).startsWith(".")) {
            QString fn = "compressDir_tmp\\" + fileNames.at(i);
            SetFileAttributesW(reinterpret_cast<LPCWSTR>(fn.utf16()),
                               FILE_ATTRIBUTE_HIDDEN);
        }
    }
#endif
    const JlCompress::Options options(dateTime, strategy);
    QVERIFY(JlCompress::compressDir(zipName, "compressDir_tmp", true, QDir::Hidden, options));
    // get the file list and check it
    QStringList fileList = JlCompress::getFileList(zipName);
    fileList.sort();
    expected.sort();
    QCOMPARE(fileList, expected);
    QFile zipFile(curDir.absoluteFilePath(zipName));
    if (!zipFile.open(QIODevice::ReadOnly)) {
        QFAIL("Can't read output zip file");
    }
    QString hash = QCryptographicHash::hash(zipFile.readAll(), QCryptographicHash::Sha256).toHex();
#ifdef Q_OS_WIN
    if (!sha256sum_win.isEmpty()) QCOMPARE(hash, sha256sum_win);
#else
    if (!sha256sum_unix.isEmpty()) QCOMPARE(hash, sha256sum_unix);
#endif
    zipFile.close();
    removeTestFiles(fileNames, "compressDir_tmp");
    curDir.remove(zipName);
}

void TestJlCompress::extractFile_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QString>("fileToExtract");
    QTest::addColumn<QString>("destName");
    QTest::addColumn<QByteArray>("encoding");
    QTest::newRow("simple") << "jlextfile.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
        << "testdir2/test2.txt" << "test2.txt" << QByteArray();
    QTest::newRow("russian") << "jlextfilerus.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << QString::fromUtf8("testdir2/тест2.txt")
            << "testdir2/subdir/test2sub.txt")
        << QString::fromUtf8("testdir2/тест2.txt")
        << QString::fromUtf8("тест2.txt") << QByteArray("IBM866");
    QTest::newRow("extract dir") << "jlextdir.zip" << (
            QStringList() << "testdir1/")
        << "testdir1/" << "testdir1/" << QByteArray();
}

void TestJlCompress::extractFile()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QString, fileToExtract);
    QFETCH(QString, destName);
    QFETCH(QByteArray, encoding);
    QDir curDir;
    if (!curDir.mkpath("jlext/jlfile")) {
        QFAIL("Couldn't mkpath jlext/jlfile");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    QFile srcFile("tmp/" + fileToExtract);
    QFile::Permissions srcPerm = srcFile.permissions();
    // Invert the "write other" flag so permissions
    // are NOT default any more. Otherwise it's impossible
    // to figure out whether the permissions were set correctly
    // or JlCompress failed to set them completely,
    // thus leaving them at the default setting.
    srcPerm ^= QFile::WriteOther;
    QVERIFY(srcFile.setPermissions(srcPerm));
    if (!createTestArchive(zipName, fileNames,
                           QTextCodec::codecForName(encoding))) {
        QFAIL("Can't create test archive");
    }
    QuaZip::setDefaultFileNameCodec(encoding);
    QVERIFY(!JlCompress::extractFile(zipName, fileToExtract,
                "jlext/jlfile/" + destName).isEmpty());
    QFileInfo destInfo("jlext/jlfile/" + destName), srcInfo("tmp/" +
            fileToExtract);
    QCOMPARE(destInfo.size(), srcInfo.size());
    QCOMPARE(destInfo.permissions(), srcInfo.permissions());
    curDir.remove("jlext/jlfile/" + destName);
    // now test the QIODevice* overload
    QFile zipFile(zipName);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    QVERIFY(!JlCompress::extractFile(&zipFile, fileToExtract,
                "jlext/jlfile/" + destName).isEmpty());
    destInfo = QFileInfo("jlext/jlfile/" + destName);
    QCOMPARE(destInfo.size(), srcInfo.size());
    QCOMPARE(destInfo.permissions(), srcInfo.permissions());
    curDir.remove("jlext/jlfile/" + destName);
    if (!fileToExtract.endsWith("/")) {
        // If we aren't extracting a directory, we need to check
        // that extractFile() fails if there is a directory
        // with the same name as the file being extracted.
        curDir.mkdir("jlext/jlfile/" + destName);
        QVERIFY(JlCompress::extractFile(zipName, fileToExtract,
                    "jlext/jlfile/" + destName).isEmpty());
    }
    zipFile.close();
    // Here we either delete the target dir or the dir created in the
    // test above.
    curDir.rmpath("jlext/jlfile/" + destName);
    removeTestFiles(fileNames);
    curDir.remove(zipName);
}

void TestJlCompress::extractFiles_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QStringList>("filesToExtract");
    QTest::newRow("simple") << "jlextfiles.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
        << (QStringList() << "testdir2/test2.txt" << "testdir1/test1.txt");
}

void TestJlCompress::extractFiles()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QStringList, filesToExtract);
    QDir curDir;
    if (!curDir.mkpath("jlext/jlfiles")) {
        QFAIL("Couldn't mkpath jlext/jlfiles");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    if (!JlCompress::compressDir(zipName, "tmp")) {
        QFAIL("Couldn't create test archive");
    }
    QVERIFY(!JlCompress::extractFiles(zipName, filesToExtract,
                "jlext/jlfiles").isEmpty());
    foreach (QString fileName, filesToExtract) {
        QFileInfo fileInfo("jlext/jlfiles/" + fileName);
        QFileInfo extInfo("tmp/" + fileName);
        QCOMPARE(fileInfo.size(), extInfo.size());
        QCOMPARE(fileInfo.permissions(), extInfo.permissions());
        curDir.remove("jlext/jlfiles/" + fileName);
        curDir.rmpath(fileInfo.dir().path());
    }
    // now test the QIODevice* overload
    QFile zipFile(zipName);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    QVERIFY(!JlCompress::extractFiles(&zipFile, filesToExtract,
                "jlext/jlfiles").isEmpty());
    foreach (QString fileName, filesToExtract) {
        QFileInfo fileInfo("jlext/jlfiles/" + fileName);
        QFileInfo extInfo("tmp/" + fileName);
        QCOMPARE(fileInfo.size(), extInfo.size());
        QCOMPARE(fileInfo.permissions(), extInfo.permissions());
        curDir.remove("jlext/jlfiles/" + fileName);
        curDir.rmpath(fileInfo.dir().path());
    }
    zipFile.close();
    curDir.rmpath("jlext/jlfiles");
    removeTestFiles(fileNames);
    curDir.remove(zipName);
}

void TestJlCompress::extractDir_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QStringList>("expectedExtracted");
    QTest::addColumn<QString>("extDir");
    QTest::addColumn<QByteArray>("fileNameCodecName");
    QTest::newRow("simple") << "jlextdir.zip"
        << (QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
        << (QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
        << "tmp/jlext/jldir/"
        << QByteArray();
    QTest::newRow("separate dir") << "sepdir.zip"
        << (QStringList() << "laj/" << "laj/lajfile.txt")
        << (QStringList() << "laj/" << "laj/lajfile.txt")
        << "tmp/jlext/jldir/"
        << QByteArray();
    QTest::newRow("Zip Slip") << "zipslip.zip"
        << (QStringList() << "test0.txt" << "../zipslip.txt")
        << (QStringList() << "test0.txt")
        << "tmp/jlext/jldir/"
        << QByteArray();
    QTest::newRow("Cyrillic") << "cyrillic.zip"
        << (QStringList() << QString::fromUtf8("Ще не вмерла Україна"))
        << (QStringList() << QString::fromUtf8("Ще не вмерла Україна"))
        << "tmp/jlext/jldir/"
        << QByteArray("KOI8-U");
    QTest::newRow("Japaneses") << "japanese.zip"
        << (QStringList() << QString::fromUtf8("日本"))
        << (QStringList() << QString::fromUtf8("日本"))
        << "tmp/jlext/jldir/"
        << QByteArray("UTF-8");
#ifdef QUAZIP_EXTRACT_TO_ROOT_TEST
    QTest::newRow("Exract to root") << "tmp/quazip-root-test.zip"
        << (QStringList() << "tmp/quazip-root-test/test.txt")
        << (QStringList() << "tmp/quazip-root-test/test.txt")
        << QString()
        << QByteArray("UTF-8");
#endif
}

class TemporarilyChangeToRoot {
    QString previousDir;
    bool success{false};
public:
    ~TemporarilyChangeToRoot() {
        if (success) {
            QDir::setCurrent(previousDir); // Let's HOPE it succeeds.
        }
    }
    void changeToRoot() {
        previousDir = QDir::currentPath();
        success = QDir::setCurrent("/");
    }
    inline bool isSuccess() const { return success; }
};

void TestJlCompress::extractDir()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QStringList, expectedExtracted);
    QFETCH(QString, extDir);
    QFETCH(QByteArray, fileNameCodecName);
    TemporarilyChangeToRoot changeToRoot;
    if (zipName == "tmp/quazip-root-test.zip") {
        changeToRoot.changeToRoot();
        if (!changeToRoot.isSuccess()) {
            QFAIL("Couldn't change to /");
        }
    }
    QTextCodec *fileNameCodec = NULL;
    if (!fileNameCodecName.isEmpty())
        fileNameCodec = QTextCodec::codecForName(fileNameCodecName);
    QDir curDir;
    if (!extDir.isEmpty() && !curDir.mkpath(extDir)) {
        QFAIL("Couldn't mkpath extDir");
    }
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    if (!createTestArchive(zipName, fileNames, fileNameCodec)) {
        QFAIL("Couldn't create test archive");
    }
    QStringList extracted;
    if (fileNameCodec == NULL)
        extracted = JlCompress::extractDir(zipName, extDir);
    else // test both overloads here
        extracted = JlCompress::extractDir(zipName, fileNameCodec, extDir);
    QCOMPARE(extracted.count(), expectedExtracted.count());
    const QString dir = extDir;
    foreach (QString fileName, expectedExtracted) {
        QString fullName = dir + fileName;
        QFileInfo fileInfo(fullName);
        QFileInfo extInfo("tmp/" + fileName);
        if (!fileInfo.isDir())
            QCOMPARE(fileInfo.size(), extInfo.size());
        QCOMPARE(fileInfo.permissions(), extInfo.permissions());
        curDir.remove(fullName);
        curDir.rmpath(fileInfo.dir().path());
        QString absolutePath = QDir(dir).absoluteFilePath(fileName);
        if (fileInfo.isDir() && !absolutePath.endsWith('/'))
            absolutePath += '/';
        QVERIFY(extracted.contains(absolutePath));
    }
    // now test the QIODevice* overload
    QFile zipFile(zipName);
    QVERIFY(zipFile.open(QIODevice::ReadOnly));
    if (fileNameCodec == NULL)
        extracted = JlCompress::extractDir(&zipFile, extDir);
    else // test both overloads here
        extracted = JlCompress::extractDir(&zipFile, fileNameCodec, extDir);
    QCOMPARE(extracted.count(), expectedExtracted.count());
    foreach (QString fileName, expectedExtracted) {
        QString fullName = dir + fileName;
        QFileInfo fileInfo(fullName);
        QFileInfo extInfo("tmp/" + fileName);
        if (!fileInfo.isDir())
            QCOMPARE(fileInfo.size(), extInfo.size());
        QCOMPARE(fileInfo.permissions(), extInfo.permissions());
        curDir.remove(fullName);
        curDir.rmpath(fileInfo.dir().path());
        QString absolutePath = QDir(dir).absoluteFilePath(fileName);
        if (fileInfo.isDir() && !absolutePath.endsWith('/'))
            absolutePath += '/';
        QVERIFY(extracted.contains(absolutePath));
    }
    zipFile.close();
    curDir.rmpath(extDir);
    removeTestFiles(fileNames);
    //curDir.remove(zipName);
}

void TestJlCompress::zeroPermissions()
{
    QuaZip zipCreator("zero.zip");
    QVERIFY(zipCreator.open(QuaZip::mdCreate));
    QuaZipFile zeroFile(&zipCreator);
    QuaZipNewInfo newInfo("zero.txt");
    newInfo.externalAttr = 0; // should be zero anyway, but just in case
    QVERIFY(zeroFile.open(QIODevice::WriteOnly, newInfo));
    zeroFile.close();
    zipCreator.close();
    QVERIFY(!JlCompress::extractFile("zero.zip", "zero.txt").isEmpty());
    QVERIFY(QFile("zero.txt").permissions() != 0);
    QDir curDir;
    curDir.remove("zero.zip");
    curDir.remove("zero.txt");
}

#ifdef QUAZIP_SYMLINK_TEST

void TestJlCompress::symlinkHandling()
{
    QStringList fileNames { "file.txt" };
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    QVERIFY(QFile::link("file.txt", "tmp/link.txt"));
    fileNames << "link.txt";
    QVERIFY(JlCompress::compressDir("symlink.zip", "tmp"));
    QDir curDir;
    QVERIFY(curDir.mkpath("extsymlink"));
    QVERIFY(!JlCompress::extractDir("symlink.zip", "extsymlink").isEmpty());
    QFileInfo linkInfo("extsymlink/link.txt");
    QVERIFY(quazip_is_symlink(linkInfo));
    removeTestFiles(fileNames, "extsymlink");
    removeTestFiles(fileNames, "tmp");
    curDir.remove("symlink.zip");
}

#endif

#ifdef QUAZIP_SYMLINK_EXTRACTION_ON_WINDOWS_TEST

void TestJlCompress::symlinkExtractionOnWindows()
{
    QuaZip zipWithSymlinks("withSymlinks.zip");
    QVERIFY(zipWithSymlinks.open(QuaZip::mdCreate));
    QuaZipFile file(&zipWithSymlinks);
    QVERIFY(file.open(QIODevice::WriteOnly, QuaZipNewInfo("file.txt")));
    file.write("contents");
    file.close();
    QuaZipNewInfo symlinkInfo("symlink.txt");
    symlinkInfo.externalAttr |= 0120000 << 16; // symlink attr
    QuaZipFile symlink(&zipWithSymlinks);
    QVERIFY(symlink.open(QIODevice::WriteOnly, symlinkInfo));
    symlink.write("file.txt"); // link target goes into contents
    symlink.close();
    zipWithSymlinks.close();
    QCOMPARE(zipWithSymlinks.getZipError(), ZIP_OK);
    // The best we can do here is to test that extraction works at all,
    // because it's hard to say what should be the “correct” result when
    // trying to extract symbolic links on Windows.
    QVERIFY(!JlCompress::extractDir("withSymlinks.zip", "symlinksOnWindows").isEmpty());
    QDir curDir;
    curDir.remove("withSymlinks.zip");
    removeTestFiles(QStringList() << "file.txt" << "symlink.txt", "symlinksOnWindows");
}

#endif
