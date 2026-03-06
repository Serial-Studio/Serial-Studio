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

#include "testquaziodevice.h"
#include <quaziodevice.h>
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtTest/QtTest>

void TestQuaZIODevice::read()
{
    QByteArray buf(256, 0);
    z_stream zouts;
    zouts.zalloc = (alloc_func) NULL;
    zouts.zfree = (free_func) NULL;
    zouts.opaque = NULL;
    deflateInit(&zouts, Z_DEFAULT_COMPRESSION);
    zouts.next_in = reinterpret_cast<Bytef*>(const_cast<char*>("test"));
    zouts.avail_in = 4;
    zouts.next_out = reinterpret_cast<Bytef*>(buf.data());
    zouts.avail_out = buf.size();
    deflate(&zouts, Z_FINISH);
    deflateEnd(&zouts);
    QBuffer testBuffer(&buf);
    testBuffer.open(QIODevice::ReadOnly);
    QuaZIODevice testDevice(&testBuffer);
    QVERIFY(testDevice.open(QIODevice::ReadOnly));
    char outBuf[5];
    QCOMPARE(testDevice.read(outBuf, 5), static_cast<qint64>(4));
    testDevice.close();
    QVERIFY(!testDevice.isOpen());
}

void TestQuaZIODevice::readMany()
{
    QByteArray buf(256, 0);
    z_stream zouts;
    zouts.zalloc = (alloc_func) NULL;
    zouts.zfree = (free_func) NULL;
    zouts.opaque = NULL;
    deflateInit(&zouts, Z_DEFAULT_COMPRESSION);
    zouts.next_in = reinterpret_cast<Bytef*>(const_cast<char*>("testtest"));
    zouts.avail_in = 8;
    zouts.next_out = reinterpret_cast<Bytef*>(buf.data());
    zouts.avail_out = buf.size();
    deflate(&zouts, Z_FINISH);
    deflateEnd(&zouts);
    QBuffer testBuffer(&buf);
    testBuffer.open(QIODevice::ReadOnly);
    QuaZIODevice testDevice(&testBuffer);
    QVERIFY(testDevice.open(QIODevice::ReadOnly));
    char outBuf[4];
    QCOMPARE(testDevice.read(outBuf, 4), static_cast<qint64>(4));
    QVERIFY(!testDevice.atEnd());
    QVERIFY(testDevice.bytesAvailable() > 0);
    QCOMPARE(testDevice.read(4).size(), 4);
    QCOMPARE(testDevice.bytesAvailable(), static_cast<qint64>(0));
    QVERIFY(testDevice.atEnd());
    testDevice.close();
    QVERIFY(!testDevice.isOpen());
}

void TestQuaZIODevice::write()
{
    QByteArray buf(256, 0);
    QBuffer testBuffer(&buf);
    testBuffer.open(QIODevice::WriteOnly);
    QuaZIODevice *testDevice = new QuaZIODevice(&testBuffer);
    QCOMPARE(testDevice->getIoDevice(), &testBuffer);
    QVERIFY(testDevice->open(QIODevice::WriteOnly));
    QCOMPARE(testDevice->write("test", 4), static_cast<qint64>(4));
    testDevice->close();
    QVERIFY(!testDevice->isOpen());
    z_stream zins;
    zins.zalloc = (alloc_func) NULL;
    zins.zfree = (free_func) NULL;
    zins.opaque = NULL;
    inflateInit(&zins);
    zins.next_in = reinterpret_cast<Bytef*>(buf.data());
    zins.avail_in = testBuffer.pos();
    char outBuf[5];
    zins.next_out = reinterpret_cast<Bytef*>(outBuf);
    zins.avail_out = 5;
    int result = inflate(&zins, Z_FINISH);
    QCOMPARE(result, Z_STREAM_END);
    inflateEnd(&zins);
    int size = 5 - zins.avail_out;
    QCOMPARE(size, 4);
    outBuf[4] = '\0';
    QCOMPARE(static_cast<const char*>(outBuf), "test");
    delete testDevice; // Test D0 destructor
}
