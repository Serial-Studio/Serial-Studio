/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/ichannelconversion.h>
#include "MdfMetaData.h"

using namespace System;

namespace MdfLibrary {

public enum class ConversionType : Byte {
  NoConversion = 0,
  Linear = 1,
  Rational = 2,
  Algebraic = 3,
  ValueToValueInterpolation = 4,
  ValueToValue = 5,
  ValueRangeToValue = 6,
  ValueToText = 7,
  ValueRangeToText = 8,
  TextToValue = 9,
  TextToTranslation = 10,
  BitfieldToText = 11,
  
  // MDF 3 types
  Polynomial = 30,
  Exponential = 31,
  Logarithmic = 32,
  DateConversion = 33,
  TimeConversion = 34
};

public ref class MdfChannelConversion {
public:
  literal UInt16 PrecisionValid = 0x0001;
  literal UInt16 RangeValid = 0x0002;
  literal UInt16 StatusString = 0x0004;

  property Int64 Index { Int64 get(); }
  property String^ Name { String^ get(); void set(String^ name); }
  property String^ Description { String^ get(); void set(String^ desc); }
  
  property bool UnitUsed { bool get(); }
  property String^ Unit { String^ get(); void set(String^ unit); }
  
  property ConversionType Type {
    ConversionType get();
    void set(ConversionType type);
  }
  property bool PrecisionUsed{ bool get(); }
  property Byte Precision { Byte get(); void set(Byte precision); }
  
  property bool RangeUsed { bool get(); }
  property Tuple<double, double>^ Range {
    Tuple<double, double>^ get();
    void set(Tuple<double, double>^);
  };

  property UInt16 Flags { UInt16 get(); void set(UInt16 flags); }
  property MdfChannelConversion^ Inverse { MdfChannelConversion^ get(); }
  property MdfMetaData^ MetaData { MdfMetaData^ get(); }
  property String^ Formula { String^ get(); void set(String^ formula); }

  property uint16_t NofParameters { uint16_t get();  }
  double Parameter(uint16_t index);
  void Parameter(uint16_t index, double parameter);

  uint64_t ParameterUInt( uint16_t index);
  void ParameterUInt(uint16_t index, uint64_t parameter);
  
  
  property uint16_t NofReferences { uint16_t get();  }
  String^ Reference( uint16_t index);
  void Reference(uint16_t index, String^ reference);
  
  
  MdfChannelConversion^ CreateInverse();
  MdfMetaData^ CreateMetaData();
private:
  MdfChannelConversion() {};

internal:
  // Note that the pointer is destroyed by the MdfReader object
  mdf::IChannelConversion *conversion_ = nullptr;
  MdfChannelConversion(mdf::IChannelConversion *conversion);
};

}