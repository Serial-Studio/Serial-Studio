/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "logmessage.h"

#include <string>

namespace util::log {
std::string GetSeverityString(LogSeverity severity) {
  switch (severity) {
    case LogSeverity::kTrace:
      return "[Trace]";
    case LogSeverity::kDebug:
      return "[Debug]";
    case LogSeverity::kInfo:
      return "[Info]";
    case LogSeverity::kNotice:
      return "[Notify]";
    case LogSeverity::kWarning:
      return "[Warning]";
    case LogSeverity::kError:
      return "[Error]";
    case LogSeverity::kCritical:
      return "[Critical]";
    case LogSeverity::kAlert:
      return "[Alert]";
    case LogSeverity::kEmergency:
      return "[Emergency]";
    default:
      break;
  }
  return "[Unknown]";
}
}  // namespace util::log