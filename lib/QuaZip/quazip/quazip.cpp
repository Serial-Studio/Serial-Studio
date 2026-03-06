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

#include <QtCore/QFile>
#include <QtCore/QFlags>
#include <QtCore/QHash>

#include "quazip.h"

#define QUAZIP_OS_UNIX 3u

/// All the internal stuff for the QuaZip class.
/**
  \internal

  This class keeps all the private stuff for the QuaZip class so it can
  be changed without breaking binary compatibility, according to the
  Pimpl idiom.
  */
class QuaZipPrivate {
  friend class QuaZip;
  private:
    Q_DISABLE_COPY(QuaZipPrivate)
    /// The pointer to the corresponding QuaZip instance.
    QuaZip *q;
    /// The codec for file names (used when UTF-8 is not enabled).
    QTextCodec *fileNameCodec;
    /// The codec for comments (used when UTF-8 is not enabled).
    QTextCodec *commentCodec;
    /// The archive file name.
    QString zipName;
    /// The device to access the archive.
    QIODevice *ioDevice;
    /// The global comment.
    QString comment;
    /// The open mode.
    QuaZip::Mode mode;
    union {
      /// The internal handle for UNZIP modes.
      unzFile unzFile_f;
      /// The internal handle for ZIP modes.
      zipFile zipFile_f;
    };
    /// Whether a current file is set.
    bool hasCurrentFile_f;
    /// The last error.
    int zipError;
    /// Whether \ref QuaZip::setDataDescriptorWritingEnabled() "the data descriptor writing mode" is enabled.
    bool dataDescriptorWritingEnabled;
    /// The zip64 mode.
    bool zip64;
    /// The auto-close flag.
    bool autoClose;
    /// The UTF-8 flag.
    bool utf8;
    /// The OS code.
    uint osCode;
    inline QTextCodec *getDefaultFileNameCodec()
    {
        if (defaultFileNameCodec == nullptr) {
          return QTextCodec::codecForLocale();
        }
        return defaultFileNameCodec;
    }
    /// The constructor for the corresponding QuaZip constructor.
    inline QuaZipPrivate(QuaZip *q):
      q(q),
      fileNameCodec(getDefaultFileNameCodec()),
      commentCodec(QTextCodec::codecForLocale()),
      ioDevice(nullptr),
      mode(QuaZip::mdNotOpen),
      hasCurrentFile_f(false),
      zipError(UNZ_OK),
      dataDescriptorWritingEnabled(true),
      zip64(false),
      autoClose(true),
      utf8(false),
      osCode(defaultOsCode)
    {
        unzFile_f = nullptr;
        zipFile_f = nullptr;
        lastMappedDirectoryEntry.num_of_file = 0;
        lastMappedDirectoryEntry.pos_in_zip_directory = 0;
    }
    /// The constructor for the corresponding QuaZip constructor.
    inline QuaZipPrivate(QuaZip *q, const QString &zipName):
      q(q),
      fileNameCodec(getDefaultFileNameCodec()),
      commentCodec(QTextCodec::codecForLocale()),
      zipName(zipName),
      ioDevice(nullptr),
      mode(QuaZip::mdNotOpen),
      hasCurrentFile_f(false),
      zipError(UNZ_OK),
      dataDescriptorWritingEnabled(true),
      zip64(false),
      autoClose(true),
      utf8(false),
      osCode(defaultOsCode)
    {
        unzFile_f = nullptr;
        zipFile_f = nullptr;
        lastMappedDirectoryEntry.num_of_file = 0;
        lastMappedDirectoryEntry.pos_in_zip_directory = 0;
    }
    /// The constructor for the corresponding QuaZip constructor.
    inline QuaZipPrivate(QuaZip *q, QIODevice *ioDevice):
      q(q),
      fileNameCodec(getDefaultFileNameCodec()),
      commentCodec(QTextCodec::codecForLocale()),
      ioDevice(ioDevice),
      mode(QuaZip::mdNotOpen),
      hasCurrentFile_f(false),
      zipError(UNZ_OK),
      dataDescriptorWritingEnabled(true),
      zip64(false),
      autoClose(true),
      utf8(false),
      osCode(defaultOsCode)
    {
        unzFile_f = nullptr;
        zipFile_f = nullptr;
        lastMappedDirectoryEntry.num_of_file = 0;
        lastMappedDirectoryEntry.pos_in_zip_directory = 0;
    }
    /// Returns either a list of file names or a list of QuaZipFileInfo.
    template<typename TFileInfo>
        bool getFileInfoList(QList<TFileInfo> *result) const;

