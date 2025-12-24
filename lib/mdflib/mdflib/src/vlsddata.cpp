/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "vlsddata.h"

namespace mdf::detail {

VlsdData::VlsdData(const std::string& text) {
  data_.reserve(text.size() + 1);
  data_.insert(data_.end(), text.cbegin(), text.cend());
  data_.push_back(0);
}
VlsdData::VlsdData(const std::vector<uint8_t>& data)
    : data_(data) {
}

bool VlsdData::operator<(const VlsdData& item) const {
  return data_ < item.data_;
}

bool VlsdData::operator==(const VlsdData& item) const {
  return data_ == item.data_;
}



}  // namespace mdf::detail