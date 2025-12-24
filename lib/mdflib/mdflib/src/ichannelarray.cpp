/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ichannelarray.h"
#include <sstream>
namespace mdf {

uint64_t IChannelArray::NofArrayValues() const {
  return ProductOfArray();
}

std::string IChannelArray::DimensionAsString() const {
  std::ostringstream out;
  out << "N";
  for (size_t dimension = 0; dimension < Dimensions(); ++dimension) {
    out << "[" << DimensionSize(dimension) << "]";
  }
  return out.str();
}

std::vector<int64_t>& IChannelArray::DataLinks() {
  const size_t prod_dim = static_cast<size_t>( ProductOfArray() );
  if (Storage() == ArrayStorage::DgTemplate
      && data_links_.size() < prod_dim) {
    data_links_.resize(prod_dim, 0);
  }
  return data_links_;
}

std::vector<CaTripleReference>& IChannelArray::DynamicSizeList() {
  const size_t dim = Dimensions();
  if ((Flags() & CaFlag::DynamicSize) != 0 && dynamic_size_list_.size() < dim) {
    dynamic_size_list_.resize(dim);
  }
  return dynamic_size_list_;
}

std::vector<CaTripleReference>& IChannelArray::InputQuantityList() {
  const size_t dim = Dimensions();
  if ((Flags() & CaFlag::InputQuantity) != 0 && input_quantity_list_.size() < dim) {
    input_quantity_list_.resize(dim);
  }
  return input_quantity_list_;
}

std::vector<const IChannelConversion*>& IChannelArray::AxisConversionList() {
  const size_t dim = Dimensions();
  if ((Flags() & CaFlag::Axis) != 0 && axis_conversion_list_.size() < dim) {
    axis_conversion_list_.resize(dim, nullptr);
  }
  return axis_conversion_list_;
}

std::vector<CaTripleReference>& IChannelArray::AxisList() {
  const size_t dim = Dimensions();
  if ((Flags() & CaFlag::Axis) != 0 && (Flags() & CaFlag::FixedAxis) == 0 &&
      axis_list_.size() < dim) {
    axis_list_.resize(dim);
  }
  return axis_list_;
}

void IChannelArray::ResizeArrays() {
  const size_t dim = Dimensions();
  const size_t sum_of_array = static_cast<size_t>( SumOfArray() );
  const size_t prod_of_array = static_cast<size_t>( ProductOfArray() );
  if (Storage() == ArrayStorage::DgTemplate &&
      data_links_.size() != prod_of_array) {
    data_links_.resize(prod_of_array,0);
  }
  if ((Flags() & CaFlag::DynamicSize) != 0 &&
      dynamic_size_list_.size() != dim) {
    dynamic_size_list_.resize(dim);
  }
  if ((Flags() & CaFlag::InputQuantity) != 0 &&
      input_quantity_list_.size() != dim) {
    input_quantity_list_.resize(dim);
  }
  if ((Flags() & CaFlag::Axis) != 0 && (Flags() & CaFlag::FixedAxis) == 0 &&
      axis_list_.size() != dim) {
    axis_list_.resize(dim);
  }
  auto& value_list = AxisValues();
  if ((Flags() & CaFlag::FixedAxis) != 0 && value_list.size() != sum_of_array) {
    value_list.resize(sum_of_array, 0.0);
  }
  auto& cycle_list = CycleCounts();
  switch (Storage()) {
    case ArrayStorage::DgTemplate:
    case ArrayStorage::CgTemplate:
      if (cycle_list.size() != prod_of_array) {
        cycle_list.resize(prod_of_array, 0);
      }
      break;

    default:
      break;
  }
}

}  // namespace mdf