    /// Stores map of filenames and file locations for unzipping
      inline void clearDirectoryMap();
      inline void addCurrentFileToDirectoryMap(const QString &fileName);
      bool goToFirstUnmappedFile();
      QHash<QString, unz64_file_pos> directoryCaseSensitive;
      QHash<QString, unz64_file_pos> directoryCaseInsensitive;
      unz64_file_pos lastMappedDirectoryEntry;
      static QTextCodec *defaultFileNameCodec;
      static uint defaultOsCode;
};

QTextCodec *QuaZipPrivate::defaultFileNameCodec = nullptr;
uint QuaZipPrivate::defaultOsCode = QUAZIP_OS_UNIX;

void QuaZipPrivate::clearDirectoryMap()
{
    directoryCaseInsensitive.clear();
    directoryCaseSensitive.clear();
    lastMappedDirectoryEntry.num_of_file = 0;
    lastMappedDirectoryEntry.pos_in_zip_directory = 0;
}

void QuaZipPrivate::addCurrentFileToDirectoryMap(const QString &fileName)
{
    if (!hasCurrentFile_f || fileName.isEmpty()) {
        return;
    }
    // Adds current file to filename map as fileName
    unz64_file_pos fileDirectoryPos;
    unzGetFilePos64(unzFile_f, &fileDirectoryPos);
    directoryCaseSensitive.insert(fileName, fileDirectoryPos);
    // Only add lowercase to directory map if not already there
    // ensures only map the first one seen
    QString lower = fileName.toLower();
    if (!directoryCaseInsensitive.contains(lower))
        directoryCaseInsensitive.insert(lower, fileDirectoryPos);
    // Mark last one
    if (fileDirectoryPos.pos_in_zip_directory > lastMappedDirectoryEntry.pos_in_zip_directory)
        lastMappedDirectoryEntry = fileDirectoryPos;
}

bool QuaZipPrivate::goToFirstUnmappedFile()
{
    zipError = UNZ_OK;
    if (mode != QuaZip::mdUnzip) {
        qWarning("QuaZipPrivate::goToNextUnmappedFile(): ZIP is not open in mdUnzip mode");
        return false;
    }
    // If not mapped anything, go to beginning
    if (lastMappedDirectoryEntry.pos_in_zip_directory == 0) {
        unzGoToFirstFile(unzFile_f);
    } else {
        // Goto the last one mapped, plus one
        unzGoToFilePos64(unzFile_f, &lastMappedDirectoryEntry);
        unzGoToNextFile(unzFile_f);
    }
    hasCurrentFile_f=zipError==UNZ_OK;
    if(zipError==UNZ_END_OF_LIST_OF_FILE)
      zipError=UNZ_OK;
    return hasCurrentFile_f;
}

QuaZip::QuaZip():
  p(new QuaZipPrivate(this))
{
}

QuaZip::QuaZip(const QString& zipName):
  p(new QuaZipPrivate(this, zipName))
{
}

QuaZip::QuaZip(QIODevice *ioDevice):
  p(new QuaZipPrivate(this, ioDevice))
{
}

QuaZip::~QuaZip()
{
  if(isOpen())
    close();
  delete p;
}

