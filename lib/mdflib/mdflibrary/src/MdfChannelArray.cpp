/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#include "MdfChannelArray.h"

namespace MdfLibrary {

ArrayType MdfChannelArray::Type::get() {
  const auto temp = array_ != nullptr ? array_->Type() : mdf::ArrayType::Array;
  return static_cast<ArrayType>(temp);
}

void MdfChannelArray::Type::set(ArrayType type) {
  if (array_ != nullptr) {
    array_->Type(static_cast<mdf::ArrayType>(type));
  }
}

ArrayStorage MdfChannelArray::Storage::get() {
  const auto temp =
      array_ != nullptr ? array_->Storage() : mdf::ArrayStorage::CnTemplate;
  return static_cast<ArrayStorage>(temp);
}

void MdfChannelArray::Storage::set(ArrayStorage storage) {
  if (array_ != nullptr) {
    array_->Storage(static_cast<mdf::ArrayStorage>(storage));
  }
}

  CaFlag MdfChannelArray::Flags::get() {
  const auto temp = array_ != nullptr ? static_cast<CaFlag>(array_->Flags())
                        : CaFlag::None;
  return temp;
  }

void MdfChannelArray::Flags::set(CaFlag flags) {
        if (array_ != nullptr) {
          array_->Flags(static_cast<uint32_t>(flags));
        }
  }
MdfChannelArray::MdfChannelArray(mdf::IChannelArray* array) { array_ = array; }

}  // namespace MdfLibrary