#include "MdfTimestamp.h"

#include "MdfNetHelper.h"

namespace MdfLibrary {
MdfUtcTimestamp::MdfUtcTimestamp(const uint64_t utc_timestamp) {
  utc_timestamp_ = utc_timestamp;
}

MdfUtcTimestamp::MdfUtcTimestamp(System::DateTime time) {
  utc_timestamp_ = MdfNetHelper::GetUnixNanoTimestamp(time);
}

std::unique_ptr<mdf::ITimestamp> MdfUtcTimestamp::GetTimestamp() {
  std::unique_ptr<mdf::UtcTimestamp> timestamp(
      new mdf::UtcTimestamp(utc_timestamp_));
  return std::move(timestamp);
}

MdfLocalTimestamp::MdfLocalTimestamp(const uint64_t local_timestamp) {
  local_timestamp_ = local_timestamp;
}

MdfLocalTimestamp::MdfLocalTimestamp(System::DateTime time) {
  local_timestamp_ = MdfNetHelper::GetLocalNanoTimestamp(time);
}

std::unique_ptr<mdf::ITimestamp> MdfLocalTimestamp::GetTimestamp() {
  std::unique_ptr<mdf::LocalTimestamp> timestamp(
      new mdf::LocalTimestamp(local_timestamp_));
  return std::move(timestamp);
}

MdfTimezoneTimestamp::MdfTimezoneTimestamp(const uint64_t utc_timestamp,
                                           const int16_t timezone_offset_min,
                                           const int16_t dst_offset_min) {
  utc_timestamp_ = utc_timestamp;
  timezone_offset_min_ = timezone_offset_min;
  dst_offset_min_ = dst_offset_min;
}

MdfTimezoneTimestamp::MdfTimezoneTimestamp(System::DateTime time,
                                           int16_t timezone_offset_min,
                                           int16_t dst_offset_min) {
  utc_timestamp_ = MdfNetHelper::GetUnixNanoTimestamp(time);
  timezone_offset_min_ = timezone_offset_min;
  dst_offset_min_ = dst_offset_min;
}

std::unique_ptr<mdf::ITimestamp> MdfTimezoneTimestamp::GetTimestamp() {
  std::unique_ptr<mdf::TimezoneTimestamp> timestamp(
      new mdf::TimezoneTimestamp(utc_timestamp_, timezone_offset_min_,
                                 dst_offset_min_));
  return std::move(timestamp);
}
}
