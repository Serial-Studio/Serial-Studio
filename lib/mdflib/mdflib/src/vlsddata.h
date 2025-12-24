/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once
#include <cstdint>
#include <vector>
#include <string>
namespace mdf::detail {

class VlsdData {
 public:
  explicit VlsdData(const std::vector<uint8_t>& data);
  explicit VlsdData(const std::string& text);

  [[nodiscard]] bool operator < (const VlsdData&  item) const;
  [[nodiscard]] bool operator == (const VlsdData&  item) const;

  [[nodiscard]] uint32_t Size() const {
    return static_cast<uint32_t>(data_.size());
  }

  [[nodiscard]] auto Cbegin() const {
    return data_.cbegin();
  }

  [[nodiscard]] auto Cend() const {
    return data_.cend();
  }
 private:
  std::vector<uint8_t> data_;
};

}  // namespace mdf
