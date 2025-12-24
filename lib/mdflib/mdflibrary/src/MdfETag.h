/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/etag.h>
using namespace System;

namespace MdfLibrary {

public enum class ETagDataType : uint8_t {
  StringType = 0,
  DecimalType = 1,
  IntegerType = 2,
  FloatType = 3,
  BooleanType = 4,
  DateType = 5,
  TimeType = 6,
  DateTimeType = 7
};

public ref class MdfETag {
public:
  MdfETag();
  MdfETag(MdfETag^ &tag);
  virtual ~MdfETag();
  
  property String^ Name {String^ get(); void set(String^ name); }
  property String^ Description {String^ get(); void set(String^ desc); }
  property String^ Unit {String^ get(); void set(String^ unit); }
  property String^ UnitRef {String^ get(); void set(String^ unit); }
  property String^ Type {String^ get(); void set(String^ type); }
  property ETagDataType DataType {
    ETagDataType get(); 
    void set(ETagDataType type);
  }
  property String^ Language {String^ get(); void set(String^ language); }
  property bool ReadOnly { bool get(); void set(bool read_only); }

  property String^ ValueAsString { String^ get(); void set(String^ value); }
  property double ValueAsFloat { double get(); void set(double value); }
  property bool ValueAsBoolean { bool get(); void set(bool value); }
  property int64_t ValueAsSigned { int64_t get(); void set(int64_t value); }
  property uint64_t ValueAsUnsigned { uint64_t get(); void set(uint64_t value); }
protected:
  !MdfETag();
internal:
  mdf::ETag *tag_ = nullptr;
  MdfETag(const mdf::ETag& tag);
};

}
