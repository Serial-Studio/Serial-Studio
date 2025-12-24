/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/etag.h"

#include "mdf/mdfhelper.h"
#include "platform.h"

namespace mdf {

template <>
void ETag::Value(const bool& value) {
  value_ = value ? "true" : "false";
}

template <>
[[nodiscard]] bool ETag::Value() const {
  if (value_.empty()) {
    return false;
  }
  return value_[0] == 'T' || value_[0] == 't' || value_[0] == 'Y' ||
         value_[0] == 'y' || value_[0] == '1';
}

template <>
[[nodiscard]] std::string ETag::Value() const {
  return value_;
}

void ETag::DataType(ETagDataType type) {
  switch (type) {
    case ETagDataType::DecimalType:
      type_ = "decimal";
      break;
    case ETagDataType::IntegerType:
      type_ = "integer";
      break;
    
    case ETagDataType::FloatType:
      type_ = "float";
      break;
    
    case ETagDataType::BooleanType:
      type_ = "boolean";
      break;
    
    case ETagDataType::DateType:
      type_ = "date";
      break;
    
    case ETagDataType::TimeType:
      type_ = "time";
      break;
    
    case ETagDataType::DateTimeType:
      type_ = "dateTime";
      break;
        
    default:
      type_ = "string";
      break;
  }
}

ETagDataType ETag::DataType() const {
  if (Platform::stricmp(type_.c_str(), "decimal") == 0) {
    return ETagDataType::DecimalType;
  } else if (Platform::stricmp(type_.c_str(), "integer") == 0) {
    return ETagDataType::IntegerType;
  } else if (Platform::stricmp(type_.c_str(), "float") == 0) {
    return ETagDataType::FloatType;
  } else if (Platform::stricmp(type_.c_str(), "boolean") == 0) {
    return ETagDataType::BooleanType;
  } else if (Platform::stricmp(type_.c_str(), "date") == 0) {
    return ETagDataType::DateType;
  } else if (Platform::stricmp(type_.c_str(), "time") == 0) {
    return ETagDataType::TimeType;
  } else if (Platform::stricmp(type_.c_str(), "dateTime") == 0) {
    return ETagDataType::DateTimeType;
  }
  
  return ETagDataType::StringType;
}

void ETag::AddTag(const ETag& tag) { tree_list_.push_back(tag); }

}  // namespace mdf
