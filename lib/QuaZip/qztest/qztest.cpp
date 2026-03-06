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

#include "qztest.h"
#include "testjlcompress.h"
#include "testjlcp_compress.h"
#include "testjlcp_extract.h"
#include "testquachecksum32.h"
#include "testquagzipfile.h"
#include "testquaziodevice.h"
#include "testquazip.h"
#include "testquazipdir.h"
#include "testquazipfile.h"
#include "testquazipfileinfo.h"
#include "testquazipnewinfo.h"

#include <quazip.h>
#include <quazipfile.h>
#include <quazip_qt_compat.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

#include <QtTest/QtTest>

bool createTestFiles(const QStringList &fileNames, int size, const QString &dir)
{
    QDir curDir;
    foreach (QString fileName, fileNames) {
        QString filePath = QDir(dir).filePath(fileName);
        QDir testDir = QFileInfo(filePath).dir();
        if (!testDir.exists()) {
            if (!curDir.mkpath(testDir.path())) {
                qWarning("Couldn't mkpath %s",
                        testDir.path().toUtf8().constData());
                return false;
            }
            //qDebug() << "Created path " << testDir.path();
            QFile dirFile(testDir.path());
            if (!dirFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                  QFileDevice::ReadGroup | QFileDevice::ExeGroup |
                                  QFileDevice::ReadOther | QFileDevice::ExeOther)) {
                qWarning("Couldn't set permissions for %s",
                         testDir.path().toUtf8().constData());
                return false;
            }
        }
        if (fileName.endsWith('/')) {
            if (!curDir.mkpath(filePath)) {
                qWarning("Couldn't mkpath %s",
				fileName.toUtf8().constData());
                return false;
            }
            //qDebug() << "Created path " << filePath;
            QFile dirFile(filePath);
            if (!dirFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                   QFileDevice::ReadGroup | QFileDevice::ExeGroup |
                                   QFileDevice::ReadOther | QFileDevice::ExeOther)) {
              qWarning("Couldn't set permissions for %s",
                       filePath.toUtf8().constData());
              return false;
            }
        } else {
            QFile testFile(filePath);
            if (!testFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qWarning("Couldn't create %s",
                        fileName.toUtf8().constData());
                return false;
            }
            testFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                QFileDevice::ReadGroup | QFileDevice::ReadOther);
            if (size == -1) {
                QTextStream testStream(&testFile);
                testStream << "This is a test file named " << fileName << quazip_endl;
            } else {
                for (int i = 0; i < size; ++i) {
                    testFile.putChar(static_cast<char>('0' + i % 10));
                }
            }
        }
    }
    return true;
}

bool createTestArchive(QuaZip &zip, const QString &zipName,
                       const QStringList &fileNames,
                       QTextCodec *codec,
                       const QString &dir)
{
    if (codec != NULL) {
        zip.setFileNameCodec(codec);
    }
    if (!zip.open(QuaZip::mdCreate)) {
        qWarning("Couldn't open %s", zipName.toUtf8().constData());
        return false;
    }
    int i = 0;
    QDateTime dt1;
    foreach (QString fileName, fileNames) {
        QuaZipFile zipFile(&zip);
        QString filePath = QDir(dir).filePath(fileName);
        QFileInfo fileInfo(filePath);
        QuaZipNewInfo newInfo(fileName, filePath);
        if (i == 0) // to test code that needs different timestamps
            newInfo.dateTime = newInfo.dateTime.addSecs(-60);
        else if (i == 1) // will use for the next file too
            dt1 = newInfo.dateTime;
        else if (i == 2) // to test identical timestamps
            newInfo.dateTime = dt1;
        if (!zipFile.open(QIODevice::WriteOnly,
                newInfo, NULL, 0,
                fileInfo.isDir() ? 0 : 8)) {
            qWarning("Couldn't open %s in %s", fileName.toUtf8()
                .constData(), zipName.toUtf8().constData());
            return false;
        }
        if (!fileInfo.isDir()) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning("Couldn't open %s", filePath.toUtf8()
                    .constData());
                return false;
            }
            while (!file.atEnd()) {
                char buf[4096];
                qint64 l = file.read(buf, 4096);
                if (l <= 0) {
                    qWarning("Couldn't read %s", filePath.toUtf8()
                        .constData());
                    return false;
                }
                if (zipFile.write(buf, l) != l) {
                    qWarning("Couldn't write to %s in %s",
                        filePath.toUtf8().constData(),
                        zipName.toUtf8().constData());
                    return false;
                }
            }
            file.close();
        }
        zipFile.close();
        ++i;
    }
    zip.setComment(QString("This is the test archive"));
    zip.close();
    if (zipName.startsWith("<")) { // something like "<QIODevice pointer>"
        return true;
    }
    return QFileInfo(zipName).exists();
}

