/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/mdfreader.h>

#include "MdfFile.h"
#include "MdfHeader.h"
#include "MdfDataGroup.h"

using namespace System;

namespace MdfLibrary {

public ref class MdfReader {
public:
  MdfReader(String^ filename);
  virtual ~MdfReader();

  property  Int64 Index {
    Int64 get();
    void set(Int64 index);
  }

  property String^ Name {String^ get();}
  property MdfFile^ File { MdfFile^ get(); }
  property MdfHeader^ Header { MdfHeader^ get(); }
  property MdfDataGroup^ DataGroup[size_t] {
    MdfDataGroup^ get(size_t index);
  }
  property bool IsOk {bool get();}
  
  bool Open();
  void Close();
  bool ReadHeader();
  bool ReadMeasurementInfo();
  bool ReadEverythingButData();
  bool ReadData( MdfDataGroup^ group);
  bool ReadPartialData( MdfDataGroup^ group, UInt64 min_sample, UInt64 max_sample);
protected:
  !MdfReader();
private:
  MdfReader() {}
internal: 
  mdf::MdfReader* reader_ = nullptr;
};


}
