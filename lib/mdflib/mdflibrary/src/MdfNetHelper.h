#pragma once
#include <cstdint>


namespace MdfNetHelper {
uint64_t GetUnixNanoTimestamp(System::DateTime time);

uint64_t GetLocalNanoTimestamp(System::DateTime time);

System::DateTime GetDateTimeFromUnixNanoTimestamp(uint64_t time_ns);

System::DateTime GetDateTimeFromLocalNanoTimestamp(uint64_t time_ns);
} // namespace MdfNetHelper