bool QuaZip::open(Mode mode, zlib_filefunc_def* ioApi)
{
  p->zipError=UNZ_OK;
  if(isOpen()) {
    qWarning("QuaZip::open(): ZIP already opened");
    return false;
  }
  QIODevice *ioDevice = p->ioDevice;
  if (ioDevice == nullptr) {
    if (p->zipName.isEmpty()) {
      qWarning("QuaZip::open(): set either ZIP file name or IO device first");
      return false;
    }
    ioDevice = new QFile(p->zipName);
  }
  unsigned flags = 0;
  switch(mode) {
    case mdUnzip:
      if (ioApi == nullptr) {
          if (p->autoClose)
              flags |= UNZ_AUTO_CLOSE;
          p->unzFile_f=unzOpenInternal(ioDevice, nullptr, 1, flags);
      } else {
          // QuaZip pre-zip64 compatibility mode
          p->unzFile_f=unzOpen2(ioDevice, ioApi);
          if (p->unzFile_f != nullptr) {
              if (p->autoClose) {
                  unzSetFlags(p->unzFile_f, UNZ_AUTO_CLOSE);
              } else {
                  unzClearFlags(p->unzFile_f, UNZ_AUTO_CLOSE);
              }
          }
      }
      if (p->unzFile_f == nullptr) {
        p->zipError = UNZ_OPENERROR;
        if (!p->zipName.isEmpty())
          delete ioDevice;
        return false;
      }
      if (ioDevice->isSequential()) {
          unzClose(p->unzFile_f);
          if (!p->zipName.isEmpty())
              delete ioDevice;
          qWarning("QuaZip::open(): only mdCreate can be used with sequential devices");
          return false;
      }
      p->mode = mode;
      p->ioDevice = ioDevice;
      return true;

    case mdCreate:
    case mdAppend:
    case mdAdd:
      if (ioApi == nullptr) {
          if (p->autoClose)
              flags |= ZIP_AUTO_CLOSE;
          if (p->dataDescriptorWritingEnabled)
              flags |= ZIP_WRITE_DATA_DESCRIPTOR;
          if (p->utf8)
              flags |= ZIP_ENCODING_UTF8;
          p->zipFile_f=zipOpen3(ioDevice,
              mode==mdCreate?APPEND_STATUS_CREATE:
              mode==mdAppend?APPEND_STATUS_CREATEAFTER:
              APPEND_STATUS_ADDINZIP,
              nullptr, nullptr, flags);
      } else {
          // QuaZip pre-zip64 compatibility mode
          p->zipFile_f=zipOpen2(ioDevice,
              mode==mdCreate?APPEND_STATUS_CREATE:
              mode==mdAppend?APPEND_STATUS_CREATEAFTER:
              APPEND_STATUS_ADDINZIP,
              nullptr,
              ioApi);
          if (p->zipFile_f != nullptr) {
              zipSetFlags(p->zipFile_f, flags);
          }
      }
      if(p->zipFile_f == nullptr) {
        p->zipError = UNZ_OPENERROR;
        if (!p->zipName.isEmpty())
          delete ioDevice;
        return false;
      }
      if (ioDevice->isSequential()) {
        if (mode != mdCreate) {
          zipClose(p->zipFile_f, nullptr);
          qWarning("QuaZip::open(): only mdCreate can be used with sequential devices");
          if (!p->zipName.isEmpty())
              delete ioDevice;
          return false;
        }
        zipSetFlags(p->zipFile_f, ZIP_SEQUENTIAL);
      }
      p->mode=mode;
      p->ioDevice = ioDevice;
      return true;

    default:
      qWarning("QuaZip::open(): unknown mode: %d", static_cast<int>(mode));
      if (!p->zipName.isEmpty())
        delete ioDevice;
      return false;
      break;
  }
}

