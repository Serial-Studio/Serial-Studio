/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "logstream.h"

namespace util::log {
LogStream::LogStream(const Loc &location, LogSeverity severity)
    : location_(location), severity_(severity) {}

LogStream::~LogStream() { LogString(location_, severity_, str()); }
}  // namespace util::log
