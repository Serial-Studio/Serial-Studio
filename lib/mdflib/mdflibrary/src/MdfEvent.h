/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/ievent.h>

#include "MdfAttachment.h"
#include "MdfMetaData.h"

namespace MdfLibrary {

public enum class EventType : uint8_t {
  RecordingPeriod = 0,       ///< Specifies a recording period (range).
  RecordingInterrupt = 1,    ///< The recording was interrupted.
  AcquisitionInterrupt = 2,  ///< The data acquisition was interrupted.
  StartRecording = 3,        ///< Start recording event.
  StopRecording = 4,         ///< Stop recording event.
  Trigger = 5,               ///< Generic event (no range).
  Marker = 6                 ///< Another generic event (maybe range).
};

public enum class SyncType : uint8_t {
  SyncTime = 1,      ///< Sync value represent time (s).
  SyncAngle = 2,     ///< Sync value represent angle (rad).
  SyncDistance = 3,  ///< Sync value represent distance (m).
  SyncIndex = 4,     ///< Sync value represent sample index.
};

public enum class RangeType : uint8_t {
  RangePoint = 0,  ///< Defines a point
  RangeStart = 1,  ///< First in a range.
  RangeEnd = 2     ///< Last in a range.
};

public enum class EventCause : uint8_t {
  CauseOther = 0,   ///< Unknown source.
  CauseError = 1,   ///< An error generated this event.
  CauseTool = 2,    ///< The tool generated this event.
  CauseScript = 3,  ///< A script generated this event.
  CauseUser = 4,    ///< A user generated this event.
};

public ref class MdfEvent {
public:
  property int64_t Index { int64_t get(); }
  property String^ Name { String^ get(); void set(String^ name); }
  property String^ Description { String^ get(); void set(String^ desc); }
  property String^ GroupName { String^ get(); void set(String^ group); }
  property EventType Type { EventType get(); void set(EventType type); }
  property SyncType Sync { SyncType get(); void set(SyncType type); }
  property RangeType Range { RangeType get(); void set(RangeType type); }
  property EventCause Cause { EventCause get(); void set(EventCause cause); }
  property size_t CreatorIndex { size_t get(); void set(size_t index); }
  property int64_t SyncValue { int64_t get(); void set(int64_t value); }
  property double SyncFactor { double get(); void set(double factor); }
  property MdfEvent^ ParentEvent {MdfEvent^ get(); void set(MdfEvent^ parent); }
  property MdfEvent^ RangeEvent {MdfEvent^ get(); void set(MdfEvent^ parent); }
  // property array<int64_t>^ Scopes { array<int64_t>^ get(); }
  property array<MdfAttachment^>^ Attachments { array<MdfAttachment^>^ get(); }
  property double PreTrig { double get(); void set(double time); }
  property double PostTrig { double get(); void set(double time); }
  property MdfMetaData^ MetaData { MdfMetaData^ get(); }
  void AddAttachment(MdfAttachment^ attachment);
private:
  MdfEvent() {}
internal:
  mdf::IEvent *event_ = nullptr;
  MdfEvent(mdf::IEvent *event);
};


}
