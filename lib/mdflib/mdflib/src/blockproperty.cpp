/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "blockproperty.h"

#include <sstream>

namespace mdf::detail {

BlockProperty::BlockProperty(const std::string &label, const std::string &value,
                             const std::string &desc, BlockItemType type)
    : label_(label), value_(value), description_(desc), type_(type) {}
/// Converts the the string value to a file position of the block.
/// It assumes the value is formatted as an hex value so this function
/// is only valid for link property types.
/// \return File position of the referenced block
int64_t BlockProperty::Link() const {
  if (Type() != BlockItemType::LinkItem) {
    return 0;
  }
  if (value_.size() <= 2) {
    return 0;
  }
  int64_t pos = 0;
  try {
    std::stringstream temp;
    temp << std::hex << value_.substr(2);
    temp >> pos;
  } catch (const std::exception &) {
    return 0;
  }
  return pos;
}

}  // namespace mdf::detail