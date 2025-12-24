/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/iattachment.h>
#include "MdfMetaData.h"
using namespace System;

namespace MdfLibrary {

public ref class MdfAttachment {
public:

  property Int64 Index { Int64 get(); }
  property UInt16 CreatorIndex {UInt16 get(); void set(UInt16 index);}
  property bool Embedded {bool get(); void set(bool embedded); }
  property bool Compressed {bool get(); void set(bool compressed); }
  property String^ Md5 {String^ get(); }
  property String^ FileName { String^ get(); void set(String^ name); }
  property String^ FileType { String^ get(); void set(String^ type); }
  property MdfMetaData^ MetaData { MdfMetaData^ get();}
  MdfMetaData^ CreateMetaData();
  
private:
  MdfAttachment() {};

internal:
  mdf::IAttachment* attachment_ = nullptr;
  MdfAttachment(mdf::IAttachment *attachment);
};
}
