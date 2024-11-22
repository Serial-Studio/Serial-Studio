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

#ifndef QSLOGDESTFILE_H
#define QSLOGDESTFILE_H

#include "QsLogDest.h"
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QByteArray>
#include <QtGlobal>
#include <memory>

namespace QsLogging
{
class QSLOG_SHARED_OBJECT RotationStrategy
{
public:
    virtual ~RotationStrategy() noexcept;

    virtual void setInitialInfo(const QFile &file) = 0;
    virtual void includeMessageInCalculation(const QString &message) = 0;
    virtual void includeMessageInCalculation(const QByteArray &message) = 0;
    virtual bool shouldRotate() = 0;
    virtual void rotate() = 0;
    virtual QIODevice::OpenMode recommendedOpenModeFlag() = 0;
};

// Never rotates file, overwrites existing file.
class QSLOG_SHARED_OBJECT NullRotationStrategy : public RotationStrategy
{
public:
    void setInitialInfo(const QFile &) override {}
    void includeMessageInCalculation(const QString &) override {}
    void includeMessageInCalculation(const QByteArray &) override {}
    bool shouldRotate() override
    {
        return false;
    }
    void rotate() override {}
    QIODevice::OpenMode recommendedOpenModeFlag() override
    {
        return QIODevice::Truncate;
    }
};

// Rotates after a size is reached, keeps a number of <= 10 backups, appends to existing file.
class QSLOG_SHARED_OBJECT SizeRotationStrategy : public RotationStrategy
{
public:
    SizeRotationStrategy() = default;
    static const int MaxBackupCount;

    void setInitialInfo(const QFile &file) override;
    void setInitialInfo(const QString& filePath, int fileSize);
    void includeMessageInCalculation(const QString &message) override;
    void includeMessageInCalculation(const QByteArray &message) override;
    bool shouldRotate() override;
    void rotate() override;
    QIODevice::OpenMode recommendedOpenModeFlag() override;

    void setMaximumSizeInBytes(qint64 size);
    void setBackupCount(int backups);

protected:
    // can be overridden for testing
    virtual bool removeFileAtPath(const QString& path);
    virtual bool fileExistsAtPath(const QString& path);
    virtual bool renameFileFromTo(const QString& from, const QString& to);

private:
    QString mFileName;
    qint64 mCurrentSizeInBytes{0};
    qint64 mMaxSizeInBytes{0};
    int mBackupsCount{0};
};

using RotationStrategyPtrU = std::unique_ptr<RotationStrategy>;

// file message sink
class QSLOG_SHARED_OBJECT FileDestination : public Destination
{
public:
    static const char* const Type;

    FileDestination(const QString& filePath, RotationStrategyPtrU &&rotationStrategy);
    void write(const LogMessage& message) override;
    bool isValid() override;
    QString type() const override;

private:
    QFile mFile;
    QTextStream mOutputStream;
    RotationStrategyPtrU mRotationStrategy;
};

}

#endif // QSLOGDESTFILE_H
