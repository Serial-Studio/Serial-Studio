// Copyright (c) 2013, Razvan Petru
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// * The name of the contributors may not be used to endorse or promote products
//   derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include "QsLogDestFile.h"
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    #include <QTextCodec>
#endif
#include <QDateTime>
#include <QString>
#include <QtGlobal>
#include <iostream>

const int QsLogging::SizeRotationStrategy::MaxBackupCount = 10;

QsLogging::RotationStrategy::~RotationStrategy() noexcept = default;

void QsLogging::SizeRotationStrategy::setInitialInfo(const QFile &file)
{
    mFileName = file.fileName();
    mCurrentSizeInBytes = file.size();
}

void QsLogging::SizeRotationStrategy::setInitialInfo(const QString &filePath, int fileSize)
{
    mFileName = filePath;
    mCurrentSizeInBytes = fileSize;
}

void QsLogging::SizeRotationStrategy::includeMessageInCalculation(const QString &message)
{
    includeMessageInCalculation(message.toUtf8());
}

void QsLogging::SizeRotationStrategy::includeMessageInCalculation(const QByteArray &message)
{
    mCurrentSizeInBytes += message.size();
}

bool QsLogging::SizeRotationStrategy::shouldRotate()
{
    return mCurrentSizeInBytes > mMaxSizeInBytes;
}

// Algorithm assumes backups will be named filename.X, where 1 <= X <= mBackupsCount.
// All X's will be shifted up.
void QsLogging::SizeRotationStrategy::rotate()
{
    if (!mBackupsCount) {
        if (!removeFileAtPath(mFileName)) {
            std::cerr << "QsLog: backup delete failed " << qPrintable(mFileName);
        }
        return;
    }

     // 1. find the last existing backup than can be shifted up
     const QString logNamePattern = mFileName + QString::fromUtf8(".%1");
     int lastExistingBackupIndex = 0;
     for (int i = 1;i <= mBackupsCount;++i) {
         const QString backupFileName = logNamePattern.arg(i);
         if (fileExistsAtPath(backupFileName)) {
             lastExistingBackupIndex = qMin(i, mBackupsCount - 1);
         } else {
             break;
         }
     }

     // 2. shift up
     for (int i = lastExistingBackupIndex;i >= 1;--i) {
         const QString oldName = logNamePattern.arg(i);
         const QString newName = logNamePattern.arg(i + 1);
         removeFileAtPath(newName);
         const bool renamed = renameFileFromTo(oldName, newName);
         if (!renamed) {
             std::cerr << "QsLog: could not rename backup " << qPrintable(oldName)
                       << " to " << qPrintable(newName);
         }
     }

     // 3. rename current log file
     const QString newName = logNamePattern.arg(1);
     if (fileExistsAtPath(newName)) {
         removeFileAtPath(newName);
     }
     if (!renameFileFromTo(mFileName, newName)) {
         std::cerr << "QsLog: could not rename log " << qPrintable(mFileName)
                   << " to " << qPrintable(newName);
     }
}

QIODevice::OpenMode QsLogging::SizeRotationStrategy::recommendedOpenModeFlag()
{
    return QIODevice::Append;
}

void QsLogging::SizeRotationStrategy::setMaximumSizeInBytes(qint64 size)
{
    Q_ASSERT(size >= 0);
    mMaxSizeInBytes = size;
}

void QsLogging::SizeRotationStrategy::setBackupCount(int backups)
{
    Q_ASSERT(backups >= 0);
    mBackupsCount = qMin(backups, SizeRotationStrategy::MaxBackupCount);
}

bool QsLogging::SizeRotationStrategy::removeFileAtPath(const QString &path)
{
    return QFile::remove(path);
}

bool QsLogging::SizeRotationStrategy::fileExistsAtPath(const QString &path)
{
    return QFile::exists(path);
}

bool QsLogging::SizeRotationStrategy::renameFileFromTo(const QString &from, const QString &to)
{
    return QFile::rename(from, to);
}

const char* const QsLogging::FileDestination::Type = "file";

QsLogging::FileDestination::FileDestination(const QString& filePath, RotationStrategyPtrU&& rotationStrategy)
    : mRotationStrategy(std::move(rotationStrategy))
{
    mFile.setFileName(filePath);
    if (!mFile.open(QFile::WriteOnly | QFile::Text | mRotationStrategy->recommendedOpenModeFlag())) {
        std::cerr << "QsLog: could not open log file " << qPrintable(filePath);
    }
    mOutputStream.setDevice(&mFile);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    mOutputStream.setCodec(QTextCodec::codecForName("UTF-8"));
#else
    mOutputStream.setEncoding(QStringConverter::Utf8);
#endif

    mRotationStrategy->setInitialInfo(mFile);
}

void QsLogging::FileDestination::write(const LogMessage& message)
{
    const QByteArray utf8Message = message.formatted.toUtf8();
    mRotationStrategy->includeMessageInCalculation(utf8Message);
    if (mRotationStrategy->shouldRotate()) {
        mOutputStream.setDevice(nullptr);
        mFile.close();
        mRotationStrategy->rotate();
        if (!mFile.open(QFile::WriteOnly | QFile::Text | mRotationStrategy->recommendedOpenModeFlag())) {
            std::cerr << "QsLog: could not reopen log file " << qPrintable(mFile.fileName());
        }
        mRotationStrategy->setInitialInfo(mFile);
        mOutputStream.setDevice(&mFile);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        mOutputStream.setCodec(QTextCodec::codecForName("UTF-8"));
#else
        mOutputStream.setEncoding(QStringConverter::Utf8);
#endif

    }

    mOutputStream << utf8Message << '\n';
    mOutputStream.flush();
}

bool QsLogging::FileDestination::isValid()
{
    return mFile.isOpen();
}

QString QsLogging::FileDestination::type() const
{
    return QString::fromLatin1(Type);
}

