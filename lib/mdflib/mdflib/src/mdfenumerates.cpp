/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/
#include <string>
#include <sstream>

#include "mdf/mdfenumerates.h"

namespace mdf {

std::string MdfBusTypeToString( uint16_t bus_type) {
  std::ostringstream type;

  if ((bus_type & MdfBusType::CAN) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "CAN";
  }

  if ((bus_type & MdfBusType::LIN) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "LIN";
  }

  if ((bus_type & MdfBusType::FlexRay) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "FlexRay";
  }

  if ((bus_type & MdfBusType::MOST) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "MOST";
  }

  if ((bus_type & MdfBusType::Ethernet) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "Ethernet";
  }
  return type.str();
}

}
