/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "logtoconsole.h"
#include "programargument.h"
#include <iostream>
#include <filesystem>

using namespace std::filesystem;

namespace mdf {

LogToConsole::LogToConsole() {
  MdfLogStream::SetLogFunction1(LogToConsole::LogFunction);
}

void LogToConsole::LogFunction(const MdfLocation& location,
                               MdfLogSeverity severity,
                               const std::string& text) {
  const ProgramArgument& arguments = ProgramArgument::Instance();
  if (arguments.NonInteractive()) {
    return;
  }

  switch (arguments.LogLevel()) {

    case 0: // Show information, debug and trace
      if (severity <= MdfLogSeverity::kInfo) {
        return;
      }
      break;

    case 1: // Show error and log
      if (severity <= MdfLogSeverity::kDebug) {
        return;
      }
      break;

    default: // Show everything
      break;
  }
  SeverityToText(severity);
  std::cout << text;
  LogLocation(location);
  std::cout << std::endl;
}

void LogToConsole::SeverityToText(MdfLogSeverity severity) {
  switch (severity) {

    case MdfLogSeverity::kDebug:
      std::cout << "[Debug] ";
      break;

    case MdfLogSeverity::kInfo:
      std::cout << "[Info] ";
      break;

    case MdfLogSeverity::kNotice:
      std::cout << "[Notice] ";
      break;

    case MdfLogSeverity::kWarning:
      std::cout << "[Warning] ";
      break;

    case MdfLogSeverity::kError:
      std::cout << "[Error] ";
      break;

    case MdfLogSeverity::kCritical:
      std::cout << "[Critical] ";
      break;

    case MdfLogSeverity::kAlert:
      std::cout << "[Alert] ";
      break;

    case MdfLogSeverity::kEmergency:
      std::cout << "[Emergency] ";
      break;

    default:
      std::cout << "[Trace] ";
      break;
  }
}

void LogToConsole::LogLocation(const MdfLocation& location) {
  if (!location.file.empty() && !location.function.empty()) {
    try {
      path fullname(location.file);
      std::cout << "\t[" << fullname.filename().string()
                << "::" << location.function
                << ":" << location.line << "] " ;
    } catch (const std::exception&) {

    }
  }
}


}  // namespace mdf