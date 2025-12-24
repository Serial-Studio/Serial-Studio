/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <cstdint>

namespace mdf::detail {
enum class BlockItemType {
  NormalItem = 0,
  LinkItem = 1,
  HeaderItem = 2,
  BlankItem = 3
};

class BlockProperty final {
 public:
  explicit BlockProperty(const std::string& label, const std::string& value,
                         const std::string& desc = {},
                         BlockItemType type = BlockItemType::NormalItem);

  [[nodiscard]] const std::string& Label() const { return label_; }

  [[nodiscard]] const std::string& Value() const { return value_; }

  [[nodiscard]] const std::string& Description() const { return description_; }

  [[nodiscard]] BlockItemType Type() const { return type_; }

  [[nodiscard]] int64_t Link() const;

 private:
  std::string label_;
  std::string value_;
  std::string description_;
  BlockItemType type_ = BlockItemType::NormalItem;
};

}  // Namespace mdf::detail
