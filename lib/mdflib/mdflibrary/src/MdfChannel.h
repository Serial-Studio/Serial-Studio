/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/ichannel.h>
#include "MdfChannelConversion.h"
#include "MdfChannelArray.h"
#include "MdfSourceInformation.h"

using namespace System;
namespace MdfLibrary {

public enum class ChannelType : uint8_t {
  FixedLength = 0,
  VariableLength = 1,
  Master = 2,
  VirtualMaster = 3,
  Sync = 4,
  MaxLength = 5,
  VirtualData = 6
};

public enum class ChannelSyncType : uint8_t {
  None = 0,
  Time = 1,
  Angle = 2,
  Distance = 3,
  Index = 4
};

public enum class ChannelDataType : uint8_t {
  UnsignedIntegerLe = 0,
  UnsignedIntegerBe = 1,
  SignedIntegerLe = 2,
  SignedIntegerBe = 3,
  FloatLe = 4,
  FloatBe = 5,
  StringAscii = 6,
  StringUTF8 = 7,
  StringUTF16Le = 8,
  StringUTF16Be = 9,
  ByteArray = 10,
  MimeSample = 11,
  MimeStream = 12,
  CanOpenDate = 13,
  CanOpenTime = 14,
  ComplexLe = 15,
  ComplexBE = 16
};

public enum class CnFlag : uint32_t {
    None = 0b0000'0000,
    AllValuesInvalid = 0b0000'0001,
    InvalidValid = 0b0000'0010,
    PrecisionValid = 0b0000'0100,
    RangeValid = 0b0000'1000,
    LimitValid = 0b0001'0000,
    ExtendedLimitValid = 0b0010'0000,
    Discrete = 0b0100'0000,
    Calibration = 0b1000'0000,
    Calculated = 0b0000'0001'0000'0000,
    Virtual = 0b0000'0010'0000'0000,
    BusEvent = 0b0000'0100'0000'0000,
    StrictlyMonotonous = 0b0000'1000'0000'0000,
    DefaultX = 0b0001'0000'0000'0000,
    EventSignal = 0b0010'0000'0000'0000,
    VlsdDataStream = 0b0100'0000'0000'0000
 };

public ref class MdfChannel {
public:
  property int64_t Index { int64_t get(); }
  property String^ Name { String^ get(); void set(String^ name); }
  property String^ DisplayName { String^ get(); void set(String^ name); }
  property String^ Description { String^ get(); void set(String^ desc); }
  
  property bool UnitUsed { bool get(); }
  property String^ Unit { String^ get(); void set(String^ unit); }

  property ChannelType Type { ChannelType get(); void set(ChannelType type); }
  property ChannelSyncType Sync {
    ChannelSyncType get();
    void set(ChannelSyncType type);
  }
  property ChannelDataType DataType {
    ChannelDataType get();
    void set(ChannelDataType type);
  }
  property CnFlag Flags { CnFlag get(); void set(CnFlag flags); }

  property uint64_t DataBytes { uint64_t get(); void set(uint64_t bytes); };  

  property bool PrecisionUsed { bool get(); }
  property Byte Precision { Byte get(); };
  
  property bool RangeUsed { bool get(); }
  property Tuple<double,double>^ Range {
    Tuple<double,double>^ get();
    void set(Tuple<double,double>^);
  };
  
  property bool LimitUsed { bool get(); }
  property Tuple<double,double>^ Limit {
    Tuple<double,double>^ get();
    void set(Tuple<double,double>^);
  };

  property bool ExtLimitUsed { bool get(); }
  property Tuple<double,double>^ ExtLimit {
    Tuple<double,double>^ get();
    void set(Tuple<double,double>^);
  };

  property double SamplingRate { double get(); void set(double rate); }
  property uint64_t VlsdRecordId { uint64_t get(); void set(uint64_t recordId);}

  property uint32_t BitCount { uint32_t get(); void set(uint32_t bits);}
  property uint16_t BitOffset { uint16_t get(); void set(uint16_t bits);}
  property MdfSourceInformation^ SourceInformation {
    MdfSourceInformation^ get();
  }
  
  property MdfChannelConversion^ ChannelConversion {
      MdfChannelConversion^ get();
  }
  
  property array<MdfChannel^>^ ChannelCompositions {
      array<MdfChannel^>^ get();
  }
  
  property MdfMetaData^ MetaData {
      MdfMetaData^ get();
  }
  
  MdfSourceInformation^ CreateSourceInformation(); 
  MdfChannelConversion^ CreateChannelConversion();
  MdfChannel^ CreateChannelComposition();
  MdfMetaData^ CreateMetaData();
  
  void SetChannelValue(const int64_t value) { SetChannelValue(value, true); };
  void SetChannelValue(const uint64_t value) { SetChannelValue(value, true); };
  void SetChannelValue(const double value) { SetChannelValue(value, true); };
  void SetChannelValue(String^ value) { SetChannelValue(value, true); };
  void SetChannelValue(array<Byte>^ value) { SetChannelValue(value, true); };

  void SetChannelValue(const int64_t value, bool valid);
  void SetChannelValue(const uint64_t value, bool valid);
  void SetChannelValue(const double value, bool valid);
  void SetChannelValue(String^ value, bool valid);
  void SetChannelValue(array<Byte>^ value, bool valid);

 private:
  MdfChannel() {}
internal:
  mdf::IChannel* channel_ = nullptr;
  MdfChannel(mdf::IChannel* channel);


};
}