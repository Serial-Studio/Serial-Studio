/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <mdf/imetadata.h>
#include "MdfETag.h"

using namespace System;
namespace MdfLibrary {

public ref class MdfMetaData {
public:
  property String^ PropertyAsString[String^] {
    String^ get(String^ index);
    void set(String^ index, String^ prop);
  }
  
  property double PropertyAsFloat[String^] {
    double get(String^ index);
    void set(String^ index, double prop);
  }
  
  property array<MdfETag^>^ Properties {
    array<MdfETag^>^ get();
  }
  
  property array<MdfETag^>^ CommonProperties {
    array<MdfETag^>^ get();
    void set(array<MdfETag^>^ prop_list);
  }

  property String^ XmlSnippet { String^ get(); void set(String^ xml); }
  
  MdfETag^ GetCommonProperty(String^ name);
  void AddCommonProperty(MdfETag^ tag);

  MdfMetaData() {}
internal:
  mdf::IMetaData * meta_data_ = nullptr;
  MdfMetaData(mdf::IMetaData *meta_data);
};
}