void QuaZip::close()
{
  p->zipError=UNZ_OK;
  switch(p->mode) {
    case mdNotOpen:
      return;
    case mdUnzip:
      p->zipError=unzClose(p->unzFile_f);
      break;
    case mdCreate:
    case mdAppend:
    case mdAdd:
      p->zipError=zipClose(p->zipFile_f, p->comment.isNull() ? nullptr : isUtf8Enabled()
        ? p->comment.toUtf8().constData()
        : p->commentCodec->fromUnicode(p->comment).constData());
      break;
    default:
      qWarning("QuaZip::close(): unknown mode: %d", static_cast<int>(p->mode));
      return;
  }
  // opened by name, need to delete the internal IO device
  if (!p->zipName.isEmpty()) {
      delete p->ioDevice;
      p->ioDevice = nullptr;
  }
  p->clearDirectoryMap();
  p->mode=mdNotOpen;
}

void QuaZip::setZipName(const QString& zipName)
{
  if(isOpen()) {
    qWarning("QuaZip::setZipName(): ZIP is already open!");
    return;
  }
  p->zipName=zipName;
  p->ioDevice = nullptr;
}

void QuaZip::setIoDevice(QIODevice *ioDevice)
{
  if(isOpen()) {
    qWarning("QuaZip::setIoDevice(): ZIP is already open!");
    return;
  }
  p->ioDevice = ioDevice;
  p->zipName = QString();
}

int QuaZip::getEntriesCount()const
{
  QuaZip *fakeThis=const_cast<QuaZip*>(this); // non-const
  fakeThis->p->zipError=UNZ_OK;
  if(p->mode!=mdUnzip) {
    qWarning("QuaZip::getEntriesCount(): ZIP is not open in mdUnzip mode");
    return -1;
  }
  unz_global_info64 globalInfo;
  if((fakeThis->p->zipError=unzGetGlobalInfo64(p->unzFile_f, &globalInfo))!=UNZ_OK)
    return p->zipError;
  return static_cast<int>(globalInfo.number_entry);
}

QString QuaZip::getComment()const
{
  QuaZip *fakeThis=const_cast<QuaZip*>(this); // non-const
  fakeThis->p->zipError=UNZ_OK;
  if(p->mode!=mdUnzip) {
    qWarning("QuaZip::getComment(): ZIP is not open in mdUnzip mode");
    return QString();
  }
  unz_global_info64 globalInfo;
  QByteArray comment;
  if((fakeThis->p->zipError=unzGetGlobalInfo64(p->unzFile_f, &globalInfo))!=UNZ_OK)
    return QString();
  comment.resize(globalInfo.size_comment);
  if((fakeThis->p->zipError=unzGetGlobalComment(p->unzFile_f, comment.data(), comment.size())) < 0)
    return QString();
  fakeThis->p->zipError = UNZ_OK;
  unsigned flags = 0;
  return (unzGetFileFlags(p->unzFile_f, &flags) == UNZ_OK) && (flags & UNZ_ENCODING_UTF8)
    ? QString::fromUtf8(comment) : p->commentCodec->toUnicode(comment);
}

bool QuaZip::setCurrentFile(const QString& fileName, CaseSensitivity cs)
{
  p->zipError=UNZ_OK;
  if(p->mode!=mdUnzip) {
    qWarning("QuaZip::setCurrentFile(): ZIP is not open in mdUnzip mode");
    return false;
  }
  if(fileName.isEmpty()) {
    p->hasCurrentFile_f=false;
    return true;
  }
  // Unicode-aware reimplementation of the unzLocateFile function
  if(p->unzFile_f==nullptr) {
    p->zipError=UNZ_PARAMERROR;
    return false;
  }
  if(fileName.length()>MAX_FILE_NAME_LENGTH) {
    p->zipError=UNZ_PARAMERROR;
    return false;
  }
  // Find the file by name
  bool sens = convertCaseSensitivity(cs) == Qt::CaseSensitive;
  QString lower, current;
  if(!sens) lower=fileName.toLower();
  p->hasCurrentFile_f=false;

  // Check the appropriate Map
  unz64_file_pos fileDirPos;
  fileDirPos.pos_in_zip_directory = 0;
  if (sens) {
      if (p->directoryCaseSensitive.contains(fileName))
          fileDirPos = p->directoryCaseSensitive.value(fileName);
  } else {
      if (p->directoryCaseInsensitive.contains(lower))
          fileDirPos = p->directoryCaseInsensitive.value(lower);
  }

  if (fileDirPos.pos_in_zip_directory != 0) {
      p->zipError = unzGoToFilePos64(p->unzFile_f, &fileDirPos);
      p->hasCurrentFile_f = p->zipError == UNZ_OK;
  }

  if (p->hasCurrentFile_f)
      return p->hasCurrentFile_f;

  // Not mapped yet, start from where we have got to so far
  for(bool more=p->goToFirstUnmappedFile(); more; more=goToNextFile()) {
    current=getCurrentFileName();
    if(current.isEmpty()) return false;
    if(sens) {
      if(current==fileName) break;
    } else {
      if(current.toLower()==lower) break;
    }
  }
  return p->hasCurrentFile_f;
}

