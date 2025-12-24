/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/ichannelobserver.h"
#include "MdfChannel.h"

using namespace System;

namespace MdfLibrary {

public ref class MdfChannelObserver {
public:
  virtual ~MdfChannelObserver();

  property uint64_t NofSamples { uint64_t get(); }
  property String^ Name { String^ get(); }
  property String^ Unit { String^ get(); }
  property MdfChannel^ Channel { MdfChannel^ get(); }

  bool IsMaster();
  
  bool GetChannelValueAsUnsigned(uint64_t sample, uint64_t% value);
  bool GetChannelValueAsSigned(uint64_t sample, int64_t% value);
  bool GetChannelValueAsFloat(uint64_t sample, double% value);  
  bool GetChannelValueAsString(uint64_t sample, String^% value);    
  bool GetChannelValueAsArray(uint64_t sample,  array<Byte>^% value);

   bool GetEngValueAsUnsigned(uint64_t sample, uint64_t% value);
   bool GetEngValueAsSigned(uint64_t sample, int64_t% value);
   bool GetEngValueAsFloat(uint64_t sample, double% value);  
   bool GetEngValueAsString(uint64_t sample, String^% value);    
   bool GetEngValueAsArray(uint64_t sample,  array<Byte>^% value);
  
protected:
  !MdfChannelObserver();
private:
  MdfChannelObserver() {}
  MdfChannelObserver(MdfChannelObserver^ &observer) {}
internal:  
  mdf::IChannelObserver *observer_ = nullptr;
  MdfChannelObserver(mdf::IChannelObserver *observer);
};

}
