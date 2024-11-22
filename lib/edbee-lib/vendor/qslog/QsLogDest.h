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

#ifndef QSLOGDEST_H
#define QSLOGDEST_H

#include "QsLogLevel.h"
#include "QsLogMessage.h"
#include "QsLogSharedLibrary.h"
#include <QtGlobal>
#include <limits>
#include <memory>
#include <functional>
class QString;
class QObject;

namespace QsLogging
{

class QSLOG_SHARED_OBJECT Destination
{
public:
    using LogFunction = std::function<void(const LogMessage& message)>;

public:
    virtual ~Destination() noexcept;
    virtual void write(const LogMessage& message) = 0;
    //!
    //! \brief isValid
    //! \return whether the destination was created correctly
    //!
    virtual bool isValid() = 0;
    //!
    //! \brief type
    //! \return the type as a string e.g: console, file.
    //!         The returned value may change in different versions of QsLog, but two destinations
    //!         of the same type will return the same value.
    //!
    virtual QString type() const = 0;
};

using DestinationPtrU = std::unique_ptr<Destination>;

// a series of "named" paramaters, to make the file destination creation more readable
enum class LogRotationOption
{
    DisableLogRotation = 0,
    EnableLogRotation  = 1
};

struct QSLOG_SHARED_OBJECT MaxSizeBytes
{
    MaxSizeBytes() : size(0) {}
    explicit MaxSizeBytes(qint64 size_) : size(size_) {}
    qint64 size;
};

struct QSLOG_SHARED_OBJECT MaxOldLogCount
{
    MaxOldLogCount() : count(0) {}
    explicit MaxOldLogCount(int count_) : count(count_) {}
    int count;
};


//! Creates logging destinations/sinks. The caller takes ownership of the destinations from the
//! factory and will pass ownership to the logger when adding the destination.
class QSLOG_SHARED_OBJECT DestinationFactory
{
public:
    static DestinationPtrU MakeFileDestination(const QString& filePath,
        LogRotationOption rotation = LogRotationOption::DisableLogRotation,
        const MaxSizeBytes &sizeInBytesToRotateAfter = MaxSizeBytes(),
        const MaxOldLogCount &oldLogsToKeep = MaxOldLogCount());
    static DestinationPtrU MakeDebugOutputDestination();
    // takes a pointer to a function
    static DestinationPtrU MakeFunctorDestination(Destination::LogFunction f);
    // takes a QObject + signal/slot
    static DestinationPtrU MakeFunctorDestination(QObject* receiver, const char* member);
};

} // end namespace

#endif // QSLOGDEST_H
