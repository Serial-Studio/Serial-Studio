/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <mdf/isourceinformation.h>
#include "MdfMetaData.h"

using namespace System;
namespace MdfLibrary {

public enum class SourceType : uint8_t {
  Other = 0,
  Ecu = 1,
  Bus = 2,
  IoDevice = 3,
  Tool = 4,
  User = 5
};

public enum class BusType : uint8_t {
  None = 0,
  Other = 1,
  Can = 2,
  Lin = 3,
  Most = 4,
  FlexRay = 5,
  Kline = 6,
  Ethernet = 7,
  Usb = 8
};

public ref class MdfSourceInformation {
public:
  literal uint8_t Simulated = 0x01;
  
  property int64_t Index {int64_t get(); }
  property String^ Name { String^ get(); void set(String^ name); }
  property String^ Description { String^ get(); void set(String^ desc); }
  property String^ Path { String^ get(); void set(String^ path); }
  property SourceType Type {SourceType get(); void set(SourceType type); }
  property BusType Bus {BusType get(); void set(BusType bus); }  
  property uint8_t Flags {uint8_t get(); void set(uint8_t flags); }
  property MdfMetaData^ MetaData { MdfMetaData^ get();}

  MdfMetaData^ CreateMetaData();

 private:
  MdfSourceInformation() {}
internal:
  mdf::ISourceInformation *info_ = nullptr;
  MdfSourceInformation(mdf::ISourceInformation *info);
};

}