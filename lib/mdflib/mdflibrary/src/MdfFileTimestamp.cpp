#include "MdfFileTimestamp.h"

#include "MdfNetHelper.h"


namespace MdfLibrary {
MdfFileTimezoneTimestamp::MdfFileTimezoneTimestamp(const uint64_t time_ns,
                                                   const int16_t timezone_min,
                                                   const int16_t dst_min) {
  time_ns_ = time_ns;
  timezone_min_ = timezone_min;
  dst_min_ = dst_min;
}

uint64_t MdfFileTimezoneTimestamp::TimeNs::get() {
  return time_ns_;
}

int16_t MdfFileTimezoneTimestamp::TimezoneMin::get() {
  return timezone_min_;
}

int16_t MdfFileTimezoneTimestamp::DstMin::get() {
  return dst_min_;
}

DateTime MdfFileTimezoneTimestamp::Time::get() {
  auto epoch = DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);
  return epoch.AddTicks(time_ns_ / 100);
}

IMdfFileTimestamp^ GetMdfFileTimestampByIMdfTimestamp(
    const mdf::IMdfTimestamp* timestamp) {
  switch (timestamp->GetTimeType()) {
    case mdf::timetype::kUtcTime:
      return gcnew MdfFileUtcTimestamp(timestamp->GetTimeNs());
    case mdf::timetype::kLocalTime:
      return gcnew MdfFileLocalTimestamp(timestamp->GetTimeNs());
    case mdf::timetype::kLocalTimeTz:
      return gcnew MdfFileLocalTimestampTz(timestamp->GetTimeNs(),
                                           timestamp->GetTzOffsetMin());
    case mdf::timetype::kTimezoneTime:
      return gcnew MdfFileTimezoneTimestamp(timestamp->GetTimeNs(),
                                            timestamp->GetTzOffsetMin(),
                                            timestamp->GetDstOffsetMin());
    default:
      throw gcnew System::ArgumentOutOfRangeException("Unknown timestamp type");
  }
}

MdfFileUtcTimestamp::MdfFileUtcTimestamp(uint64_t utc_time_ns) {
  utc_time_ns_ = utc_time_ns;
}

uint64_t MdfFileUtcTimestamp::TimeNs::get() {
  return utc_time_ns_;
}

int16_t MdfFileUtcTimestamp::TimezoneMin::get() {
  return 0;
}

int16_t MdfFileUtcTimestamp::DstMin::get() {
  return 0;
}

DateTime MdfFileUtcTimestamp::Time::get() {
  return MdfNetHelper::GetDateTimeFromUnixNanoTimestamp(utc_time_ns_);
}

MdfFileLocalTimestamp::MdfFileLocalTimestamp(uint64_t local_time_ns) {
  local_time_ns_ = local_time_ns;
}

uint64_t MdfFileLocalTimestamp::TimeNs::get() {
  return local_time_ns_;
}

int16_t MdfFileLocalTimestamp::TimezoneMin::get() {
  return 0;
}

int16_t MdfFileLocalTimestamp::DstMin::get() {
  return 0;
}

DateTime MdfFileLocalTimestamp::Time::get() {
  return MdfNetHelper::GetDateTimeFromLocalNanoTimestamp(local_time_ns_);
}

MdfFileLocalTimestampTz::MdfFileLocalTimestampTz(uint64_t local_time_ns,
                                                 int16_t tz_min) {
  local_time_ns_ = local_time_ns;
  tz_min_ = tz_min;
}

uint64_t MdfFileLocalTimestampTz::TimeNs::get() {
  return local_time_ns_;
}

int16_t MdfFileLocalTimestampTz::TimezoneMin::get() {
  return tz_min_;
}

int16_t MdfFileLocalTimestampTz::DstMin::get() {
  return 0;
}

DateTime MdfFileLocalTimestampTz::Time::get() {
  auto epoch = DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);
  int64_t ticks = local_time_ns_ / 100;
  auto offset = System::TimeSpan::FromMinutes(tz_min_);
  ticks -= offset.Ticks;
  return epoch.AddTicks(ticks);
}
}
