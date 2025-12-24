#include "mdf3timestamp.h"

#include "mdf/mdfhelper.h"
namespace {
constexpr uint64_t kNanosecondsPerSecond = 1'000'000'000;
constexpr uint64_t kNanosecondsPerMinute = 60 * kNanosecondsPerSecond;
constexpr uint64_t kNanosecondsPerHour = 60 * kNanosecondsPerMinute;
constexpr uint32_t kSecondsPerMinute = 60;
constexpr uint32_t kSecondsPerHour = 60 * kSecondsPerMinute;
}  // namespace
namespace mdf::detail {

void Mdf3Timestamp::GetBlockProperty(detail::BlockPropertyList& dest) const {
  MdfBlock::GetBlockProperty(dest);
}

uint64_t Mdf3Timestamp::Read(std::streambuf& buffer) { return 0; }

uint64_t Mdf3Timestamp::Write(std::streambuf& buffer) {
  return MdfBlock::Write(buffer);
}

void Mdf3Timestamp::SetTime(uint64_t time) {
  date_ = MdfHelper::NanoSecToDDMMYYYY(time);
  time_ = MdfHelper::NanoSecToHHMMSS(time);
  local_timestamp_ = time + MdfHelper::GmtOffsetNs();
  utc_offset_ = static_cast<int16_t>(MdfHelper::GmtOffsetNs() /
                                     kNanosecondsPerHour);
  time_quality_ = 0;
  timer_id_ = "Local PC Reference Time";
}

void Mdf3Timestamp::SetTime(mdf::ITimestamp& timestamp) {
  if (dynamic_cast<UtcTimestamp*>(&timestamp)) {
    SetTime(timestamp.GetTimeNs());
    return;
  }

  if (auto local_time = dynamic_cast<LocalTimestamp*>(&timestamp)) {
    date_ = MdfHelper::NanoSecUtcToDDMMYYYY(local_time->GetTimeNs());
    time_ = MdfHelper::NanoSecUtcToHHMMSS(local_time->GetTimeNs());
    local_timestamp_ = timestamp.GetTimeNs() - MdfHelper::DstOffsetNs();
    utc_offset_ = static_cast<int16_t>(MdfHelper::GmtOffsetNs() /
                                       kNanosecondsPerHour);
    time_quality_ = 0;
    timer_id_ = "Local PC Reference Time";
    return;
  }

  if (auto tz = dynamic_cast<TimezoneTimestamp*>(&timestamp)) {
    date_ = MdfHelper::NanoSecTzToDDMMYYYY(
        timestamp.GetTimeNs(), tz->GetTimezoneMin(), tz->GetDstMin());
    time_ = MdfHelper::NanoSecTzToHHMMSS(timestamp.GetTimeNs(),
                                         tz->GetTimezoneMin(), tz->GetDstMin());
    local_timestamp_ = timestamp.GetTimeNs() +
                       tz->GetTimezoneMin() * kNanosecondsPerMinute;
    utc_offset_ = static_cast<int16_t>(timestamp.GetTimezoneMin() / 60);
    time_quality_ = 0;
    timer_id_ = "Local PC Reference Time";
  }
}

uint64_t Mdf3Timestamp::GetTimeNs() const { return local_timestamp_; }
uint16_t Mdf3Timestamp::GetTzOffsetMin() const { return utc_offset_ * 60; }
uint16_t Mdf3Timestamp::GetDstOffsetMin() const {
  // mdf3 timestamp does not have DST offset
  return 0;
}

timetype::MdfTimestampType Mdf3Timestamp::GetTimeType() const {
  return timetype::kLocalTimeTz;
}
}  // namespace mdf::detail
