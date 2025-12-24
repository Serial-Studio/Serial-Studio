/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ievent.h"

namespace mdf {

void IEvent::Description(const std::string& description) {
  auto* metadata = CreateMetaData();
  if (metadata != nullptr) {
    metadata->StringProperty("TX", description);
  }
}

std::string IEvent::Description() const {
  const auto* metadata = MetaData();
  return metadata != nullptr ? metadata->StringProperty("TX") : std::string();
}

void IEvent::PreTrig(double pre_trig) {
  auto* metadata = CreateMetaData();
  if (metadata != nullptr) {
    metadata->FloatProperty("pre_trigger_interval", pre_trig);
  }
}

double IEvent::PreTrig() const {
  const auto* metadata = MetaData();
  return metadata != nullptr ? metadata->FloatProperty("pre_trigger_interval")
                             : 0.0;
}

void IEvent::PostTrig(double post_trig) {
  auto* metadata = CreateMetaData();
  if (metadata != nullptr) {
    metadata->FloatProperty("post_trigger_interval", post_trig);
  }
}

double IEvent::PostTrig() const {
  const auto* metadata = MetaData();
  return metadata != nullptr ? metadata->FloatProperty("post_trigger_interval")
                             : 0.0;
}

std::string IEvent::TypeToString() const {
  switch (Type()) {
    case EventType::RecordingPeriod:
      return "Recording";
    case EventType::RecordingInterrupt:
      return "Recording Interrupt";
    case EventType::AcquisitionInterrupt:
      return "Acquisition Interrupt";
    case EventType::StartRecording:
      return "Start Recording";
    case EventType::StopRecording:
      return "Stop Recording";
    case EventType::Trigger:
      return "Trigger";
    case EventType::Marker:
      return "Marker";
    default:
      break;
  }
  return {};
}

std::string IEvent::ValueToString() const {
  const double value = static_cast<double>(SyncValue()) * SyncFactor();
  std::string unit;
  switch (Sync()) {
    case SyncType::SyncTime:
      unit = "s";
      break;

    case SyncType::SyncAngle:
      unit = "rad";
      break;

    case SyncType::SyncDistance:
      unit = "m";
      break;

    case SyncType::SyncIndex:
      unit = "#";
      break;

    default:
      break;
  }
  std::ostringstream temp;
  temp << value;
  if (!unit.empty()) {
    temp << " " << unit;
  }
  return temp.str();
}

std::string IEvent::RangeToString() const {
  switch (Range()) {
    case RangeType::RangePoint:
      return "Point";
    case RangeType::RangeStart:
      return "Start";
    case RangeType::RangeEnd:
      return "End";
    default:
      break;
  }

  return {};
}

std::string IEvent::CauseToString() const {
  switch (Cause()) {
    case EventCause::CauseOther:
      return "Other";
    case EventCause::CauseError:
      return "Error";
    case EventCause::CauseTool:
      return "Tool";
    case EventCause::CauseScript:
      return "Script";
    case EventCause::CauseUser:
      return "User";
    default:
      break;
  }

  return {};
}

void IEvent::SetEvComment(const EvComment &ev_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(ev_comment.ToXml());
  }
}

void IEvent::GetEvComment(EvComment &ev_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    ev_comment.FromXml(meta_data->XmlSnippet());
  }
}

}  // namespace mdf