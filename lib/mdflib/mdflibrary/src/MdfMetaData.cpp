/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfMetaData.h"
#include "mdflibrary.h"

namespace MdfLibrary {

String^ MdfMetaData::PropertyAsString::get(String^ index) {
  return meta_data_ != nullptr ? MdfLibrary::Utf8Conversion(
   meta_data_->StringProperty(MdfLibrary::Utf8Conversion(index))) : gcnew String("");
}

void MdfMetaData::PropertyAsString::set(String^ index, String^ prop) {
  if (meta_data_ != nullptr) {
    meta_data_->StringProperty(MdfLibrary::Utf8Conversion(index),
                               MdfLibrary::Utf8Conversion(prop));
  }
}

double MdfMetaData::PropertyAsFloat::get(String^ index) {
  return meta_data_ != nullptr
             ? meta_data_->FloatProperty(MdfLibrary::Utf8Conversion(index))
             : 0.0;
}

void MdfMetaData::PropertyAsFloat::set(String^ index, double prop) {
  if (meta_data_ != nullptr) {
    meta_data_->FloatProperty(MdfLibrary::Utf8Conversion(index), prop);
  }  
}

array<MdfETag^>^ MdfMetaData::Properties::get() {
  if (meta_data_ == nullptr) {
    return gcnew array<MdfETag^>(0);    
  }
  const auto list = meta_data_->Properties();
  array<MdfETag^>^ temp_list =
    gcnew array<MdfETag^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    const auto& temp = list[index];
    temp_list[static_cast<int>(index)] = gcnew MdfETag(temp);
  }
  return temp_list;
}

array<MdfETag^>^ MdfMetaData::CommonProperties::get() {
  if (meta_data_ == nullptr) {
    return gcnew array<MdfETag^>(0);    
  }
  const auto list = meta_data_->CommonProperties();
  array<MdfETag^>^ temp_list =
    gcnew array<MdfETag^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    const auto& temp = list[index];
    temp_list[static_cast<int>(index)] = gcnew MdfETag(temp);
  }
  return temp_list;
}

void MdfMetaData::CommonProperties::set(array<MdfETag^>^ prop_list) {
  std::vector<mdf::ETag> temp_list;
  for (int index = 0; index < prop_list->Length; ++index) {
    const auto* tag = prop_list[index]->tag_;
    temp_list.emplace_back(tag != nullptr ? *tag : mdf::ETag());
  }
  if (meta_data_ != nullptr) {
    meta_data_->CommonProperties(temp_list);
  }
}

String^ MdfMetaData::XmlSnippet::get() {
  return meta_data_ != nullptr ? MdfLibrary::Utf8Conversion(
   meta_data_->XmlSnippet()) : gcnew String("");
}

void MdfMetaData::XmlSnippet::set(String^ xml) {
if (meta_data_ != nullptr) {
    meta_data_->XmlSnippet(MdfLibrary::Utf8Conversion(xml));
  }   
}

MdfETag^ MdfMetaData::GetCommonProperty(String^ name) {
  const auto temp =
      meta_data_ != nullptr
          ? meta_data_->CommonProperty(MdfLibrary::Utf8Conversion(name))
          : mdf::ETag();
  return gcnew MdfETag(temp);
}

void MdfMetaData::AddCommonProperty(MdfETag^ tag) {
  if (meta_data_ != nullptr && tag->tag_ != nullptr) {
    meta_data_->CommonProperty(*tag->tag_);
  }
}


MdfMetaData::MdfMetaData(mdf::IMetaData* meta_data)
  : meta_data_(meta_data) {
  
}

}
