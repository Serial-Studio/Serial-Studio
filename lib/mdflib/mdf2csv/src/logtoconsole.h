/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once

#include <mdf/mdflogstream.h>

namespace mdf {

class LogToConsole {
 public:
  LogToConsole();
  static void LogFunction(const MdfLocation& location, MdfLogSeverity severity,
                   const std::string& text);
 private:
  static void SeverityToText(MdfLogSeverity severity);
  static void LogLocation(const MdfLocation& location);
};

}  // namespace mdf


