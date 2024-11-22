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

#include "QsLog.h"
#include "QsLogDest.h"
#ifdef QS_LOG_SEPARATE_THREAD
#include <QThread>
#include <QWaitCondition>
#include <queue>
#endif
#include <QMutex>
#include <QDateTime>
#include <QLatin1String>
#include <QtGlobal>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <vector>

namespace QsLogging
{
using DestinationList = std::vector<DestinationPtrU>;

#ifdef QS_LOG_SEPARATE_THREAD
//! Messages can be enqueued from other threads and will be logged one by one.
//! Note: std::queue was used instead of QQueue because it accepts types missing operator=.
class LoggerThread : public QThread
{
public:
    void enqueue(const LogMessage& message)
    {
        QMutexLocker locker(&mutex);
        messageQueue.push(message);
        waitCondition.wakeOne();
    }

    void requestStop()
    {
        QMutexLocker locker(&mutex);
        requestInterruption();
        waitCondition.wakeOne();
    }

protected:
    virtual void run()
    {
        while (true) {
            QMutexLocker locker(&mutex);
            if (messageQueue.empty() && !isInterruptionRequested()) {
                waitCondition.wait(&mutex);
            }
            if (isInterruptionRequested()) {
                break;
            }

            const LogMessage msg = messageQueue.front();
            messageQueue.pop();
            locker.unlock();
            Logger::instance().write(msg);
        }
    }

private:
    QMutex mutex;
    QWaitCondition waitCondition;
    std::queue<LogMessage> messageQueue;
};
#endif


class LoggerImpl
{
public:
    LoggerImpl();
    ~LoggerImpl();

#ifdef QS_LOG_SEPARATE_THREAD
    bool shutDownLoggerThread();

    LoggerThread loggerThread;
#endif
    QMutex logMutex;
    Level level;
    DestinationList destList;
};

LoggerImpl::LoggerImpl()
    : level(InfoLevel)
{
    destList.reserve(2); // assume at least file + console
#ifdef QS_LOG_SEPARATE_THREAD
    loggerThread.start(QThread::LowPriority);
#endif
}

LoggerImpl::~LoggerImpl()
{
#ifdef QS_LOG_SEPARATE_THREAD
#if defined(Q_OS_WIN) && defined(QSLOG_IS_SHARED_LIBRARY)
    // Waiting on the thread here is too late and can lead to deadlocks. More details:
    // * Another reason not to do anything scary in your DllMain:
    //   https://blogs.msdn.microsoft.com/oldnewthing/20040128-00/?p=40853
    // * Dynamic Link libraries best practices:
    //   https://msdn.microsoft.com/en-us/library/windows/desktop/dn633971%28v=vs.85%29.aspx#general_best_practices
    Q_ASSERT(loggerThread.isFinished());
    if (!loggerThread.isFinished()) {
        qCritical("You must shut down the QsLog thread, otherwise deadlocks can occur!");
    }
#endif
    shutDownLoggerThread();
#endif
}

#ifdef QS_LOG_SEPARATE_THREAD
bool LoggerImpl::shutDownLoggerThread()
{
    if (loggerThread.isFinished()) {
        return true;
    }

    loggerThread.requestStop();
    return loggerThread.wait();
}
#endif


Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

// tries to extract the level from a string log message. If available, conversionSucceeded will
// contain the conversion result.
Level Logger::levelFromLogMessage(const QString& logMessage, bool* conversionSucceeded)
{
    using namespace QsLogging;

    if (conversionSucceeded)
        *conversionSucceeded = true;

    if (logMessage.startsWith(QLatin1String(LevelName(TraceLevel))))
        return TraceLevel;
    if (logMessage.startsWith(QLatin1String(LevelName(DebugLevel))))
        return DebugLevel;
    if (logMessage.startsWith(QLatin1String(LevelName(InfoLevel))))
        return InfoLevel;
    if (logMessage.startsWith(QLatin1String(LevelName(WarnLevel))))
        return WarnLevel;
    if (logMessage.startsWith(QLatin1String(LevelName(ErrorLevel))))
        return ErrorLevel;
    if (logMessage.startsWith(QLatin1String(LevelName(FatalLevel))))
        return FatalLevel;

    if (conversionSucceeded)
        *conversionSucceeded = false;
    return OffLevel;
}

Logger::~Logger() noexcept = default;

#if defined(Q_OS_WIN)
bool Logger::shutDownLoggerThread()
{
#ifdef QS_LOG_SEPARATE_THREAD
    return d->shutDownLoggerThread();
#else
    return true;
#endif
}
#endif

void Logger::addDestination(DestinationPtrU&& destination)
{
    Q_ASSERT(destination.get());
    QMutexLocker lock(&d->logMutex);
    d->destList.emplace_back(std::move(destination));
}

DestinationPtrU Logger::removeDestination(const QString &type)
{
    QMutexLocker lock(&d->logMutex);

    const auto it = std::find_if(d->destList.begin(), d->destList.end(), [&type](const DestinationPtrU& dest){
        return dest->type() == type;
    });

    if (it != d->destList.end()) {
        auto removed = std::move(*it);
        d->destList.erase(it);
        return removed;
    }

    return DestinationPtrU();
}

bool Logger::hasDestinationOfType(const char* type) const
{
    QMutexLocker lock(&d->logMutex);
    const QLatin1String latin1Type(type);
    for (const auto& dest : d->destList) {
        if (dest->type() == latin1Type) {
            return true;
        }
    }

    return false;
}

void Logger::setLoggingLevel(Level newLevel)
{
    d->level = newLevel;
}

Level Logger::loggingLevel() const
{
    return d->level;
}

Logger::Helper::~Helper() noexcept
{
    const LogMessage msg(buffer, QDateTime::currentDateTimeUtc(), level);
    Logger::instance().enqueueWrite(msg);
}


Logger::Logger()
    : d(new LoggerImpl)
{
    qRegisterMetaType<LogMessage>("QsLogging::LogMessage");
}

//! directs the message to the task queue or writes it directly
void Logger::enqueueWrite(const LogMessage& message)
{
#ifdef QS_LOG_SEPARATE_THREAD
    d->loggerThread.enqueue(message);
#else
    write(message);
#endif
}

//! Sends the message to all the destinations. The level for this message is passed in case
//! it's useful for processing in the destination.
void Logger::write(const LogMessage& message)
{
    QMutexLocker lock(&d->logMutex);
    for (auto& dest : d->destList) {
        dest->write(message);
    }
}

} // end namespace
