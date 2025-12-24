/*
* Copyright 2022 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "mdfbuslogger.h"

namespace mdf::detail {
MdfBusLogger::MdfBusLogger() {
  type_of_writer_ = MdfWriterType::MdfBusLogger;
}

MdfBusLogger::~MdfBusLogger() {
  write_cache_.Exit();
}


}  // namespace mdf