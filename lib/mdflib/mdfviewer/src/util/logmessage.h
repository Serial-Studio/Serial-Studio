/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file logmessage.h
 * Defines the log message structure.
 */
#pragma once
#include <chrono>
#include <string>

#include "logging.h"

namespace util::log {
std::string GetSeverityString(
    LogSeverity severity);  ///< Returns the log level as a string

/** \struct LogMessage logmessage.h "logging.h"
 *
 * Structure that holds one log message. The log message simply consist of time,
 * severity, location and text. The severity is just a fancy name for log level
 * while the location is the file, line and function where the message was
 * created.
 *
 */
struct LogMessage {
  std::chrono::time_point<std::chrono::system_clock> timestamp =
      std::chrono::system_clock::now();       ///< Time of the message (UTC)
  std::string message;                        ///< Message text
  LogSeverity severity = LogSeverity::kInfo;  ///< Type of message
  util::log::Loc location;                    ///< Source file and function
};

}  // namespace util::log
