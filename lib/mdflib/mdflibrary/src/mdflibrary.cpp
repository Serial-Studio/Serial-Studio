/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <string>
#include <locale>

#include <mdf/mdffactory.h>
#include <mdf/mdfreader.h>
#include <mdf/mdfhelper.h>
#include <msclr/marshal_cppstd.h>

#include "mdflibrary.h"

using namespace msclr::interop;

namespace {

void LogFunc(mdf::MdfLogSeverity severity,
  const std::string& function,
  const std::string& message) {
  const auto sev = static_cast<MdfLibrary::MdfLogSeverity>(severity);
  auto func = gcnew String(function.c_str());
  auto text = gcnew String(message.c_str());
  
  MdfLibrary::MdfLibrary::Instance->FireLogEvent(sev, func, text);
}

void NoLog(mdf::MdfLogSeverity severity,
             const std::string& function,
             const std::string& message) {
}

}
namespace MdfLibrary {
MdfLibrary::MdfLibrary() {
  mdf::MdfFactory::SetLogFunction2(LogFunc);  
}

MdfLibrary::!MdfLibrary() {
  mdf::MdfFactory::SetLogFunction2(NoLog);
}

MdfLibrary::~MdfLibrary() {
  this->!MdfLibrary();
}

MdfLibrary^ MdfLibrary::Instance::get() {
  if (instance_ == nullptr) {
    instance_ = gcnew MdfLibrary();
  }
  return instance_;
}

bool MdfLibrary::IsMdfFile(String^ filename) {
  return mdf::IsMdfFile(Utf8Conversion(filename));
}

MdfChannelObserver^ MdfLibrary::CreateChannelObserver(MdfDataGroup^ data_group,
    MdfChannelGroup^ channel_group, MdfChannel^ channel) {
  if (data_group == nullptr || channel_group == nullptr ||
    channel == nullptr) {
    return nullptr;
  }
  if (data_group->group_ == nullptr || channel_group->group_ == nullptr
      || channel->channel_ == nullptr) {
    return nullptr;
  }
  auto observer = mdf::CreateChannelObserver(*data_group->group_,
    *channel_group->group_, *channel->channel_);
  if (!observer) {
    return nullptr;
  }
  auto temp = gcnew MdfChannelObserver(observer.get());
  observer.release();
  return temp;
}

MdfChannelObserver^ MdfLibrary::CreateChannelObserverByChannelName(
    MdfDataGroup^ data_group, String^ channel_name) {
  if (data_group == nullptr || String::IsNullOrEmpty(channel_name)) {
    return nullptr;
  }
  if (data_group->group_ == nullptr) {
    return nullptr;
  }
  auto observer = mdf::CreateChannelObserver(*data_group->group_,
   Utf8Conversion(channel_name));
  if (!observer) {
    return nullptr;
  }
  auto temp = gcnew MdfChannelObserver(observer.release());
  return temp;  
}

array<MdfChannelObserver^>^ MdfLibrary::CreateChannelObserverForChannelGroup(
    MdfDataGroup^ data_group, MdfChannelGroup^ channel_group) {
  if (data_group == nullptr || channel_group == nullptr
    || data_group->group_ == nullptr || channel_group->group_ == nullptr) {
    return gcnew array<MdfChannelObserver^>(0);
  }

  mdf::ChannelObserverList list;
  mdf::CreateChannelObserverForChannelGroup(*data_group->group_,
    *channel_group->group_, list);
  array<MdfChannelObserver^> ^temp = 
      gcnew array<MdfChannelObserver^>(static_cast<int>(list.size()));
  for (int index = 0; index < static_cast<int>(list.size()); ++index) {
    auto& observer = list[index];
    temp[index] = gcnew MdfChannelObserver(observer.release());
  }
  return temp;  
}

array<MdfChannelObserver^>^ MdfLibrary::CreateChannelObserverForDataGroup(
    MdfDataGroup^ data_group) {
  if (data_group == nullptr || data_group->group_ == nullptr) {
    return gcnew array<MdfChannelObserver^>(0);
  }
 
  mdf::ChannelObserverList list;
  mdf::CreateChannelObserverForDataGroup(*data_group->group_, list);
  array<MdfChannelObserver^> ^temp = 
      gcnew array<MdfChannelObserver^>(static_cast<int>(list.size()));
  for (int index = 0; index < static_cast<int>(list.size()); ++index) {
    auto& observer = list[index];
    temp[index] = gcnew MdfChannelObserver(observer.release());
  }
  return temp;  
}

String^ MdfLibrary::Utf8Conversion(const std::string& utf8_string) {
  array<byte>^ c_array =
    gcnew array<byte>(static_cast<int>(utf8_string.length()));
  for (size_t i = 0; i < utf8_string.length(); i++) {
      c_array[static_cast<int>(i)] = utf8_string[i];
  }

  System::Text::Encoding^ u8enc = System::Text::Encoding::UTF8;
  return u8enc->GetString(c_array);
}

std::string MdfLibrary::Utf8Conversion(String^ text) {

  array<byte>^ c_array = System::Text::Encoding::UTF8->GetBytes(text);

  std::string utf8_string;
  utf8_string.resize(c_array->Length);
  for (int i = 0; i < c_array->Length; i++) utf8_string[i] = c_array[i];
  return utf8_string;
 
}

void MdfLibrary::AddLog(MdfLogSeverity severity, String ^ function,
                             String ^ message) {
  FireLogEvent(severity, function, message);
}

uint64_t MdfLibrary::NowNs() {
  return mdf::MdfHelper::NowNs();
}

void MdfLibrary::FireLogEvent(MdfLogSeverity severity, String^ function,
                              String^ message) {
  LogEvent(severity, function, message);
}

}
