#ifndef QUAZIP_TEST_QUAZIP_H
#define QUAZIP_TEST_QUAZIP_H

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

#if (QT_VERSION >= 0x050100)
#define QUAZIP_TEST_QSAVEFILE
#endif

class TestQuaZip: public QObject {
    Q_OBJECT
private slots:
    void getFileList_data();
    void getFileList();
    void add_data();
    void add();
    void setFileNameCodec_data();
    void setFileNameCodec();
    void setOsCode_data();
    void setOsCode();
    void setDataDescriptorWritingEnabled();
    void testQIODeviceAPI();
    void setZipName();
    void setIoDevice();
    void setCommentCodec();
    void setAutoClose();
#ifdef QUAZIP_TEST_QSAVEFILE
    void saveFileBug();
#endif
    void testSequential();
};

#endif // QUAZIP_TEST_QUAZIP_H