bool createTestArchive(const QString &zipName,
                       const QStringList &fileNames,
                       const QString &dir) {
    return createTestArchive(zipName, fileNames, NULL, dir);
}

bool createTestArchive(QIODevice *ioDevice,
                              const QStringList &fileNames,
                              QTextCodec *codec,
                              const QString &dir)
{
    QuaZip zip(ioDevice);
    return createTestArchive(zip, "<QIODevice pointer>", fileNames, codec, dir);
}

bool createTestArchive(const QString &zipName,
                              const QStringList &fileNames,
                              QTextCodec *codec,
                              const QString &dir) {
    QuaZip zip(zipName);
    return createTestArchive(zip, zipName, fileNames, codec, dir);
}

void removeTestFiles(const QStringList &fileNames, const QString &dir)
{
    QDir curDir;
    foreach (QString fileName, fileNames) {
        curDir.remove(QDir(dir).filePath(fileName));
    }
    foreach (QString fileName, fileNames) {
        QDir fileDir = QFileInfo(QDir(dir).filePath(fileName)).dir();
        if (fileDir.exists()) {
            // Non-empty dirs won't get removed, and that's good.
            curDir.rmpath(fileDir.path());
        }
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    int err = 0;
    {
        TestQuaZip testQuaZip;
        err = qMax(err, QTest::qExec(&testQuaZip, app.arguments()));
    }
    {
        TestQuaZipFile testQuaZipFile;
        err = qMax(err, QTest::qExec(&testQuaZipFile, app.arguments()));
    }
    {
        TestQuaChecksum32 testQuaChecksum32;
        err = qMax(err, QTest::qExec(&testQuaChecksum32, app.arguments()));
    }
    {
        TestJlCompress testJlCompress;
        err = qMax(err, QTest::qExec(&testJlCompress, app.arguments()));
    }
    {
        TestQuaZipDir testQuaZipDir;
        err = qMax(err, QTest::qExec(&testQuaZipDir, app.arguments()));
    }
    {
        TestQuaZIODevice testQuaZIODevice;
        err = qMax(err, QTest::qExec(&testQuaZIODevice, app.arguments()));
    }
    {
        TestQuaGzipFile testQuaGzipFile;
        err = qMax(err, QTest::qExec(&testQuaGzipFile, app.arguments()));
    }
    {
        TestQuaZipNewInfo testQuaZipNewInfo;
        err = qMax(err, QTest::qExec(&testQuaZipNewInfo, app.arguments()));
    }
    {
        TestQuaZipFileInfo testQuaZipFileInfo;
        err = qMax(err, QTest::qExec(&testQuaZipFileInfo, app.arguments()));
    }
    if (QString(qgetenv("TEST_CR_COMPRESS")) == "true")
    {
      TestJlCpCompress testJlCpCompress;
      err = qMax(err, QTest::qExec(&testJlCpCompress, app.arguments()));
    }
    if (QString(qgetenv("TEST_CR_DECOMPRESS")) == "true")
    {
      TestJlCpExtract testJlCpExtract;
      err = qMax(err, QTest::qExec(&testJlCpExtract, app.arguments()));
    }

    if (err == 0) {
        qDebug("All tests executed successfully");
    } else {
        qWarning("There were errors in some of the tests above.");
    }
    return err;
}
