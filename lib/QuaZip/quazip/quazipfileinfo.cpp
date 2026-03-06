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

#include "quazipfileinfo.h"

#include "quazip_qt_compat.h"

#include <QtCore/QDataStream>

static QFile::Permissions permissionsFromExternalAttr(quint32 externalAttr) {
    quint32 uPerm = (externalAttr & 0xFFFF0000u) >> 16;
    QFile::Permissions perm;
    if ((uPerm & 0400) != 0)
        perm |= QFile::ReadOwner;
    if ((uPerm & 0200) != 0)
        perm |= QFile::WriteOwner;
    if ((uPerm & 0100) != 0)
        perm |= QFile::ExeOwner;
    if ((uPerm & 0040) != 0)
        perm |= QFile::ReadGroup;
    if ((uPerm & 0020) != 0)
        perm |= QFile::WriteGroup;
    if ((uPerm & 0010) != 0)
        perm |= QFile::ExeGroup;
    if ((uPerm & 0004) != 0)
        perm |= QFile::ReadOther;
    if ((uPerm & 0002) != 0)
        perm |= QFile::WriteOther;
    if ((uPerm & 0001) != 0)
        perm |= QFile::ExeOther;
    return perm;

}

QFile::Permissions QuaZipFileInfo::getPermissions() const
{
    return permissionsFromExternalAttr(externalAttr);
}

QFile::Permissions QuaZipFileInfo64::getPermissions() const
{
    return permissionsFromExternalAttr(externalAttr);
}

bool QuaZipFileInfo64::isSymbolicLink() const
{
    quint32 uPerm = (externalAttr & 0xFFFF0000u) >> 16;
    return (uPerm & 0170000) == 0120000;
}

bool QuaZipFileInfo64::toQuaZipFileInfo(QuaZipFileInfo &info) const
{
    bool noOverflow = true;
    info.name = name;
    info.versionCreated = versionCreated;
    info.versionNeeded = versionNeeded;
    info.flags = flags;
    info.method = method;
    info.dateTime = dateTime;
    info.crc = crc;
    if (compressedSize > 0xFFFFFFFFu) {
        info.compressedSize = 0xFFFFFFFFu;
        noOverflow = false;
    } else {
        info.compressedSize = compressedSize;
    }
    if (uncompressedSize > 0xFFFFFFFFu) {
        info.uncompressedSize = 0xFFFFFFFFu;
        noOverflow = false;
    } else {
        info.uncompressedSize = uncompressedSize;
    }
    info.diskNumberStart = diskNumberStart;
    info.internalAttr = internalAttr;
    info.externalAttr = externalAttr;
    info.comment = comment;
    info.extra = extra;
    return noOverflow;
}

static QDateTime getNTFSTime(const QByteArray &extra, int position,
                             int *fineTicks)
{
    QDateTime dateTime;
    QuaExtraFieldHash extraHash = QuaZipFileInfo64::parseExtraField(extra);
    QList<QByteArray> ntfsExtraFields = extraHash[QUAZIP_EXTRA_NTFS_MAGIC];
    if (ntfsExtraFields.isEmpty())
        return dateTime;
    QByteArray ntfsExtraField = ntfsExtraFields.at(0);
    if (ntfsExtraField.length() <= 4)
        return dateTime;
    QByteArray ntfsAttributes = ntfsExtraField.mid(4);
    QuaExtraFieldHash ntfsHash = QuaZipFileInfo64::parseExtraField(ntfsAttributes);
    QList<QByteArray> ntfsTimeAttributes = ntfsHash[QUAZIP_EXTRA_NTFS_TIME_MAGIC];
    if (ntfsTimeAttributes.isEmpty())
        return dateTime;
    QByteArray ntfsTimes = ntfsTimeAttributes.at(0);
    if (ntfsTimes.size() < 24)
        return dateTime;
    QDataStream timeReader(ntfsTimes);
    timeReader.setByteOrder(QDataStream::LittleEndian);
    timeReader.device()->seek(position);
    quint64 time;
    timeReader >> time;
    if (time == 0)
        return dateTime;
    QDateTime base = quazip_since_epoch_ntfs();
    dateTime = base.addMSecs(time / 10000);
    if (fineTicks != nullptr) {
        *fineTicks = static_cast<int>(time % 10000);
    }
    return dateTime;
}

QDateTime QuaZipFileInfo64::getNTFSmTime(int *fineTicks) const
{
    return getNTFSTime(extra, 0, fineTicks);
}

QDateTime QuaZipFileInfo64::getNTFSaTime(int *fineTicks) const
{
    return getNTFSTime(extra, 8, fineTicks);
}

QDateTime QuaZipFileInfo64::getNTFScTime(int *fineTicks) const
{
    return getNTFSTime(extra, 16, fineTicks);
}

QDateTime QuaZipFileInfo64::getExtTime(const QByteArray &extra, int flag)
{
    QDateTime dateTime;
    QuaExtraFieldHash extraHash = QuaZipFileInfo64::parseExtraField(extra);
    QList<QByteArray> extTimeFields = extraHash[QUAZIP_EXTRA_EXT_TIME_MAGIC];
    if (extTimeFields.isEmpty())
        return dateTime;
    QByteArray extTimeField = extTimeFields.at(0);
    if (extTimeField.length() < 1)
        return dateTime;
    QDataStream input(extTimeField);
    input.setByteOrder(QDataStream::LittleEndian);
    quint8 flags;
    input >> flags;
    int flagsRemaining = flags;
    while (!input.atEnd()) {
        int nextFlag = flagsRemaining & -flagsRemaining;
        flagsRemaining &= flagsRemaining - 1;
        qint32 time;
        input >> time;
        if (nextFlag == flag) {
            QDateTime base = quazip_since_epoch();
            dateTime = base.addSecs(time);
            return dateTime;
        }
    }
    return dateTime;
}

QDateTime QuaZipFileInfo64::getExtModTime() const
{
    return getExtTime(extra, 1);
}

QuaExtraFieldHash QuaZipFileInfo64::parseExtraField(const QByteArray &extraField)
{
    QDataStream input(extraField);
    input.setByteOrder(QDataStream::LittleEndian);
    QHash<quint16, QList<QByteArray> > result;
    while (!input.atEnd()) {
        quint16 id, size;
        input >> id;
        if (input.status() == QDataStream::ReadPastEnd)
            return result;
        input >> size;
        if (input.status() == QDataStream::ReadPastEnd)
            return result;
        QByteArray data;
        data.resize(size);
        int read = input.readRawData(data.data(), data.size());
        if (read < data.size())
            return result;
        result[id] << data;
    }
    return result;
}
