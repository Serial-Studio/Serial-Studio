#ifndef QUA_ZIPFILEINFO_H
#define QUA_ZIPFILEINFO_H

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

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QHash>

#include "quazip_global.h"

/// The typedef to store extra field parse results
typedef QHash<quint16, QList<QByteArray> > QuaExtraFieldHash;

/// Information about a file inside archive.
/**
 * \deprecated Use QuaZipFileInfo64 instead. Not only it supports large files,
 * but also more convenience methods as well.
 *
 * Call QuaZip::getCurrentFileInfo() or QuaZipFile::getFileInfo() to
 * fill this structure. */
struct QUAZIP_EXPORT QuaZipFileInfo {
  /// File name.
  QString name;
  /// Version created by.
  quint16 versionCreated;
  /// Version needed to extract.
  quint16 versionNeeded;
  /// General purpose flags.
  quint16 flags;
  /// Compression method.
  quint16 method;
  /// Last modification date and time.
  QDateTime dateTime;
  /// CRC.
  quint32 crc;
  /// Compressed file size.
  quint32 compressedSize;
  /// Uncompressed file size.
  quint32 uncompressedSize;
  /// Disk number start.
  quint16 diskNumberStart;
  /// Internal file attributes.
  quint16 internalAttr;
  /// External file attributes.
  quint32 externalAttr;
  /// Comment.
  QString comment;
  /// Extra field.
  QByteArray extra;
  /// Get the file permissions.
  /**
    Returns the high 16 bits of external attributes converted to
    QFile::Permissions.
    */
  QFile::Permissions getPermissions() const;
};

/// Information about a file inside archive (with zip64 support).
/** Call QuaZip::getCurrentFileInfo() or QuaZipFile::getFileInfo() to
 * fill this structure. */
struct QUAZIP_EXPORT QuaZipFileInfo64 {
  /// File name.
  QString name;
  /// Version created by.
  quint16 versionCreated;
  /// Version needed to extract.
  quint16 versionNeeded;
  /// General purpose flags.
  quint16 flags;
  /// Compression method.
  quint16 method;
  /// Last modification date and time.
  /**
   * This is the time stored in the standard ZIP header. This format only allows
   * to store time with 2-second precision, so the seconds will always be even
   * and the milliseconds will always be zero. If you need more precise
   * date and time, you can try to call the getNTFSmTime() function or
   * its siblings, provided that the archive itself contains these NTFS times.
   */
  QDateTime dateTime;
  /// CRC.
  quint32 crc;
  /// Compressed file size.
  quint64 compressedSize;
  /// Uncompressed file size.
  quint64 uncompressedSize;
  /// Disk number start.
  quint16 diskNumberStart;
  /// Internal file attributes.
  quint16 internalAttr;
  /// External file attributes.
  quint32 externalAttr;
  /// Comment.
  QString comment;
  /// Extra field.
  QByteArray extra;
  /// Get the file permissions.
  /**
    Returns the high 16 bits of external attributes converted to
    QFile::Permissions.
    */
  QFile::Permissions getPermissions() const;
  /// Checks whether the file is a symbolic link.
  /**
    Returns true iff the highest 16 bits of the external attributes
    indicate that the file is a symbolic link according to Unix file mode.
   */
  bool isSymbolicLink() const;
  /// Converts to QuaZipFileInfo
  /**
    If any of the fields are greater than 0xFFFFFFFFu, they are set to
    0xFFFFFFFFu exactly, not just truncated. This function should be mainly used
    for compatibility with the old code expecting QuaZipFileInfo, in the cases
    when it's impossible or otherwise unadvisable (due to ABI compatibility
    reasons, for example) to modify that old code to use QuaZipFileInfo64.

    \return \c true if all fields converted correctly, \c false if an overflow
    occured.
    */
  bool toQuaZipFileInfo(QuaZipFileInfo &info) const;
  /// Returns the NTFS modification time
  /**
   * The getNTFS*Time() functions only work if there is an NTFS extra field
   * present. Otherwise, they all return invalid null timestamps.
   * @param fineTicks If not null, the fractional part of milliseconds returned
   *                  there, measured in 100-nanosecond ticks. Will be set to
   *                  zero if there is no NTFS extra field.
   * @sa dateTime
   * @sa getNTFSaTime()
   * @sa getNTFScTime()
   * @return The NTFS modification time, UTC
   */
  QDateTime getNTFSmTime(int *fineTicks = nullptr) const;
  /// Returns the NTFS access time
  /**
   * The getNTFS*Time() functions only work if there is an NTFS extra field
   * present. Otherwise, they all return invalid null timestamps.
   * @param fineTicks If not null, the fractional part of milliseconds returned
   *                  there, measured in 100-nanosecond ticks. Will be set to
   *                  zero if there is no NTFS extra field.
   * @sa dateTime
   * @sa getNTFSmTime()
   * @sa getNTFScTime()
   * @return The NTFS access time, UTC
   */
  QDateTime getNTFSaTime(int *fineTicks = nullptr) const;
  /// Returns the NTFS creation time
  /**
   * The getNTFS*Time() functions only work if there is an NTFS extra field
   * present. Otherwise, they all return invalid null timestamps.
   * @param fineTicks If not null, the fractional part of milliseconds returned
   *                  there, measured in 100-nanosecond ticks. Will be set to
   *                  zero if there is no NTFS extra field.
   * @sa dateTime
   * @sa getNTFSmTime()
   * @sa getNTFSaTime()
   * @return The NTFS creation time, UTC
   */
  QDateTime getNTFScTime(int *fineTicks = nullptr) const;
  /// Returns the extended modification timestamp
  /**
   * The getExt*Time() functions only work if there is an extended timestamp
   * extra field (ID 0x5455) present. Otherwise, they all return invalid null
   * timestamps.
   *
   * QuaZipFileInfo64 only contains the modification time because it's extracted
   * from @ref extra, which contains the global extra field, and access and
   * creation time are in the local header which can be accessed through
   * @ref QuaZipFile.
   *
   * @sa dateTime
   * @sa QuaZipFile::getExtModTime()
   * @sa QuaZipFile::getExtAcTime()
   * @sa QuaZipFile::getExtCrTime()
   * @return The extended modification time, UTC
   */
  QDateTime getExtModTime() const;
  /// Checks whether the file is encrypted.
  bool isEncrypted() const {return (flags & 1) != 0;}
  /// Parses extra field
  /**
   * The returned hash table contains a list of data blocks for every header ID
   * in the provided extra field. The number of data blocks in a hash table value
   * equals to the number of occurrences of the appropriate header id. In most cases,
   * a block with a specific header ID only occurs once, and therefore the returned
   * hash table will contain a list consisting of a single element for that header ID.
   *
   * @param extraField extra field to parse
   * @return header id to list of data block hash
   */
  static QuaExtraFieldHash parseExtraField(const QByteArray &extraField);
  /// Extracts extended time from the extra field
  /**
   * Utility function used by various getExt*Time() functions, but can be used directly
   * if the extra field is obtained elsewhere (from a third party library, for example).
   *
   * @param extra the extra field for a file
   * @param flag 1 - modification time, 2 - access time, 4 - creation time
   * @return the extracted time or null QDateTime if not present
   * @sa getExtModTime()
   * @sa QuaZipFile::getExtModTime()
   * @sa QuaZipFile::getExtAcTime()
   * @sa QuaZipFile::getExtCrTime()
   */
  static QDateTime getExtTime(const QByteArray &extra, int flag);
};

#endif
