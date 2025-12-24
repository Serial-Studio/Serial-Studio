#include "timestamp.h"
namespace util::time {
uint64_t TimeStampToNs(TimeStamp timestamp) {
  // 1.na since midnight
  // 2 sec since 1970
  // 3 Add them
  const auto ns_midnight = std::chrono::duration_cast<std::chrono::nanoseconds>(
                               timestamp.time_since_epoch()) %
                           1000'000'000;
  const auto sec_1970 = std::chrono::system_clock::to_time_t(timestamp);
  uint64_t ns = sec_1970;
  ns *= 1'000'000'000;
  ns += ns_midnight.count();
  return ns;
}
std::string GetLocalTimestampWithMs(
    std::chrono::time_point<std::chrono::system_clock> timestamp) {
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      timestamp.time_since_epoch()) %
                  1000;
  const auto timer = std::chrono::system_clock::to_time_t(timestamp);
  const struct tm* bt = localtime(&timer);

  std::ostringstream text;
  text << std::put_time(bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0')
       << std::setw(3) << ms.count();
  return text.str();
}
std::string NsToLocalDate(uint64_t ns_since_1970) {
  const auto system_time =
      static_cast<std::time_t>(ns_since_1970 / 1'000'000'000);
  const struct tm* bt = localtime(&system_time);
  std::ostringstream text;
  text << std::put_time(bt, "%x");
  return text.str();
}

std::string NsToLocalTime(uint64_t ns_since_1970, int format) {
  const auto system_time =
      static_cast<std::time_t>(ns_since_1970 / 1'000'000'000);
  const struct tm* bt = localtime(&system_time);
  std::ostringstream text;
  text << std::put_time(bt, "%X");

  std::ostringstream extra;
  switch (format) {
    case 1: {  // Show ms
      const auto ms_sec = (ns_since_1970 / 1'000'000) % 1'000;
      extra << '.' << std::setfill('0') << std::setw(3) << ms_sec;
      break;
    }

    case 2: {  // Show us
      const auto us_sec = (ns_since_1970 / 1'000) % 1'000'000;
      extra << '.' << std::setfill('0') << std::setw(6) << us_sec;
      break;
    }

    case 3: {  // Show ns
      const auto ns_sec = (ns_since_1970) % 1'000'000'000;
      extra << '.' << std::setfill('0') << std::setw(9) << ns_sec;
      break;
    }

    case 0:  // Show seconds
    default:
      return text.str();
  }

  const std::string input = text.str();
  std::ostringstream output;
  bool is_am_pm = false;
  bool is_blank = false;
  for (const char char_in : input) {
    if (std::isalpha(char_in) && !is_am_pm) {  // Assume AM PM and insert ms
      output << extra.str();
      if (is_blank) {
        output << " ";
        is_blank = false;
      }
      is_am_pm = true;
      output << char_in;
    } else if (iswspace(char_in)) {
      is_blank = true;
    } else {
      if (is_blank) {
        output << " ";
        is_blank = false;
      }
      output << char_in;
    }
  }
  if (!is_am_pm) {
    output << extra.str();
  }
  return output.str();
}

}  // namespace util::time