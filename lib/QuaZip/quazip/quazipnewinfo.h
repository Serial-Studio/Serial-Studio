#ifndef QUA_ZIPNEWINFO_H
#define QUA_ZIPNEWINFO_H

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

Original ZIP package is copyrighted by Gilles Vollant, see
quazip/(un)zip.h files for details, basically it's zlib license.
 **/

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QString>

#include "quazip_global.h"

#include "quazipfileinfo.h"

/// Information about a file to be created.
/** This structure holds information about a file to be created inside
 * ZIP archive. At least name should be set to something correct before
 * passing this structure to
 * QuaZipFile::open(OpenMode,const QuaZipNewInfo&,int,int,bool).
 *
 * Zip64 support of this structure is slightly limited: in the raw mode (when
 * a pre-compressed file is written into a ZIP file as-is), it is necessary
 * to specify the uncompressed file size and the appropriate field is 32 bit.
 * Since the raw mode is used extremely rare, there is no real need to have
 * a separate QuaZipNewInfo64 structure like QuaZipFileInfo64. It may be added
 * in the future though, if there is a demand for the raw mode with zip64
 * archives.
 **/
struct QUAZIP_EXPORT QuaZipNewInfo {
  /// File name.
  /** This field holds file name inside archive, including path relative
   * to archive root.
   **/
  QString name;
  /// File timestamp.
  /** This is the last file modification date and time. Will be stored
   * in the archive central directory. It is a good practice to set it
   * to the source file timestamp instead of archive creating time. Use
   * setFileDateTime() or QuaZipNewInfo(const QString&, const QString&).
   **/
  QDateTime dateTime;
  /// File internal attributes.
  quint16 internalAttr;
  /// File external attributes.
  /**
    The highest 16 bits contain Unix file permissions and type (dir or
    file). The constructor QuaZipNewInfo(const QString&, const QString&)
    takes permissions from the provided file.
    */
  quint32 externalAttr;
  /// File comment.
  /** Will be encoded in UTF-8 encoding.
   **/
  QString comment;
  /// File local extra field.
  QByteArray extraLocal;
  /// File global extra field.
  QByteArray extraGlobal;
  /// Uncompressed file size.
  /** This is only needed if you are using raw file zipping mode, i. e.
   * adding precompressed file in the zip archive.
   **/
  ulong uncompressedSize;
  /// Constructs QuaZipNewInfo instance.
  /** Initializes name with \a name, dateTime with current date and
   * time. Attributes are initialized with zeros, comment and extra
   * field with null values.
   **/
  QuaZipNewInfo(const QString& name);
  /// Constructs QuaZipNewInfo instance.
  /** Initializes name with \a name. Timestamp and permissions are taken
   * from the specified file. If the \a file does not exists or its timestamp
   * is inaccessible (e. g. you do not have read permission for the
   * directory file in), uses current time and zero permissions. Other attributes are
   * initialized with zeros, comment and extra field with null values.
   * \sa setFileDateTime()
   **/
  QuaZipNewInfo(const QString& name, const QString& file);
  /// Constructs QuaZipNewInfo instance.
  /** Initializes name with \a name and provided timestamp.  Permissions are taken
   * from the specified file. If the \a file does not exists or
   * is inaccessible (e. g. you do not have read permission for the
   * directory file in), uses zero permissions. Other attributes are
   * initialized with zeros, comment and extra field with null values.
   * \sa setFileDateTime()
   **/
  QuaZipNewInfo(const QString& name, const QString& file, const QDateTime& dateTime);
  /// Initializes the new instance from existing file info.
  /** Mainly used when copying files between archives.
   *
   * Both extra fields are initialized to existing.extra.
   * @brief QuaZipNewInfo
   * @param existing
   */
  QuaZipNewInfo(const QuaZipFileInfo &existing);
  /// Initializes the new instance from existing file info.
  /** Mainly used when copying files between archives.
   *
   * Both extra fields are initialized to existing.extra.
   * @brief QuaZipNewInfo
   * @param existing
   */
  QuaZipNewInfo(const QuaZipFileInfo64 &existing);
  /// Sets the file timestamp from the existing file.
  /** Use this function to set the file timestamp from the existing
   * file. Use it like this:
   * \code
   * QuaZipFile zipFile(&zip);
   * QFile file("file-to-add");
   * file.open(QIODevice::ReadOnly);
   * QuaZipNewInfo info("file-name-in-archive");
   * info.setFileDateTime("file-to-add"); // take the timestamp from file
   * zipFile.open(QIODevice::WriteOnly, info);
   * \endcode
   *
   * This function does not change dateTime if some error occured (e. g.
   * file is inaccessible).
   **/
  void setFileDateTime(const QString& file);
  /// Sets the file permissions from the existing file.
  /**
    Takes permissions from the file and sets the high 16 bits of
    external attributes. Uses QFileInfo to get permissions on all
    platforms.
    */
  void setFilePermissions(const QString &file);
  /// Sets the file permissions.
  /**
    Modifies the highest 16 bits of external attributes. The type part
    is set to dir if the name ends with a slash, and to regular file
    otherwise.
    */
  void setPermissions(QFile::Permissions permissions);
  /// Sets the NTFS times from an existing file.
  /**
   * If the file doesn't exist, a warning is printed to the stderr and nothing
   * is done. Otherwise, all three times, as reported by
   * QFileInfo::lastModified(), QFileInfo::lastRead() and
   * QFileInfo::birthTime() (>=Qt5.10) or QFileInfo::created(), are written to
   * the NTFS extra field record.
   *
   * The NTFS record is written to
   * both the local and the global extra fields, updating the existing record
   * if there is one, or creating a new one and appending it to the end
   * of each extra field.
   *
   * The microseconds will be zero, as they aren't reported by QFileInfo.
   * @param fileName
   */
  void setFileNTFSTimes(const QString &fileName);
  /// Sets the NTFS modification time.
  /**
   * The time is written into the NTFS record in
   * both the local and the global extra fields, updating the existing record
   * if there is one, or creating a new one and appending it to the end
   * of each extra field. When updating an existing record, all other fields
   * are left intact.
   * @param mTime The new modification time.
   * @param fineTicks The fractional part of milliseconds, in 100-nanosecond
   *        ticks (i. e. 9999 ticks = 999.9 microsecond). Values greater than
   *        9999 will add milliseconds or even seconds, but this can be
   *        confusing and therefore is discouraged.
   */
  void setFileNTFSmTime(const QDateTime &mTime, int fineTicks = 0);
  /// Sets the NTFS access time.
  /**
   * The time is written into the NTFS record in
   * both the local and the global extra fields, updating the existing record
   * if there is one, or creating a new one and appending it to the end
   * of each extra field. When updating an existing record, all other fields
   * are left intact.
   * @param aTime The new access time.
   * @param fineTicks The fractional part of milliseconds, in 100-nanosecond
   *        ticks (i. e. 9999 ticks = 999.9 microsecond). Values greater than
   *        9999 will add milliseconds or even seconds, but this can be
   *        confusing and therefore is discouraged.
   */
  void setFileNTFSaTime(const QDateTime &aTime, int fineTicks = 0);
  /// Sets the NTFS creation time.
  /**
   * The time is written into the NTFS record in
   * both the local and the global extra fields, updating the existing record
   * if there is one, or creating a new one and appending it to the end
   * of each extra field. When updating an existing record, all other fields
   * are left intact.
   * @param cTime The new creation time.
   * @param fineTicks The fractional part of milliseconds, in 100-nanosecond
   *        ticks (i. e. 9999 ticks = 999.9 microsecond). Values greater than
   *        9999 will add milliseconds or even seconds, but this can be
   *        confusing and therefore is discouraged.
   */
  void setFileNTFScTime(const QDateTime &cTime, int fineTicks = 0);
};

#endif