bool QuaZip::goToFirstFile()
{
  p->zipError=UNZ_OK;
  if(p->mode!=mdUnzip) {
    qWarning("QuaZip::goToFirstFile(): ZIP is not open in mdUnzip mode");
    return false;
  }
  p->zipError=unzGoToFirstFile(p->unzFile_f);
  p->hasCurrentFile_f=p->zipError==UNZ_OK;
  return p->hasCurrentFile_f;
}

bool QuaZip::goToNextFile()
{
  p->zipError=UNZ_OK;
  if(p->mode!=mdUnzip) {
    qWarning("QuaZip::goToFirstFile(): ZIP is not open in mdUnzip mode");
    return false;
  }
  p->zipError=unzGoToNextFile(p->unzFile_f);
  p->hasCurrentFile_f=p->zipError==UNZ_OK;
  if(p->zipError==UNZ_END_OF_LIST_OF_FILE)
    p->zipError=UNZ_OK;
  return p->hasCurrentFile_f;
}

bool QuaZip::getCurrentFileInfo(QuaZipFileInfo *info)const
{
    QuaZipFileInfo64 info64;
    if (info == nullptr) { // Very unlikely because of the overloads
        return false;
    }
    if (!getCurrentFileInfo(&info64)) {
        return false;
    }
    info64.toQuaZipFileInfo(*info);
    return true;
}

bool QuaZip::getCurrentFileInfo(QuaZipFileInfo64 *info)const
{
  QuaZip *fakeThis=const_cast<QuaZip*>(this); // non-const
  fakeThis->p->zipError=UNZ_OK;
  if(p->mode!=mdUnzip) {
    qWarning("QuaZip::getCurrentFileInfo(): ZIP is not open in mdUnzip mode");
    return false;
  }
  unz_file_info64 info_z;
  QByteArray fileName;
  QByteArray extra;
  QByteArray comment;
  if(info==nullptr) return false;
  if(!isOpen()||!hasCurrentFile()) return false;
  if((fakeThis->p->zipError=unzGetCurrentFileInfo64(p->unzFile_f, &info_z, nullptr, 0, nullptr, 0, nullptr, 0))!=UNZ_OK)
    return false;
  fileName.resize(info_z.size_filename);
  extra.resize(info_z.size_file_extra);
  comment.resize(info_z.size_file_comment);
  if((fakeThis->p->zipError=unzGetCurrentFileInfo64(p->unzFile_f, nullptr,
      fileName.data(), fileName.size(),
      extra.data(), extra.size(),
      comment.data(), comment.size()))!=UNZ_OK)
    return false;
  info->versionCreated=info_z.version;
  info->versionNeeded=info_z.version_needed;
  info->flags=info_z.flag;
  info->method=info_z.compression_method;
  info->crc=info_z.crc;
  info->compressedSize=info_z.compressed_size;
  info->uncompressedSize=info_z.uncompressed_size;
  info->diskNumberStart=info_z.disk_num_start;
  info->internalAttr=info_z.internal_fa;
  info->externalAttr=info_z.external_fa;
  info->name=(info->flags & UNZ_ENCODING_UTF8) ? QString::fromUtf8(fileName) : p->fileNameCodec->toUnicode(fileName);
  info->comment=(info->flags & UNZ_ENCODING_UTF8) ? QString::fromUtf8(comment) : p->commentCodec->toUnicode(comment);
  info->extra=extra;
  info->dateTime=QDateTime(
      QDate(info_z.tmu_date.tm_year, info_z.tmu_date.tm_mon+1, info_z.tmu_date.tm_mday),
      QTime(info_z.tmu_date.tm_hour, info_z.tmu_date.tm_min, info_z.tmu_date.tm_sec));
  // Add to directory map
  p->addCurrentFileToDirectoryMap(info->name);
  return true;
}

