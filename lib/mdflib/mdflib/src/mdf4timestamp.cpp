/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf4timestamp.h"

#include <chrono>

namespace mdf::detail {

void Mdf4Timestamp::GetBlockProperty(detail::BlockPropertyList &dest) const {
  dest.emplace_back("ISO Time", GetTimeString());
  dest.emplace_back("TZ Offset [min]", std::to_string(tz_offset_));
  dest.emplace_back("DST Offset [min]", std::to_string(dst_offset_));

  std::ostringstream flags;
  flags << (flags_ & TimestampFlag::kLocalTimestamp ? "Local" : "UTC") << ","
        << (flags_ & TimestampFlag::kTimeOffsetValid ? "Offset" : "No Offset");

  dest.emplace_back("Time Flags", flags.str());
}

uint64_t Mdf4Timestamp::Read(std::streambuf& buffer) {
  file_position_ = detail::GetFilePosition(buffer);
  uint64_t bytes = ReadNumber(buffer, time_);
  bytes += ReadNumber(buffer, tz_offset_);
  bytes += ReadNumber(buffer, dst_offset_);
  bytes += ReadNumber(buffer, flags_);
  return bytes;
}

uint64_t Mdf4Timestamp::Write(std::streambuf& buffer) {
  if (file_position_ <= 0) {
    file_position_ = detail::GetFilePosition(buffer);
  } else {
    detail::SetFilePosition(buffer, file_position_);
  }
  uint64_t bytes = WriteNumber(buffer, time_);
  bytes += WriteNumber(buffer, tz_offset_);
  bytes += WriteNumber(buffer, dst_offset_);
  bytes += WriteNumber(buffer, flags_);
  return bytes;
}

void Mdf4Timestamp::SetTime(uint64_t time) {
  time_ = time;
  tz_offset_ = 0;
  dst_offset_ = 0;
  flags_ = TimestampFlag::kUtcTimestamp;
}
void Mdf4Timestamp::SetTime(ITimestamp &timestamp) {
  time_ = timestamp.GetTimeNs();
  if (dynamic_cast<UtcTimestamp *>(&timestamp)) {
    flags_ = TimestampFlag::kUtcTimestamp;
    tz_offset_ = 0;
    dst_offset_ = 0;
  } else if (dynamic_cast<LocalTimestamp *>(&timestamp)) {
    flags_ = TimestampFlag::kLocalTimestamp;
    tz_offset_ = 0;
    dst_offset_ = 0;
  } else if (dynamic_cast<TimezoneTimestamp *>(&timestamp)) {
    flags_ = TimestampFlag::kTimeOffsetValid;
    tz_offset_ = timestamp.GetTimezoneMin();
    dst_offset_ = timestamp.GetDstMin();
  } else {
    throw std::runtime_error("Unknown timestamp type");
  }
}

Mdf4Timestamp::Mdf4Timestamp()
    : time_(0), tz_offset_(0), dst_offset_(0), flags_(0) {}

std::string Mdf4Timestamp::GetTimeString() const {
  using namespace std::chrono;
  nanoseconds ns(time_);
  system_clock::time_point tp(duration_cast<system_clock::duration>(ns));
  minutes tz_offset(tz_offset_);
  minutes dst_offset(dst_offset_);
  tp += tz_offset + dst_offset;

  std::time_t time_t_tp = system_clock::to_time_t(tp);
  std::tm *tm_tp = std::gmtime(&time_t_tp);

  std::ostringstream date_time;
  date_time << std::put_time(tm_tp, "%Y-%m-%d %H:%M:%S");
  date_time << " + " << time_ % 1000'000'000 << " ns";

  if (flags_ & TimestampFlag::kUtcTimestamp) {
    date_time << " [UTC]";
  } else if (flags_ & TimestampFlag::kTimeOffsetValid) {
    date_time << "[GMT";
    if (tz_offset_ >= 0) {
      date_time << "+" << tz_offset_ / 60;
    }
    date_time << ", ";
    if (dst_offset_ > 0) {
      date_time << "DST+" << dst_offset_ / 60 << "h]";
    } else {
      date_time << "no DST]";
    }
  } else if (flags_ & TimestampFlag::kLocalTimestamp) {
    date_time << " [Local]";
  }

  return date_time.str();
}
uint64_t Mdf4Timestamp::GetTimeNs() const { return time_; }
uint16_t Mdf4Timestamp::GetTzOffsetMin() const { return tz_offset_; }
uint16_t Mdf4Timestamp::GetDstOffsetMin() const { return dst_offset_; }

timetype::MdfTimestampType Mdf4Timestamp::GetTimeType() const {
  if (flags_ & TimestampFlag::kTimeOffsetValid) {
    return timetype::kTimezoneTime;
  }
  if (flags_ & TimestampFlag::kLocalTimestamp) {
    return timetype::kLocalTime;
  }
  return timetype::kUtcTime;
}
}  // namespace mdf::detail