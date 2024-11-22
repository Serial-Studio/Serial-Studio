// Copyright (c) 2014, Razvan Petru
// Copyright (c) 2014, Omar Carey
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

#ifndef QSLOGDESTFUNCTOR_H
#define QSLOGDESTFUNCTOR_H

#include "QsLogDest.h"
#include <QObject>

namespace QsLogging
{
// Offers various types of function-like sinks.
// This is an advanced destination type. Depending on your configuration, LogFunction might be
// called from a different thread or even a different binary. You should not access QsLog from
// inside LogFunction and should not perform any time-consuming operations.
// logMessageReady is connected through a queued connection and trace messages are not included
class QSLOG_SHARED_OBJECT FunctorDestination : public QObject, public Destination
{
    Q_OBJECT
public:
    static const char* const Type;

    explicit FunctorDestination(LogFunction f);
    FunctorDestination(QObject* receiver, const char* member);

    void write(const LogMessage& message) override;
    bool isValid() override;
    QString type() const override;

protected:
    // int used to avoid registering a new enum type
    Q_SIGNAL void logMessageReady(const QsLogging::LogMessage& message);

private:
    LogFunction mLogFunction;
};
}

#endif // QSLOGDESTFUNCTOR_H
