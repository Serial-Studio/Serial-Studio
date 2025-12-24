#include "MdfNetHelper.h"

uint64_t MdfNetHelper::GetUnixNanoTimestamp(System::DateTime time) {
  auto epoch =
      System::DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);
  return time.ToUniversalTime().Subtract(epoch).Ticks * 100;
}

uint64_t MdfNetHelper::GetLocalNanoTimestamp(System::DateTime time) {
  auto epoch =
      System::DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);
  System::DateTime local_time = time.ToLocalTime();
  return local_time.Subtract(epoch).Ticks * 100;
}

System::DateTime MdfNetHelper::GetDateTimeFromUnixNanoTimestamp(
    uint64_t time_ns) {
  auto epoch =
      System::DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);
  return epoch.AddTicks(time_ns / 100);
}

System::DateTime MdfNetHelper::GetDateTimeFromLocalNanoTimestamp(
    uint64_t local_time_ns) {
  auto epoch = System::DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);
  int64_t ticks = local_time_ns / 100;
  auto offset = System::TimeZoneInfo::Local->GetUtcOffset(epoch);
  ticks -= offset.Ticks;
  return epoch.AddTicks(ticks);
}
