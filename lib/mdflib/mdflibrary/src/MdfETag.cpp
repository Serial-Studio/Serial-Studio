/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfETag.h"
#include "mdflibrary.h"

namespace MdfLibrary {

MdfETag::MdfETag()
  : tag_(new mdf::ETag) {
}

MdfETag::MdfETag(MdfETag^& tag)
  : tag_(new mdf::ETag) {
  *tag_ = *tag->tag_;
}

MdfETag::~MdfETag() {
  this->!MdfETag();
}

String^ MdfETag::Name::get() {
  return tag_ != nullptr ? MdfLibrary::Utf8Conversion(tag_->Name()) : gcnew String("");  
}

void MdfETag::Name::set(String^ name) {
  if (tag_ != nullptr) {
    tag_->Name(MdfLibrary::Utf8Conversion(name));
 }   
}

String^ MdfETag::Description::get() {
  return tag_ != nullptr ? MdfLibrary::Utf8Conversion(tag_->Description()) : gcnew String("");    
}

void MdfETag::Description::set(String^ desc) {
  if (tag_ != nullptr) {
    tag_->Description(MdfLibrary::Utf8Conversion(desc));
  }    
}

String^ MdfETag::Unit::get() {
  return tag_ != nullptr ? MdfLibrary::Utf8Conversion(tag_->Unit()) : gcnew String("");     
}

void MdfETag::Unit::set(String^ unit) {
  if (tag_ != nullptr) {
    tag_->Unit(MdfLibrary::Utf8Conversion(unit));
  }      
}

String^ MdfETag::UnitRef::get() {
  return tag_ != nullptr ? MdfLibrary::Utf8Conversion(tag_->UnitRef()) : gcnew String("");    
}

void MdfETag::UnitRef::set(String^ unit) {
  if (tag_ != nullptr) {
    tag_->UnitRef(MdfLibrary::Utf8Conversion(unit));
  }        
}

String^ MdfETag::Type::get() {
  return tag_ != nullptr ? MdfLibrary::Utf8Conversion(tag_->Type()) : gcnew String("");   
}

void MdfETag::Type::set(String^ type) {
  if (tag_ != nullptr) {
    tag_->Type(MdfLibrary::Utf8Conversion(type));
  }    
}

ETagDataType MdfETag::DataType::get() {
  return tag_ != nullptr ?
    static_cast<ETagDataType>(tag_->DataType()) : ETagDataType::StringType; 
}

void MdfETag::DataType::set(ETagDataType type) {
  if (tag_ != nullptr) {
    tag_->DataType(static_cast<mdf::ETagDataType>( tag_->DataType()));
  }  
}

String^ MdfETag::Language::get() {
  return tag_ != nullptr ? MdfLibrary::Utf8Conversion(tag_->Language()) : gcnew String("");     
}

void MdfETag::Language::set(String^ language) {
  if (tag_ != nullptr) {
    tag_->Language(MdfLibrary::Utf8Conversion(language));
  }    
}

bool MdfETag::ReadOnly::get() {
  return tag_ != nullptr ? tag_->ReadOnly() : false;
}

void MdfETag::ReadOnly::set(bool read_only) {
  if (tag_ != nullptr) {
    tag_->ReadOnly(read_only);
  }
}

String^ MdfETag::ValueAsString::get() {
  return tag_ != nullptr ? MdfLibrary::Utf8Conversion(tag_->Value<std::string>()) : gcnew String("");    
}

void MdfETag::ValueAsString::set(String^ value) {
   if (tag_ != nullptr) {
    tag_->Value(MdfLibrary::Utf8Conversion(value));
   } 
}

double MdfETag::ValueAsFloat::get() {
  return tag_ != nullptr ? tag_->Value<double>() : 0.0;
}

void MdfETag::ValueAsFloat::set(double value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

bool MdfETag::ValueAsBoolean::get() {
  return tag_ != nullptr ? tag_->Value<bool>() : false;
}

void MdfETag::ValueAsBoolean::set(bool value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

int64_t MdfETag::ValueAsSigned::get() {
  return tag_ != nullptr ? tag_->Value<int64_t>() : 0;
}

void MdfETag::ValueAsSigned::set(int64_t value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

uint64_t MdfETag::ValueAsUnsigned::get() {
  return tag_ != nullptr ? tag_->Value<uint64_t>() : 0;
}

void MdfETag::ValueAsUnsigned::set(uint64_t value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

MdfETag::!MdfETag() {
  delete tag_;
  tag_ = nullptr;
}

MdfETag::MdfETag(const mdf::ETag& tag)
: tag_( new mdf::ETag )
{
  *tag_ = tag;
}

}
