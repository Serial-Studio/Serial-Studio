/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file logging.h
 * \brief Standard log interfaces.
 */
#pragma once
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <string>

#if __has_include(<source_location>)
#include <source_location>
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#endif

namespace util::log {
///< Defines the log severity level
enum class LogSeverity : uint8_t {
  kTrace = 0,  ///< Trace or listen message
  kDebug,      ///< Debug message
  kInfo,       ///< Informational message
  kNotice,     ///< Notice message. Notify the user.
  kWarning,    ///< Warning message
  kError,      ///< Error message
  kCritical,   ///< Critical message (device error)
  kAlert,      ///< Alert or alarm message
  kEmergency   ///< Fatal error message
};

/** \typedef Loc
 * The Loc is a wrapper around the std::location library. This library is new in
 * C++20.
 */

#if __has_include(<source_location>)
using Loc = std::source_location;
#elif __has_include(<experimental/source_location>)
using Loc = std::experimental::source_location;
#else
struct Loc {
  uint_least32_t line() const noexcept { return 0; }
  uint_least32_t column() const noexcept { return 0; }
  const char *file_name() const noexcept { return "UNKNOWN"; }
  const char *function_name() const noexcept { return "UNKNOWN"; }
  static constexpr Loc current() noexcept { return Loc{}; }
};
#endif

void LogDebug(const Loc &loc, const char *fmt,
              ...);  ///< Creates a debug message message
void LogInfo(const Loc &loc, const char *fmt,
             ...);  ///< Creates an information message
void LogError(const Loc &loc, const char *fmt,
              ...);  ///< Creates an error message
void LogString(const Loc &loc, LogSeverity severity,
               const std::string &message);  ///< Creates a generic message

/** \brief Search and returns the path to the Notepad application..
 * Search primary for the Notepad++ application and second for the Notepad
 * application.
 * @return The path to the Notepad++ or the Notepad application.
 */
std::string FindNotepad();

/** \brief Backup up a file with the 9 last changes.
 *
 * Backup a file by adding a sequence number 0..9 to the file (file_N.ext).
 * @param filename Full path to the file.
 * @param remove_file If set to true the file will be renamed to file_0. If set
 * to false the file will copy its content to file_0. The latter is slower but
 * safer.
 * @return True if successful
 */
bool BackupFiles(const std::string &filename, bool remove_file = true);

}  // namespace util::log