QString QuaZip::getCurrentFileName()const
{
  QuaZip *fakeThis=const_cast<QuaZip*>(this); // non-const
  fakeThis->p->zipError=UNZ_OK;
  if(p->mode!=mdUnzip) {
    qWarning("QuaZip::getCurrentFileName(): ZIP is not open in mdUnzip mode");
    return QString();
  }
  if(!isOpen()||!hasCurrentFile()) return QString();
  QByteArray fileName(MAX_FILE_NAME_LENGTH, 0);
  unz_file_info64 file_info;
  if((fakeThis->p->zipError=unzGetCurrentFileInfo64(p->unzFile_f, &file_info, fileName.data(), fileName.size(),
      nullptr, 0, nullptr, 0))!=UNZ_OK)
    return QString();
  fileName.resize(file_info.size_filename);
  QString result = (file_info.flag & UNZ_ENCODING_UTF8)
    ? QString::fromUtf8(fileName) : p->fileNameCodec->toUnicode(fileName);
  if (result.isEmpty())
      return result;
  // Add to directory map
  p->addCurrentFileToDirectoryMap(result);
  return result;
}

void QuaZip::setFileNameCodec(QTextCodec *fileNameCodec)
{
  p->fileNameCodec=fileNameCodec;
}

void QuaZip::setFileNameCodec(const char *fileNameCodecName)
{
    p->fileNameCodec=QTextCodec::codecForName(fileNameCodecName);
}

void QuaZip::setOsCode(uint osCode)
{
    p->osCode = osCode;
}

uint QuaZip::getOsCode() const
{
    return p->osCode;
}

QTextCodec *QuaZip::getFileNameCodec()const
{
  return p->fileNameCodec;
}

void QuaZip::setCommentCodec(QTextCodec *commentCodec)
{
  p->commentCodec=commentCodec;
}

void QuaZip::setCommentCodec(const char *commentCodecName)
{
  p->commentCodec=QTextCodec::codecForName(commentCodecName);
}

QTextCodec *QuaZip::getCommentCodec()const
{
  return p->commentCodec;
}

QString QuaZip::getZipName() const
{
  return p->zipName;
}

QIODevice *QuaZip::getIoDevice() const
{
  if (!p->zipName.isEmpty()) // opened by name, using an internal QIODevice
    return nullptr;
  return p->ioDevice;
}

QuaZip::Mode QuaZip::getMode()const
{
  return p->mode;
}

bool QuaZip::isOpen()const
{
  return p->mode!=mdNotOpen;
}

int QuaZip::getZipError() const
{
  return p->zipError;
}

void QuaZip::setComment(const QString& comment)
{
  p->comment=comment;
}

bool QuaZip::hasCurrentFile()const
{
  return p->hasCurrentFile_f;
}

unzFile QuaZip::getUnzFile()
{
  return p->unzFile_f;
}

zipFile QuaZip::getZipFile()
{
  return p->zipFile_f;
}

void QuaZip::setDataDescriptorWritingEnabled(bool enabled)
{
    p->dataDescriptorWritingEnabled = enabled;
}

bool QuaZip::isDataDescriptorWritingEnabled() const
{
    return p->dataDescriptorWritingEnabled;
}

