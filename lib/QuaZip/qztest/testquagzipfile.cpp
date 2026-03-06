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

#include "testquagzipfile.h"
#include <zlib.h>
#include <QtCore/QDir>
#include <quagzipfile.h>
#include <QtTest/QtTest>

void TestQuaGzipFile::read()
{
    QDir curDir;
    curDir.mkpath("tmp");
    gzFile file = gzopen("tmp/test.gz", "wb");
    gzwrite(file, "test", 4);
    gzclose(file);
    QuaGzipFile testFile("tmp/test.gz");
    QVERIFY(testFile.open(QIODevice::ReadOnly));
    char buf[5];
    buf[4] = '\0';
    QCOMPARE(testFile.read(buf, 5), static_cast<qint64>(4));
    testFile.close();
    QVERIFY(!testFile.isOpen());
    QCOMPARE(static_cast<const char*>(buf), "test");
    curDir.remove("tmp/test.gz");
    curDir.rmdir("tmp");
}

void TestQuaGzipFile::write()
{
    QDir curDir;
    curDir.mkpath("tmp");
    QuaGzipFile testFile;
    testFile.setFileName("tmp/test.gz");
    QCOMPARE(testFile.getFileName(), QString::fromLatin1("tmp/test.gz"));
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    QCOMPARE(testFile.write("test", 4), static_cast<qint64>(4));
    QVERIFY(testFile.flush());
    testFile.close();
    QVERIFY(!testFile.isOpen());
    gzFile file = gzopen("tmp/test.gz", "rb");
    char buf[5];
    buf[4] = '\0';
    QCOMPARE(gzread(file, buf, 5), 4);
    gzclose(file);
    QCOMPARE(static_cast<const char*>(buf), "test");
    curDir.remove("tmp/test.gz");
    curDir.rmdir("tmp");
}

void TestQuaGzipFile::constructorDestructor()
{
    QuaGzipFile *f1 = new QuaGzipFile();
    delete f1; // D0 destructor
    QObject parent;
    QuaGzipFile f2(&parent);
    QuaGzipFile f3("test.gz", &parent);
}
