/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file logstream.h
 * \brief Implements a stream interface to log messages. This is the file to use
 * in most applications. The LOG_INFO() and the LOG_ERROR() macros is wrap
 * around the location calls.
 */
#pragma once
#include <sstream>

#include "logging.h"

namespace util::log {

#define LOG_TRACE()                               \
  util::log::LogStream(util::log::Loc::current(), \
                       util::log::LogSeverity::kTrace)  ///< Trace log stream
#define LOG_DEBUG()                               \
  util::log::LogStream(util::log::Loc::current(), \
                       util::log::LogSeverity::kDebug)  ///< Debug log stream
#define LOG_INFO()                                \
  util::log::LogStream(util::log::Loc::current(), \
                       util::log::LogSeverity::kInfo)  ///< Info log stream
#define LOG_ERROR()                               \
  util::log::LogStream(util::log::Loc::current(), \
                       util::log::LogSeverity::kError)  ///< Error log stream

/** \class LogStream logstream.h "logstream,h"
 * \brief This class implements a stream interface around a log message.
 *
 * The class implements a stream interface around a log message. You should not
 * uss this class directly. Instead use the LOG_INFO() and LOG_ERROR() macro
 * instead.
 *
 * Usage:
 * \code
 * LOG_INFO() << "Foo was here";
 * LOG_ERROR() << "Foo is not leaving";
 * \endcode
 */
class LogStream : public std::ostringstream {
 public:
  LogStream(const Loc &location, LogSeverity severity);  ///< Constructor
  ~LogStream() override;                                 ///< Destructor

  LogStream() = delete;
  LogStream(const LogStream &) = delete;
  LogStream(LogStream &&) = delete;
  LogStream &operator=(const LogStream &) = delete;
  LogStream &operator=(LogStream &&) = delete;

 private:
  Loc location_;          ///< File and function location.
  LogSeverity severity_;  ///< Log level of the stream
};
}  // namespace util::log
