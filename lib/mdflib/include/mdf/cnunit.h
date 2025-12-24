/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>

#include "mdf/ccunit.h"


namespace mdf {

class CnUnit : public CcUnit {
 public:
  CnUnit();
  explicit CnUnit(std::string unit);
};

}  // namespace mdf


