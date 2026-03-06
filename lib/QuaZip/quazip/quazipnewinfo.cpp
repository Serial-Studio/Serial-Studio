/*
Copyright (C) 2005-2014 Sergey A. Tachenov

This file is part of QuaZip.

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

#include <QtCore/QFileInfo>

#include "quazipnewinfo.h"
#include "quazip_qt_compat.h"

#include <string.h>

static void QuaZipNewInfo_setPermissions(QuaZipNewInfo *info,
        QFile::Permissions perm, bool isDir, bool isSymLink = false)
{
    quint32 uPerm = isDir ? 0040000 : 0100000;

    if ( isSymLink ) {
#ifdef Q_OS_WIN
        uPerm = 0200000;
#else
        uPerm = 0120000;
#endif
    }

    if ((perm & QFile::ReadOwner) != 0)
        uPerm |= 0400;
    if ((perm & QFile::WriteOwner) != 0)
        uPerm |= 0200;
    if ((perm & QFile::ExeOwner) != 0)
        uPerm |= 0100;
    if ((perm & QFile::ReadGroup) != 0)
        uPerm |= 0040;
    if ((perm & QFile::WriteGroup) != 0)
        uPerm |= 0020;
    if ((perm & QFile::ExeGroup) != 0)
        uPerm |= 0010;
    if ((perm & QFile::ReadOther) != 0)
        uPerm |= 0004;
    if ((perm & QFile::WriteOther) != 0)
        uPerm |= 0002;
    if ((perm & QFile::ExeOther) != 0)
        uPerm |= 0001;
    info->externalAttr = (info->externalAttr & ~0xFFFF0000u) | (uPerm << 16);
}

template<typename FileInfo>
void QuaZipNewInfo_init(QuaZipNewInfo &self, const FileInfo &existing)
{
    self.name = existing.name;
    self.dateTime = existing.dateTime;
    self.internalAttr = existing.internalAttr;
    self.externalAttr = existing.externalAttr;
    self.comment = existing.comment;
    self.extraLocal = existing.extra;
    self.extraGlobal = existing.extra;
    self.uncompressedSize = existing.uncompressedSize;
}

QuaZipNewInfo::QuaZipNewInfo(const QuaZipFileInfo &existing)
{
    QuaZipNewInfo_init(*this, existing);
}

QuaZipNewInfo::QuaZipNewInfo(const QuaZipFileInfo64 &existing)
{
    QuaZipNewInfo_init(*this, existing);
}

QuaZipNewInfo::QuaZipNewInfo(const QString& name):
  name(name), dateTime(QDateTime::currentDateTime()), internalAttr(0), externalAttr(0),
  uncompressedSize(0)
{
}

QuaZipNewInfo::QuaZipNewInfo(const QString& name, const QString& file):
  name(name), internalAttr(0), externalAttr(0), uncompressedSize(0)
{
  QFileInfo info(file);
  QDateTime lm = info.lastModified();
  if (!info.exists()) {
    dateTime = QDateTime::currentDateTime();
  } else {
    dateTime = lm;
    QuaZipNewInfo_setPermissions(this, info.permissions(), info.isDir(), quazip_is_symlink(info));
  }
}

QuaZipNewInfo::QuaZipNewInfo(const QString& name, const QString& file, const QDateTime& dateTime):
  name(name), dateTime(dateTime), internalAttr(0), externalAttr(0), uncompressedSize(0)
{
  QFileInfo info(file);
  if (info.exists()) {
    QuaZipNewInfo_setPermissions(this, info.permissions(), info.isDir(), quazip_is_symlink(info));
  }
}

void QuaZipNewInfo::setFileDateTime(const QString& file)
{
  QFileInfo info(file);
  QDateTime lm = info.lastModified();
  if (info.exists())
    dateTime = lm;
}

void QuaZipNewInfo::setFilePermissions(const QString &file)
{
    QFileInfo info = QFileInfo(file);
    QFile::Permissions perm = info.permissions();
    QuaZipNewInfo_setPermissions(this, perm, info.isDir(), quazip_is_symlink(info));
}

void QuaZipNewInfo::setPermissions(QFile::Permissions permissions)
{
    QuaZipNewInfo_setPermissions(this, permissions, name.endsWith(QLatin1String("/")));
}

void QuaZipNewInfo::setFileNTFSTimes(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (!fi.exists()) {
        qWarning("QuaZipNewInfo::setFileNTFSTimes(): '%s' doesn't exist",
                 fileName.toUtf8().constData());
        return;
    }
    setFileNTFSmTime(fi.lastModified());
    setFileNTFSaTime(fi.lastRead());
    setFileNTFScTime(quazip_ctime(fi));
}

static void setNTFSTime(QByteArray &extra, const QDateTime &time, int position,
                        int fineTicks) {
    int ntfsPos = -1, timesPos = -1;
    unsigned ntfsLength = 0, ntfsTimesLength = 0;
    for (int i = 0; i <= extra.size() - 4; ) {
        unsigned type = static_cast<unsigned>(static_cast<unsigned char>(
                                                  extra.at(i)))
                | (static_cast<unsigned>(static_cast<unsigned char>(
                                                  extra.at(i + 1))) << 8);
        i += 2;
        unsigned length = static_cast<unsigned>(static_cast<unsigned char>(
                                                  extra.at(i)))
                | (static_cast<unsigned>(static_cast<unsigned char>(
                                                  extra.at(i + 1))) << 8);
        i += 2;
        if (type == QUAZIP_EXTRA_NTFS_MAGIC) {
            ntfsPos = i - 4; // the beginning of the NTFS record
            ntfsLength = length;
            if (length <= 4) {
                break; // no times in the NTFS record
            }
            i += 4; // reserved
            while (i <= extra.size() - 4) {
                unsigned tag = static_cast<unsigned>(
                            static_cast<unsigned char>(extra.at(i)))
                        | (static_cast<unsigned>(
                               static_cast<unsigned char>(extra.at(i + 1)))
                           << 8);
                i += 2;
                unsigned tagsize = static_cast<unsigned>(
                            static_cast<unsigned char>(extra.at(i)))
                        | (static_cast<unsigned>(
                               static_cast<unsigned char>(extra.at(i + 1)))
                           << 8);
                i += 2;
                if (tag == QUAZIP_EXTRA_NTFS_TIME_MAGIC) {
                    timesPos = i - 4; // the beginning of the NTFS times tag
                    ntfsTimesLength = tagsize;
                    break;
                }
                i += tagsize;
            }
            break; // I ain't going to search for yet another NTFS record!
        }
        i += length;
    }
    if (ntfsPos == -1) {
        // No NTFS record, need to create one.
        ntfsPos = extra.size();
        ntfsLength = 32;
        extra.resize(extra.size() + 4 + ntfsLength);
        // the NTFS record header
        extra[ntfsPos] = static_cast<char>(QUAZIP_EXTRA_NTFS_MAGIC);
        extra[ntfsPos + 1] = static_cast<char>(QUAZIP_EXTRA_NTFS_MAGIC >> 8);
        extra[ntfsPos + 2] = 32; // the 2-byte size in LittleEndian
        extra[ntfsPos + 3] = 0;
        // zero the record
        memset(extra.data() + ntfsPos + 4, 0, 32);
        timesPos = ntfsPos + 8;
        // now set the tag data
        extra[timesPos] = static_cast<char>(QUAZIP_EXTRA_NTFS_TIME_MAGIC);
        extra[timesPos + 1] = static_cast<char>(QUAZIP_EXTRA_NTFS_TIME_MAGIC
                                               >> 8);
        // the size:
        extra[timesPos + 2] = 24;
        extra[timesPos + 3] = 0;
        ntfsTimesLength = 24;
    }
    if (timesPos == -1) {
        // No time tag in the NTFS record, need to add one.
        timesPos = ntfsPos + 4 + ntfsLength;
        extra.resize(extra.size() + 28);
        // Now we need to move the rest of the field
        // (possibly zero bytes, but memmove() is OK with that).
        // 0 ......... ntfsPos .. ntfsPos + 4   ... timesPos
        // <some data> <header>   <NTFS record>     <need-to-move data>    <end>
        memmove(extra.data() + timesPos + 28, extra.data() + timesPos,
                extra.size() - 28 - timesPos);
        ntfsLength += 28;
        // now set the tag data
        extra[timesPos] = static_cast<char>(QUAZIP_EXTRA_NTFS_TIME_MAGIC);
        extra[timesPos + 1] = static_cast<char>(QUAZIP_EXTRA_NTFS_TIME_MAGIC
                                               >> 8);
        // the size:
        extra[timesPos + 2] = 24;
        extra[timesPos + 3] = 0;
        // zero the record
        memset(extra.data() + timesPos + 4, 0, 24);
        ntfsTimesLength = 24;
    }
    if (ntfsTimesLength < 24) {
        // Broken times field. OK, this is really unlikely, but just in case...
        size_t timesEnd = timesPos + 4 + ntfsTimesLength;
        extra.resize(extra.size() + (24 - ntfsTimesLength));
        // Move it!
        // 0 ......... timesPos .... timesPos + 4 .. timesEnd
        // <some data> <time header> <broken times> <need-to-move data> <end>
        memmove(extra.data() + timesEnd + (24 - ntfsTimesLength),
                extra.data() + timesEnd,
                extra.size() - (24 - ntfsTimesLength) - timesEnd);
        // Now we have to increase the NTFS record and time tag lengths.
        ntfsLength += (24 - ntfsTimesLength);
        ntfsTimesLength = 24;
        extra[ntfsPos + 2] = static_cast<char>(ntfsLength);
        extra[ntfsPos + 3] = static_cast<char>(ntfsLength >> 8);
        extra[timesPos + 2] = static_cast<char>(ntfsTimesLength);
        extra[timesPos + 3] = static_cast<char>(ntfsTimesLength >> 8);
    }
    quint64 ticks = quazip_ntfs_ticks(time, fineTicks);
    extra[timesPos + 4 + position] = static_cast<char>(ticks);
    extra[timesPos + 5 + position] = static_cast<char>(ticks >> 8);
    extra[timesPos + 6 + position] = static_cast<char>(ticks >> 16);
    extra[timesPos + 7 + position] = static_cast<char>(ticks >> 24);
    extra[timesPos + 8 + position] = static_cast<char>(ticks >> 32);
    extra[timesPos + 9 + position] = static_cast<char>(ticks >> 40);
    extra[timesPos + 10 + position] = static_cast<char>(ticks >> 48);
    extra[timesPos + 11 + position] = static_cast<char>(ticks >> 56);
}

void QuaZipNewInfo::setFileNTFSmTime(const QDateTime &mTime, int fineTicks)
{
    setNTFSTime(extraLocal, mTime, 0, fineTicks);
    setNTFSTime(extraGlobal, mTime, 0, fineTicks);
}

void QuaZipNewInfo::setFileNTFSaTime(const QDateTime &aTime, int fineTicks)
{
    setNTFSTime(extraLocal, aTime, 8, fineTicks);
    setNTFSTime(extraGlobal, aTime, 8, fineTicks);
}

void QuaZipNewInfo::setFileNTFScTime(const QDateTime &cTime, int fineTicks)
{
    setNTFSTime(extraLocal, cTime, 16, fineTicks);
    setNTFSTime(extraGlobal, cTime, 16, fineTicks);
}
