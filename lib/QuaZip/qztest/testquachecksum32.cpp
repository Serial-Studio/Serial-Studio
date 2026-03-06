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

#include "testquachecksum32.h"

#include <quaadler32.h>
#include <quacrc32.h>

#include <QtTest/QtTest>

void TestQuaChecksum32::calculate()
{
    QuaCrc32 crc32;
    QCOMPARE(crc32.calculate("Wikipedia"), 0xADAAC02Eu);
    QuaAdler32 adler32;
    QCOMPARE(adler32.calculate("Wikipedia"), 0x11E60398u);
}

void TestQuaChecksum32::update()
{
    QuaCrc32 crc32;
    crc32.update("Wiki");
    crc32.update("pedia");
    QCOMPARE(crc32.value(), 0xADAAC02Eu);
    QuaAdler32 adler32;
    adler32.update("Wiki");
    adler32.update("pedia");
    QCOMPARE(adler32.value(), 0x11E60398u);
}
