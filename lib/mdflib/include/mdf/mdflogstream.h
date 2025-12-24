/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file mdflogstream.h
 * \brief The mdf log stream file is intended to isolate the logging so
 * the library can be built without dependency of the util and boost libraries.
 * The applications in the library do however include the above libraries.
 */
#pragma once

#include <sstream>
#include <string>

/** \brief This is a replacement for the std::source_location library.
 * The standard source_location library cannot be used due to user requirement
 * of C++ 17 version.
 */
struct MdfLocation {
  int line = 0;      ///< Source code line.
  int column = 0;    ///< Source code column.
  std::string file;  ///< Source code file name (avoid path).
  std::string function; ///< Source code function.
};

#include "mdf/mdffactory.h"

namespace mdf {


#define MDF_TRACE() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kTrace)  ///< Trace log message
#define MDF_DEBUG() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kDebug)  ///< Debug log message
#define MDF_INFO() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kInfo)  ///< Info log message
#define MDF_ERROR() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kError)  ///< Error log message

/** \brief MDF log function definition. */
using MdfLogFunction1 = std::function<void(const MdfLocation &location,
  MdfLogSeverity severity, const std::string &text)>;

/** \brief MDF log stream interface.
 *
 *
 */
class MdfLogStream : public std::ostringstream {
 public:
  MdfLogStream(MdfLocation location, MdfLogSeverity severity);  ///< Constructor
  ~MdfLogStream() override;                                    ///< Destructor

  MdfLogStream() = delete;
  MdfLogStream(const MdfLogStream&) = delete;
  MdfLogStream(MdfLogStream&&) = delete;
  MdfLogStream& operator=(const MdfLogStream&) = delete;
  MdfLogStream& operator=(MdfLogStream&&) = delete;

  /** \brief Sets a log function. */
  static void SetLogFunction1(const MdfLogFunction1& func);
  /** \brief Sets a log function. */
  static void SetLogFunction2(const MdfLogFunction2& func);
  
 protected:
  MdfLocation location_;     ///< File and function location.
  MdfLogSeverity severity_;  ///< Log level of the stream

  /** \brief Defines the logging function. */
  virtual void LogString(const MdfLocation& location, MdfLogSeverity severity,
                         const std::string& text);
};

}  // namespace mdf
