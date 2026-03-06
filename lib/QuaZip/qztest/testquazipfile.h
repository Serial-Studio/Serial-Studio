#ifndef QUAZIP_TEST_QUAZIPFILE_H
#define QUAZIP_TEST_QUAZIPFILE_H

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

#include <QtCore/QObject>
#include <QtCore/QStringList>

class TestQuaZipFile: public QObject {
    Q_OBJECT
private slots:
    void zipUnzip_data();
    void zipUnzip();
    void bytesAvailable_data();
    void bytesAvailable();
    void atEnd_data();
    void atEnd();
    void posRead_data();
    void posRead();
    void posWrite_data();
    void posWrite();
    void getZip();
    void setZipName();
    void getFileInfo();
    void setFileName();
    void constructorDestructor();
    void setFileAttrs();
    void largeFile();
};

#endif // QUAZIP_TEST_QUAZIPFILE_H
