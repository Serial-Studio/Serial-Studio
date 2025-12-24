/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/ifilehistory.h>
#include "MdfMetaData.h"
#include "MdfTimestamp.h"
#include "MdfFileTimestamp.h"

using namespace System;

namespace MdfLibrary {
public ref class MdfFileHistory {
public:
  property int64_t Index { int64_t get(); }
  property uint64_t Time { uint64_t get(); void set(uint64_t time); }
  property MdfMetaData^ MetaData
  {
    MdfMetaData^ get();
  }
  property String^ Description { String^ get(); void set(String^ desc); }
  property String^ ToolName { String^ get(); void set(String^ name); }
  property String^ ToolVendor { String^ get(); void set(String^ vendor); }
  property String^ ToolVersion { String^ get(); void set(String^ version); }
  property String^ UserName { String^ get(); void set(String^ user); }

  void SetStartTime(IMdfTimeStamp^ timestamp);
  IMdfFileTimestamp^ GetStartTime();

private:
  MdfFileHistory() {
  }

internal:
  mdf::IFileHistory* history_ = nullptr;
  MdfFileHistory(mdf::IFileHistory* history);
};
}