template<typename TFileInfo>
TFileInfo QuaZip_getFileInfo(QuaZip *zip, bool *ok);

template<>
QuaZipFileInfo QuaZip_getFileInfo(QuaZip *zip, bool *ok)
{
    QuaZipFileInfo info;
    *ok = zip->getCurrentFileInfo(&info);
    return info;
}

template<>
QuaZipFileInfo64 QuaZip_getFileInfo(QuaZip *zip, bool *ok)
{
    QuaZipFileInfo64 info;
    *ok = zip->getCurrentFileInfo(&info);
    return info;
}

template<>
QString QuaZip_getFileInfo(QuaZip *zip, bool *ok)
{
    QString name = zip->getCurrentFileName();
    *ok = !name.isEmpty();
    return name;
}

template<typename TFileInfo>
bool QuaZipPrivate::getFileInfoList(QList<TFileInfo> *result) const
{
  QuaZipPrivate *fakeThis=const_cast<QuaZipPrivate*>(this);
  fakeThis->zipError=UNZ_OK;
  if (mode!=QuaZip::mdUnzip) {
    qWarning("QuaZip::getFileNameList/getFileInfoList(): "
            "ZIP is not open in mdUnzip mode");
    return false;
  }
  QString currentFile;
  if (q->hasCurrentFile()) {
      currentFile = q->getCurrentFileName();
  }
  if (q->goToFirstFile()) {
      do {
          bool ok;
          result->append(QuaZip_getFileInfo<TFileInfo>(q, &ok));
          if (!ok)
              return false;
      } while (q->goToNextFile());
  }
  if (zipError != UNZ_OK)
      return false;
  if (currentFile.isEmpty()) {
      if (!q->goToFirstFile())
          return false;
  } else {
      if (!q->setCurrentFile(currentFile))
          return false;
  }
  return true;
}

QStringList QuaZip::getFileNameList() const
{
    QStringList list;
    if (!p->getFileInfoList(&list))
        return QStringList();
    return list;
}

QList<QuaZipFileInfo> QuaZip::getFileInfoList() const
{
    QList<QuaZipFileInfo> list;
    if (!p->getFileInfoList(&list))
        return QList<QuaZipFileInfo>();
    return list;
}

QList<QuaZipFileInfo64> QuaZip::getFileInfoList64() const
{
    QList<QuaZipFileInfo64> list;
    if (!p->getFileInfoList(&list))
        return QList<QuaZipFileInfo64>();
    return list;
}

Qt::CaseSensitivity QuaZip::convertCaseSensitivity(QuaZip::CaseSensitivity cs)
{
  if (cs == csDefault) {
#ifdef Q_OS_WIN
      return Qt::CaseInsensitive;
#else
      return Qt::CaseSensitive;
#endif
  } else {
      return cs == csSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  }
}

void QuaZip::setDefaultFileNameCodec(QTextCodec *codec)
{
    QuaZipPrivate::defaultFileNameCodec = codec;
}

void QuaZip::setDefaultFileNameCodec(const char *codecName)
{
    setDefaultFileNameCodec(QTextCodec::codecForName(codecName));
}

void QuaZip::setDefaultOsCode(uint osCode)
{
    QuaZipPrivate::defaultOsCode = osCode;
}

uint QuaZip::getDefaultOsCode()
{
    return QuaZipPrivate::defaultOsCode;
}

void QuaZip::setZip64Enabled(bool zip64)
{
    p->zip64 = zip64;
}

bool QuaZip::isZip64Enabled() const
{
    return p->zip64;
}

void QuaZip::setUtf8Enabled(bool utf8)
{
    p->utf8 = utf8;
}

bool QuaZip::isUtf8Enabled() const
{
    return p->utf8;
}

bool QuaZip::isAutoClose() const
{
    return p->autoClose;
}

void QuaZip::setAutoClose(bool autoClose) const
{
    p->autoClose = autoClose;
}
