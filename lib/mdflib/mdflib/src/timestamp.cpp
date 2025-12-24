#include "mdf/itimestamp.h"

#include <cctype>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "mdf/mdfhelper.h"
#include "mdf/mdflogstream.h"

using namespace std::chrono;

namespace {
constexpr uint64_t kNanosecondsPerSecond = 1'000'000'000;
constexpr uint64_t kNanosecondsPerMinute = 60 * kNanosecondsPerSecond;
constexpr uint64_t kNanosecondsPerHour = 60 * kNanosecondsPerMinute;
constexpr uint32_t kSecondsPerMinute = 60;
constexpr uint32_t kSecondsPerHour = 60 * kSecondsPerMinute;
}  // namespace

namespace mdf {

UtcTimestamp::UtcTimestamp() {
  const auto now = system_clock::now();
  const auto ns_midnight = duration_cast<nanoseconds>(
                               now.time_since_epoch()) %
                           1000'000'000;
  const auto sec_1970 = system_clock::to_time_t(now);
  utc_timestamp_ = sec_1970;
  utc_timestamp_ *= 1'000'000'000;
  utc_timestamp_ += ns_midnight.count();
}

UtcTimestamp::UtcTimestamp(uint64_t utc_timestamp)
    : utc_timestamp_(utc_timestamp) {

}

UtcTimestamp::UtcTimestamp(const std::string &iso_date_time)
: utc_timestamp_(0) {
  FromIsoDateTime(iso_date_time);
}

uint64_t UtcTimestamp::GetTimeNs() const { return utc_timestamp_; }

int16_t UtcTimestamp::GetDstMin() const { return 0; }

int16_t UtcTimestamp::GetTimezoneMin() const { return 0; }

uint64_t UtcTimestamp::GetUtcTimeNs() const { return utc_timestamp_; }

std::string UtcTimestamp::ToIsoDate() const {
  const auto system_time = static_cast<std::time_t>(utc_timestamp_ / 1'000'000'000);
  std::ostringstream text;
  if (const struct tm *bt = std::gmtime(&system_time);
      bt != nullptr ) {
    text << std::put_time(bt, "%Y-%m-%dZ");
  }
  return text.str();
}

std::string UtcTimestamp::ToIsoTime(bool include_ms) const {
  const auto ms_sec = (utc_timestamp_ / 1'000'000) % 1'000;
  const auto system_time = static_cast<std::time_t>(utc_timestamp_ / 1'000'000'000);
  std::ostringstream text;
  if (struct tm *bt = std::gmtime(&system_time);
      bt != nullptr ) {
    text << std::put_time(bt, "%H:%M:%S");
  }
  if (ms_sec > 0 && include_ms) {
    text << '.' << std::setfill('0') << std::setw(3) << ms_sec;
  }
  text << "Z";
  return text.str();
}

std::string UtcTimestamp::ToIsoDateTime(bool include_ms) const {
  const auto ms_sec = (utc_timestamp_ / 1'000'000) % 1'000;
  const auto system_time = static_cast<std::time_t>(utc_timestamp_ / 1'000'000'000);
  std::ostringstream text;
  if (struct tm *bt = std::gmtime(&system_time);
      bt != nullptr ) {
    text << std::put_time(bt, "%Y-%m-%dT%H:%M:%S");
  }
  if (ms_sec > 0 && include_ms) {
    text << '.' << std::setfill('0') << std::setw(3) << ms_sec;
  }
  text << "Z";
  return text.str();
}

void UtcTimestamp::FromIsoDateTime(const std::string &date_time) {
  enum DateState {
    Start,
    Year,
    Month,
    Day,
    Hour,
    Minute,
    Second,
    MilliSecond,
    TzHour,
    TzMinute,
    End
  } state = DateState::Start;

  std::ostringstream year;
  std::ostringstream month;
  std::ostringstream day;

  std::ostringstream hour;
  std::ostringstream minute;
  std::ostringstream second;
  std::ostringstream millisecond;

  std::ostringstream tz_hour;
  std::ostringstream tz_minute;

  bool utc = false;

  for (char input : date_time) {
    switch (state) {
      case DateState::Start:
        if ( std::isdigit(input) ) {
          year << input;
          state = DateState::Year;
        } else if (input == 'T') {
          year << "1970";
          month << "01";
          day << "01";
          state = DateState::Hour;
        }
        break;

      case DateState::Year:
        if (input == '-') {
          state = DateState::Month;
        } else if (std::isdigit(input) && year.str().size() < 4) {
          year << input;
        } else if (std::isdigit(input) && year.str().size() >= 4) {
          month << input;
          state = DateState::Month;
        } else if (input == 'Z') {
          utc = true;
        }
        break;

      case DateState::Month:
        if (input == '-') {
          state = DateState::Day;
        } else if (std::isdigit(input) && month.str().size() < 2) {
          month << input;
        } else if (std::isdigit(input) && month.str().size() >= 2) {
          day << input;
          state = DateState::Day;
        } else if (input == 'Z') {
          utc = true;
        }
        break;

        case DateState::Day:
          if (input == ' ' || input == 'T') {
            state = DateState::Hour;
          } else if (std::isdigit(input) && day.str().size() < 2) {
            day << input;
          } else if (input == 'Z') {
            utc = true;
          } else if (input == '-' || input == '+') {
            tz_hour << input;
            state = DateState::TzHour;
          }
          break;

        case DateState::Hour:
          if (input == ':') {
            state = DateState::Minute;
          } else if (std::isdigit(input) && hour.str().size() < 2) {
            hour << input;
          } else if (std::isdigit(input) && hour.str().size() >= 2) {
            minute << input;
            state = DateState::Minute;
          } else if (input == 'Z') {
            utc = true;
          } else if (input == '-' || input == '+') {
            tz_hour << input;
            state = DateState::TzHour;
          }
          break;

        case DateState::Minute:
          if (input == ':') {
            state = DateState::Second;
          } else if (std::isdigit(input) && minute.str().size() < 2) {
            minute << input;
          } else if (std::isdigit(input) && minute.str().size() >= 2) {
            second << input;
            state = DateState::Second;
          } else if (input == 'Z') {
            utc = true;
          } else if (input == '-' || input == '+') {
            tz_hour << input;
            state = DateState::TzHour;
          }
          break;

        case DateState::Second:
          if (input == '.') {
            state = DateState::MilliSecond;
          } else if (std::isdigit(input) && second.str().size() < 2) {
            second << input;
          } else if (input == 'Z') {
            utc = true;
          } else if (input == '-' || input == '+') {
            tz_hour << input;
            state = DateState::TzHour;
          }
          break;

        case DateState::MilliSecond:
          if (input == '.') {
            state = DateState::MilliSecond;
          } else if (std::isdigit(input) && minute.str().size() < 9) {
            millisecond << input;
          } else if (input == 'Z') {
            utc = true;
          } else if (input == '-' || input == '+') {
            tz_hour << input;
            state = DateState::TzHour;
          }
          break;

        case DateState::TzHour:
          if (input == ':') {
            state = DateState::TzMinute;
          } else if (std::isdigit(input) ) {
            tz_hour << input;
          }
          break;

        case DateState::TzMinute:
          if (std::isdigit(input) && tz_minute.str().size() < 2) {
            tz_minute << input;
          } else if (input == 'Z') {
            utc = true;
            state = DateState::End;
          }
          break;

        default:
          break;
    }
  }
  try {
    std::tm date = {};
    // Years since 1900
    date.tm_year = year.str().empty() ? 70 : std::stoi(year.str()) - 1900;
    if (date.tm_year < 70) {
      date.tm_year = 70;
    }

    // Strange but 0..11
    date.tm_mon = month.str().empty() ? 0 : std::stoi(month.str()) - 1;
    if (date.tm_mon < 0 || date.tm_mon > 11) {
      date.tm_mon = 0;
    }

    // 1..31 as normal
    date.tm_mday = day.str().empty() ? 0 : std::stoi(day.str());
    if (date.tm_mday < 1 || date.tm_mday > 31) {
      date.tm_mday = 1;
    }

    // 0..23 as normal
    date.tm_hour = hour.str().empty() ? 0 : std::stoi(hour.str());
    if (date.tm_hour < 0 || date.tm_hour > 23) {
      date.tm_hour = 0;
    }

    // 0..59 as normal
    date.tm_min = minute.str().empty() ? 0 : std::stoi(minute.str());
    if (date.tm_min < 0 || date.tm_min > 59) {
      date.tm_min = 0;
    }

    // 0..59 as normal
    date.tm_sec = second.str().empty() ? 0 : std::stoi(second.str());
    if (date.tm_sec < 0 || date.tm_sec > 59) {
      date.tm_sec = 0;
    }
    date.tm_isdst = -1;

    int tz_offset = 0;  // Offset in seconds

    // Note that TZ offset includes DST
    tz_offset = tz_hour.str().empty() ? 0 : std::stoi(tz_hour.str());
    // MDF_TRACE() << tz_hour.str() << "/" << tz_offset << std::endl;
    tz_offset *= 60;  // Now minutes
    tz_offset += tz_minute.str().empty() ? 0 : std::stoi(tz_minute.str());
    tz_offset *= 60;  // Now in seconds

    // 0..59 as normal
    date.tm_min = minute.str().empty() ? 0 : std::stoi(minute.str());
    if (date.tm_min < 0 || date.tm_min > 59) {
      date.tm_min = 0;
    }


#ifdef _WIN32
    time_t utc_time = _mkgmtime(&date);
#else
    time_t utc_time = timegm(&date);
#endif
/*
    if (date.tm_isdst == 1) {
      time -= 3600;
    }
    time += MdfHelper::GmtOffsetNs() / 1'000'000'000;
*/
    // time should be second UTC
    if (!utc) {
      utc_time -= tz_offset;  // time is now UTC
    }

    utc_timestamp_ = utc_time;
    utc_timestamp_ *= 1'000'000'000;  // Now ns
    if (millisecond.str().empty()) {
      return;
    } else if (millisecond.str().size() <= 3) {
      utc_timestamp_ += std::stoull(millisecond.str()) * 1'000'000;
    } else if (millisecond.str().size() <= 6) {
      utc_timestamp_ += std::stoull(millisecond.str()) * 1'000;
    } else {
      utc_timestamp_ += std::stoull(millisecond.str());
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Invalid cast: " << err.what();
  }
}


LocalTimestamp::LocalTimestamp(uint64_t local_timestamp)
    : local_timestamp_(local_timestamp) {
  timezone_offset_min_ = static_cast<int16_t>(MdfHelper::GmtOffsetNs() /
                                              kNanosecondsPerMinute);
  // The DST offset is dependent on the local time
  // local = utc + tz + dst(utc)
  // The below solution assume that the local timestamp is close to
  // the current time
  dst_offset_min_ = static_cast<int16_t>(MdfHelper::DstOffsetNs() /
                                         kNanosecondsPerMinute);
}

uint64_t LocalTimestamp::GetTimeNs() const { return local_timestamp_; }

int16_t LocalTimestamp::GetTimezoneMin() const { return timezone_offset_min_; }

int16_t LocalTimestamp::GetDstMin() const { return dst_offset_min_; }

uint64_t LocalTimestamp::GetUtcTimeNs() const {
  return local_timestamp_ -
         (MdfHelper::TimeZoneOffset() * kNanosecondsPerSecond);
}


TimezoneTimestamp::TimezoneTimestamp(uint64_t utc_timestamp,
                                     int16_t timezone_offset_min,
                                     int16_t dst_offset_min)
    : utc_timestamp_(utc_timestamp),
      timezone_offset_min_(timezone_offset_min),
      dst_offset_min_(dst_offset_min) {}

uint64_t TimezoneTimestamp::GetTimeNs() const { return utc_timestamp_; }
int16_t TimezoneTimestamp::GetTimezoneMin() const {
  return timezone_offset_min_;
}
int16_t TimezoneTimestamp::GetDstMin() const { return dst_offset_min_; }
uint64_t TimezoneTimestamp::GetUtcTimeNs() const { return utc_timestamp_; }

}  // namespace mdf