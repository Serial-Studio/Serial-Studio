/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <mdf/ichannelarray.h>

using namespace System;

namespace MdfLibrary {

/** \brief Type of array.
 *
 */
public enum class ArrayType : uint8_t {
  Array = 0,                ///< Array
  ScalingAxis = 1,          ///< Scaling axis.
  LookUp = 2,               ///< Lookup array.
  IntervalAxis = 3,         ///< Interval axis.
  ClassificationResult = 4  ///< Classification result.
};

/** \brief Type of storage.
 *
 */
public enum class ArrayStorage : uint8_t {
   CnTemplate = 0,  ///< Channel template.
   CgTemplate = 1,  ///< Channel group template.
   DgTemplate = 2   ///< Data group template.
 };

/** \brief Channel array flags..
 *
 */
public enum class CaFlag : uint32_t {
   None = 0b0000'0000, 		          ///< No flags.
   DynamicSize = 0b0000'0001,         ///< Dynamic size
   InputQuantity = 0b0000'0010,       ///< Input quantity.
   OutputQuantity = 0b0000'0100,      ///< Output quantity.
   ComparisonQuantity = 0b0000'1000,  ///< Comparison quantity.
   Axis = 0b0001'0000,                ///< Axis
   FixedAxis = 0b0010'0000,           ///< Fixed axis.
   InverseLayout = 0b0100'0000,       ///< Inverse layout.
   LeftOpenInterval = 0b1000'0000,    ///< Left-over interval.
   StandardAxis = 0b0001'0000'0000    ///< Standard axis.
 };

public ref class MdfChannelArray {
public:
  property ArrayType Type {
    ArrayType get();
    void set(ArrayType type);
  }
  property ArrayStorage Storage {
	ArrayStorage get();
	void set(ArrayStorage storage);
  }
  property CaFlag Flags {
        CaFlag get();
        void set(CaFlag flags);
  }

 private:
  MdfChannelArray() {}

 internal: 
  mdf::IChannelArray *array_ = nullptr;
  MdfChannelArray(mdf::IChannelArray *array_);
};
}  // namespace mdf
