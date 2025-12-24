/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/cnunit.h"

namespace mdf {
CnUnit::CnUnit()
: CcUnit() {
  block_name_ ="CN";
}

CnUnit::CnUnit(std::string unit)
: CcUnit(std::move(unit) ) {
  block_name_ ="CN";
}

}  // namespace mdf