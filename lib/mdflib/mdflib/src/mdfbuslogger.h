/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once

#include "mdf4writer.h"

namespace mdf::detail {

class MdfBusLogger : public Mdf4Writer {
 public:
  MdfBusLogger();
  ~MdfBusLogger() override;

};

}  // namespace mdf::detail
