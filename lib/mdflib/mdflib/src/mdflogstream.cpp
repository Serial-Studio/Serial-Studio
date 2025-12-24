/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdflogstream.h"

namespace {

mdf::MdfLogFunction1 LogFunction1;
mdf::MdfLogFunction2 LogFunction2;

}  // end namespace

namespace mdf {

MdfLogStream::MdfLogStream(MdfLocation location, MdfLogSeverity severity)
    : location_(std::move(location)), severity_(severity) {}

MdfLogStream::~MdfLogStream() {
  MdfLogStream::LogString(location_, severity_, str());
}

void MdfLogStream::LogString(const MdfLocation &location, MdfLogSeverity severity,
                             const std::string &text) {
  if (LogFunction1) {
    LogFunction1(location, severity, text);
  }
  if (LogFunction2) {
    std::ostringstream func;
    func << location.file << ":" << location.function;
    LogFunction2(severity, func.str(), text);
  }
}

void MdfLogStream::SetLogFunction1(const MdfLogFunction1 &func) {
  LogFunction1 = func;
}

void MdfLogStream::SetLogFunction2(const MdfLogFunction2 &func) {
  LogFunction2 = func;
}
}  // namespace mdf