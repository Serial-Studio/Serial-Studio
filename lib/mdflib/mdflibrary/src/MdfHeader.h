/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/iheader.h>

#include "MdfAttachment.h"
#include "MdfMetaData.h"
#include "MdfFileHistory.h"
#include "MdfEvent.h"
#include "MdfDataGroup.h"
#include "MdfFileTimestamp.h"

using namespace System;

namespace MdfLibrary {

public ref class MdfHeader {
public:
  property int64_t Index {int64_t get(); }
  property String^ Description{ String^ get(); void set(String^ desc); }  
  property String^ Author { String^ get(); void set(String^ author); }
  property String^ Department { String^ get(); void set(String^ department); }
  property String^ Project { String^ get(); void set(String^ project); }
  property String^ Subject { String^ get(); void set(String^ subject); }
  property String^ MeasurementId { String^ get(); void set(String^ uuid); }
  property String^ RecorderId { String^ get(); void set(String^ uuid); }
  property int64_t RecorderIndex { int64_t get(); void set(int64_t index); }
  property uint64_t StartTime { uint64_t get(); void set(uint64_t time); }
  
  property bool IsStartAngleUsed { bool get(); }
  property double StartAngle { double get(); void set(double angle); }
  property bool IsStartDistanceUsed { bool get(); }
  property double StartDistance { double get(); void set(double distance); }
  property MdfMetaData^ MetaData { MdfMetaData^ get(); }
  property array<MdfAttachment^>^ Attachments { array<MdfAttachment^>^ get(); }
  property array<MdfFileHistory^>^ FileHistories { array<MdfFileHistory^>^ get(); }
  property array<MdfEvent^>^ Events { array<MdfEvent^>^ get(); }
  property array<MdfDataGroup^>^ DataGroups { array<MdfDataGroup^>^ get(); }
  property MdfDataGroup^ LastDataGroup { MdfDataGroup^ get();} 

  MdfAttachment^ CreateAttachment();
  MdfFileHistory^ CreateFileHistory();
  MdfEvent^ CreateEvent();
  MdfDataGroup^ CreateDataGroup();
  MdfMetaData^ CreateMetaData();
  void SetStartTime(IMdfTimeStamp^ timestamp);
  IMdfFileTimestamp^ GetStartTime();

private:
  MdfHeader() {};
internal:
  mdf::IHeader *header_ = nullptr;
  MdfHeader(mdf::IHeader *header);
};

}
