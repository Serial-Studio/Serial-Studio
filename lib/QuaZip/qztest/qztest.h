#ifndef QUAZIP_TEST_QZTEST_H
#define QUAZIP_TEST_QZTEST_H

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

#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <quazip_qt_compat.h>

extern bool createTestFiles(const QStringList &fileNames,
                            int size = -1,
                            const QString &dir = "tmp");
extern void removeTestFiles(const QStringList &fileNames, const QString
        &dir = "tmp");
extern bool createTestArchive(const QString &zipName, 
                              const QStringList &fileNames, 
                              const QString &dir = "tmp");
extern bool createTestArchive(const QString &zipName,
                              const QStringList &fileNames,
                              QTextCodec *codec,
                              const QString &dir = "tmp");
extern bool createTestArchive(QIODevice *ioDevice,
                              const QStringList &fileNames,
                              QTextCodec *codec,
                              const QString &dir = "tmp");

#endif // QUAZIP_TEST_QZTEST_H
