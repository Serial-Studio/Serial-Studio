#pragma once
#include <string>
#include <cstdint>

namespace mdf {

/**
 * \brief Interface for timestamp handling in MDF files.
 */
class ITimestamp {
 public:
  /**
   * \brief Virtual destructor for ITimestamp.
   */
  virtual ~ITimestamp() = default;
  /**
   * \brief Get the time in nanoseconds.
   * \return Time in nanoseconds.
   */
  [[nodiscard]] virtual uint64_t GetTimeNs() const = 0;
  /**
   * \brief Get the timezone offset in minutes.
   * \return Timezone offset in minutes.
   */
  [[nodiscard]] virtual int16_t GetTimezoneMin() const = 0;
  /**
   * \brief Get the daylight saving time offset in minutes.
   * \return DST offset in minutes.
   */
  [[nodiscard]] virtual int16_t GetDstMin() const = 0;
  /**
   * \brief Get the UTC time in nanoseconds.
   * \return UTC time in nanoseconds.
   */
  [[nodiscard]] virtual uint64_t GetUtcTimeNs() const = 0;

};

/**
 * \brief Class representing a UTC timestamp.
 */
class UtcTimestamp : public ITimestamp {
 public:

  UtcTimestamp();
  /**
   * \brief Constructor for UtcTimeStamp.
   * \param utc_timestamp The UTC timestamp in nanoseconds.
   */
  explicit UtcTimestamp(uint64_t utc_timestamp);
  explicit UtcTimestamp(const std::string& iso_date_time);

  [[nodiscard]] uint64_t GetTimeNs() const override;
  [[nodiscard]] int16_t GetTimezoneMin() const override;
  [[nodiscard]] int16_t GetDstMin() const override;
  [[nodiscard]] uint64_t GetUtcTimeNs() const override;

  [[nodiscard]] std::string ToIsoDate() const;
  [[nodiscard]] std::string ToIsoTime(bool include_ms) const;
  [[nodiscard]] std::string ToIsoDateTime(bool include_ms) const;

  void FromIsoDateTime(const std::string& date_time);

 private:
  uint64_t utc_timestamp_;  ///< The UTC timestamp in nanoseconds.
};

/**
 * \brief Class representing a local timestamp, with timezone and DST offset.
 */
class LocalTimestamp : public ITimestamp {
 public:

  /**
   * \brief Constructor for LocalTimeStamp.
   * \param local_timestamp The local timestamp in nanoseconds, with timezone
   * and DST offset.
   */
  explicit LocalTimestamp(uint64_t local_timestamp);
  [[nodiscard]] uint64_t GetTimeNs() const override;
  [[nodiscard]] int16_t GetTimezoneMin() const override;
  [[nodiscard]] int16_t GetDstMin() const override;
  [[nodiscard]] uint64_t GetUtcTimeNs() const override;


 private:
  uint64_t local_timestamp_;  ///< The local timestamp in nanoseconds, with timezone and DST offset.
  int16_t timezone_offset_min_ = 0;  ///< The timezone offset in minutes.
  int16_t dst_offset_min_ = 0;  ///< The daylight saving time offset in minutes.
};

/**
 * \brief Class representing a timestamp with timezone information.
 */
class TimezoneTimestamp : public ITimestamp {
 public:
  /**
   * \brief Constructor for TimezoneTimeStamp.
   * \param utc_timestamp The UTC timestamp in nanoseconds.
   * \param timezone_offset_min The timezone offset in minutes.
   * \param dst_offset_min The daylight saving time offset in minutes.
   */
  TimezoneTimestamp(uint64_t utc_timestamp, int16_t timezone_offset_min,
                    int16_t dst_offset_min);
  [[nodiscard]] uint64_t GetTimeNs() const override;
  [[nodiscard]] int16_t GetTimezoneMin() const override;
  [[nodiscard]] int16_t GetDstMin() const override;
  [[nodiscard]] uint64_t GetUtcTimeNs() const override;

 private:
  uint64_t utc_timestamp_;       ///< The UTC timestamp in nanoseconds.
  int16_t timezone_offset_min_;  ///< The timezone offset in minutes.
  int16_t dst_offset_min_;  ///< The daylight saving time offset in minutes.
};
}  // namespace mdf