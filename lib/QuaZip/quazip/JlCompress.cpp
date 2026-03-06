/*
Copyright (C) 2010 Roberto Pompermaier
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

#include "JlCompress.h"
#include <memory>

bool JlCompress::copyData(QIODevice &inFile, QIODevice &outFile)
{
    while (!inFile.atEnd()) {
        char buf[4096];
        qint64 readLen = inFile.read(buf, 4096);
        if (readLen <= 0)
            return false;
        if (outFile.write(buf, readLen) != readLen)
            return false;
    }
    return true;
}

bool JlCompress::compressFile(QuaZip* zip, QString fileName, QString fileDest) {
  return compressFile(zip, fileName, fileDest, Options());
}

bool JlCompress::compressFile(QuaZip* zip, QString fileName, QString fileDest, const Options& options) {
    // zip: object where to add the file
    // fileName: real file name
    // fileDest: file name inside the zip object

    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdCreate &&
        zip->getMode()!=QuaZip::mdAppend &&
        zip->getMode()!=QuaZip::mdAdd) return false;

    QuaZipFile outFile(zip);
    if (options.getDateTime().isNull()) {
      if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest, fileName), nullptr, 0, options.getCompressionMethod(), options.getCompressionLevel())) return false;
    }
    else {
      if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest, fileName, options.getDateTime()), nullptr, 0, options.getCompressionMethod(), options.getCompressionLevel())) return false;
    }

    QFileInfo input(fileName);
    if (quazip_is_symlink(input)) {
        // Not sure if we should use any specialized codecs here.
        // After all, a symlink IS just a byte array. And
        // this is mostly for Linux, where UTF-8 is ubiquitous these days.
        QString path = quazip_symlink_target(input);
        QString relativePath = input.dir().relativeFilePath(path);
        outFile.write(QFile::encodeName(relativePath));
    } else {
        QFile inFile;
        inFile.setFileName(fileName);
        if (!inFile.open(QIODevice::ReadOnly))
            return false;
        if (!copyData(inFile, outFile) || outFile.getZipError()!=UNZ_OK)
            return false;
        inFile.close();
    }

    outFile.close();
    return outFile.getZipError() == UNZ_OK;
}

bool JlCompress::compressSubDir(QuaZip* zip, QString dir, QString origDir, bool recursive, QDir::Filters filters) {
  return compressSubDir(zip, dir, origDir, recursive, filters, Options());
}

bool JlCompress::compressSubDir(QuaZip* zip, QString dir, QString origDir, bool recursive, QDir::Filters filters, const Options& options) {
    // zip: object where to add the file
    // dir: current real directory
    // origDir: original real directory
    // (path(dir)-path(origDir)) = path inside the zip object

    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdCreate &&
        zip->getMode()!=QuaZip::mdAppend &&
        zip->getMode()!=QuaZip::mdAdd) return false;

    QDir directory(dir);
    if (!directory.exists()) return false;

    QDir origDirectory(origDir);
    if (dir != origDir) {
        QuaZipFile dirZipFile(zip);
        std::unique_ptr<QuaZipNewInfo> qzni;
        if (options.getDateTime().isNull()) {
            qzni = std::make_unique<QuaZipNewInfo>(origDirectory.relativeFilePath(dir) + QLatin1String("/"), dir);
        }
        else {
            qzni = std::make_unique<QuaZipNewInfo>(origDirectory.relativeFilePath(dir) + QLatin1String("/"), dir, options.getDateTime());
        }
        if (!dirZipFile.open(QIODevice::WriteOnly, *qzni, nullptr, 0, 0)) {
            return false;
        }
        dirZipFile.close();
    }

    // Whether to compress the subfolders, recursion
    if (recursive) {
        // For each subfolder
        QFileInfoList files = directory.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot|filters);
        for (const auto& file : files) {
            if (!file.isDir()) // needed for Qt < 4.7 because it doesn't understand AllDirs
                continue;
            // Compress subdirectory
            if(!compressSubDir(zip,file.absoluteFilePath(),origDir,recursive,filters, options)) return false;
        }
    }

    // For each file in directory
    QFileInfoList files = directory.entryInfoList(QDir::Files|filters);
    for (const auto& file : files) {
        // If it's not a file or it's the compressed file being created
        if(!file.isFile()||file.absoluteFilePath()==zip->getZipName()) continue;

        // Create relative name for the compressed file
        QString filename = origDirectory.relativeFilePath(file.absoluteFilePath());

        // Compress the file
        if (!compressFile(zip,file.absoluteFilePath(),filename, options)) return false;
    }

    return true;
}

bool JlCompress::extractFile(QuaZip* zip, QString fileName, QString fileDest) {
    // zip: object where to add the file
    // filename: real file name
    // fileincompress: file name of the compressed file

    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdUnzip) return false;

    if (!fileName.isEmpty())
        zip->setCurrentFile(fileName);
    QuaZipFile inFile(zip);
    if(!inFile.open(QIODevice::ReadOnly) || inFile.getZipError()!=UNZ_OK) return false;

    // Check existence of resulting file
    QDir curDir;
    if (fileDest.endsWith(QLatin1String("/"))) {
        if (!curDir.mkpath(fileDest)) {
            return false;
        }
    } else {
        if (!curDir.mkpath(QFileInfo(fileDest).absolutePath())) {
            return false;
        }
    }

    QuaZipFileInfo64 info;
    if (!zip->getCurrentFileInfo(&info))
        return false;

    QFile::Permissions srcPerm = info.getPermissions();
    if (fileDest.endsWith(QLatin1String("/")) && QFileInfo(fileDest).isDir()) {
        if (srcPerm != 0) {
            QFile(fileDest).setPermissions(srcPerm);
        }
        return true;
    }

    if (info.isSymbolicLink()) {
        QString target = QFile::decodeName(inFile.readAll());
        return QFile::link(target, fileDest);
    }

    // Open resulting file
    QFile outFile;
    outFile.setFileName(fileDest);
    if(!outFile.open(QIODevice::WriteOnly)) return false;

    // Copy data
    if (!copyData(inFile, outFile) || inFile.getZipError()!=UNZ_OK) {
        outFile.close();
        removeFile(QStringList(fileDest));
        return false;
    }
    outFile.close();

    // Close file
    inFile.close();
    if (inFile.getZipError()!=UNZ_OK) {
        removeFile(QStringList(fileDest));
        return false;
    }

    if (srcPerm != 0) {
        outFile.setPermissions(srcPerm);
    }
    return true;
}

bool JlCompress::removeFile(QStringList listFile) {
    bool ret = true;
    // For each file
    for (int i=0; i<listFile.count(); i++) {
        // Remove
        ret = ret && QFile::remove(listFile.at(i));
    }
    return ret;
}

bool JlCompress::compressFile(QString fileCompressed, QString file) {
  return compressFile(fileCompressed, file, JlCompress::Options());
}

bool JlCompress::compressFile(QString fileCompressed, QString file, const Options& options) {
    // Create zip
    QuaZip zip(fileCompressed);
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip.open(QuaZip::mdCreate)) {
        QFile::remove(fileCompressed);
        return false;
    }

    // Add file
    if (!compressFile(&zip,file,QFileInfo(file).fileName(), options)) {
        QFile::remove(fileCompressed);
        return false;
    }

    // Close zip
    zip.close();
    if(zip.getZipError()!=0) {
        QFile::remove(fileCompressed);
        return false;
    }

    return true;
}

bool JlCompress::compressFiles(QString fileCompressed, QStringList files) {
    return compressFiles(fileCompressed, files, Options());
}

bool JlCompress::compressFiles(QString fileCompressed, QStringList files, const Options& options) {
  // Create zip
  QuaZip zip(fileCompressed);
  QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
  if(!zip.open(QuaZip::mdCreate)) {
    QFile::remove(fileCompressed);
    return false;
  }

  // Compress files
  QFileInfo info;
  for (int index = 0; index < files.size(); ++index ) {
    const QString & file( files.at( index ) );
    info.setFile(file);
    if (!info.exists() || !compressFile(&zip,file,info.fileName(), options)) {
      QFile::remove(fileCompressed);
      return false;
    }
  }

  // Close zip
  zip.close();
  if(zip.getZipError()!=0) {
    QFile::remove(fileCompressed);
    return false;
  }

  return true;
}

bool JlCompress::compressDir(QString fileCompressed, QString dir, bool recursive) {
    return compressDir(fileCompressed, dir, recursive, QDir::Filters());
}

bool JlCompress::compressDir(QString fileCompressed, QString dir,
                             bool recursive, QDir::Filters filters)
{
    return compressDir(fileCompressed, dir, recursive, filters, Options());
}

bool JlCompress::compressDir(QString fileCompressed, QString dir,
                             bool recursive, QDir::Filters filters, const Options& options)
{
  // Create zip
  QuaZip zip(fileCompressed);
  QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
  if(!zip.open(QuaZip::mdCreate)) {
    QFile::remove(fileCompressed);
    return false;
  }

  // Add the files and subdirectories
  if (!compressSubDir(&zip,dir,dir,recursive, filters, options)) {
    QFile::remove(fileCompressed);
    return false;
  }

  // Close zip
  zip.close();
  if(zip.getZipError()!=0) {
    QFile::remove(fileCompressed);
    return false;
  }

  return true;
}

QString JlCompress::extractFile(QString fileCompressed, QString fileName, QString fileDest) {
    // Open zip
    QuaZip zip(fileCompressed);
    return extractFile(zip, fileName, fileDest);
}

QString JlCompress::extractFile(QuaZip &zip, QString fileName, QString fileDest)
{
    if(!zip.open(QuaZip::mdUnzip)) {
        return QString();
    }

    // Extract file
    if (fileDest.isEmpty())
        fileDest = fileName;
    if (!extractFile(&zip,fileName,fileDest)) {
        return QString();
    }

    // Close zip
    zip.close();
    if(zip.getZipError()!=0) {
        removeFile(QStringList(fileDest));
        return QString();
    }
    return QFileInfo(fileDest).absoluteFilePath();
}

QStringList JlCompress::extractFiles(QString fileCompressed, QStringList files, QString dir) {
    // Create zip
    QuaZip zip(fileCompressed);
    return extractFiles(zip, files, dir);
}

QStringList JlCompress::extractFiles(QuaZip &zip, const QStringList &files, const QString &dir)
{
    if(!zip.open(QuaZip::mdUnzip)) {
        return QStringList();
    }

    // Extract file
    QStringList extracted;
    for (int i=0; i<files.count(); i++) {
        QString absPath = QDir(dir).absoluteFilePath(files.at(i));
        if (!extractFile(&zip, files.at(i), absPath)) {
            removeFile(extracted);
            return QStringList();
        }
        extracted.append(absPath);
    }

    // Close zip
    zip.close();
    if(zip.getZipError()!=0) {
        removeFile(extracted);
        return QStringList();
    }

    return extracted;
}

QStringList JlCompress::extractDir(QString fileCompressed, QTextCodec* fileNameCodec, QString dir) {
    // Open zip
    QuaZip zip(fileCompressed);
    if (fileNameCodec)
        zip.setFileNameCodec(fileNameCodec);
    return extractDir(zip, dir);
}

QStringList JlCompress::extractDir(QString fileCompressed, QString dir) {
    return extractDir(fileCompressed, nullptr, dir);
}

QStringList JlCompress::extractDir(QuaZip &zip, const QString &dir)
{
    if(!zip.open(QuaZip::mdUnzip)) {
        return QStringList();
    }
    QString cleanDir = QDir::cleanPath(dir);
    QDir directory(cleanDir);
    QString absCleanDir = directory.absolutePath();
    if (!absCleanDir.endsWith(QLatin1Char('/'))) // It only ends with / if it's the FS root.
        absCleanDir += QLatin1Char('/');
    QStringList extracted;
    if (!zip.goToFirstFile()) {
        return QStringList();
    }
    do {
        QString name = zip.getCurrentFileName();
        QString absFilePath = directory.absoluteFilePath(name);
        QString absCleanPath = QDir::cleanPath(absFilePath);
        if (!absCleanPath.startsWith(absCleanDir))
            continue;
        if (!extractFile(&zip, QLatin1String(""), absFilePath)) {
            removeFile(extracted);
            return QStringList();
        }
        extracted.append(absFilePath);
    } while (zip.goToNextFile());

    // Close zip
    zip.close();
    if(zip.getZipError()!=0) {
        removeFile(extracted);
        return QStringList();
    }

    return extracted;
}

QStringList JlCompress::getFileList(QString fileCompressed) {
    // Open zip
    QuaZip* zip = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    return getFileList(zip);
}

QStringList JlCompress::getFileList(QuaZip *zip)
{
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QStringList();
    }

    // Extract file names
    QStringList lst;
    QuaZipFileInfo64 info;
    for(bool more=zip->goToFirstFile(); more; more=zip->goToNextFile()) {
      if(!zip->getCurrentFileInfo(&info)) {
          delete zip;
          return QStringList();
      }
      lst << info.name;
      //info.name.toLocal8Bit().constData()
    }

    // Close zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        return QStringList();
    }
    delete zip;
    return lst;
}

QStringList JlCompress::extractDir(QIODevice* ioDevice, QTextCodec* fileNameCodec, QString dir)
{
    QuaZip zip(ioDevice);
    if (fileNameCodec)
        zip.setFileNameCodec(fileNameCodec);
    return extractDir(zip, dir);
}

QStringList JlCompress::extractDir(QIODevice *ioDevice, QString dir)
{
    return extractDir(ioDevice, nullptr, dir);
}

QStringList JlCompress::getFileList(QIODevice *ioDevice)
{
    QuaZip *zip = new QuaZip(ioDevice);
    return getFileList(zip);
}

QString JlCompress::extractFile(QIODevice *ioDevice, QString fileName, QString fileDest)
{
    QuaZip zip(ioDevice);
    return extractFile(zip, fileName, fileDest);
}

QStringList JlCompress::extractFiles(QIODevice *ioDevice, QStringList files, QString dir)
{
    QuaZip zip(ioDevice);
    return extractFiles(zip, files, dir);
} 
