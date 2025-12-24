#pragma once

#include "mdf/itimestamp.h"

namespace mdf {

namespace timetype
{
enum MdfTimestampType {
 kUtcTime, /**< Represents the UTC time. */
 kLocalTime, /**< Represents the local time. */
 kLocalTimeTz, /**< Represents the local time with timezone offset. */
 kTimezoneTime /**< Represents the time with timezone offset. */
};
}

class ITimestamp;

/**
 * \brief Interface for MDF timestamp handling.
 */
class IMdfTimestamp {
 public:
  /**
   * \brief Set the time in nanoseconds.
   * \param time The time in nanoseconds.
   */
  virtual void SetTime(uint64_t time) = 0;
  /**
   * \brief Set the time using an ITimestamp object.
   * \param timestamp An ITimestamp object representing the time.
   */
  virtual void SetTime(ITimestamp &timestamp) = 0;
  /**
   * \brief Get the time in nanoseconds.
   * \return The time in nanoseconds.
   */
  [[nodiscard]] virtual uint64_t GetTimeNs() const = 0;
  /**
   * \brief Get the timezone offset in minutes.
   * \return The timezone offset in minutes.
   */
  [[nodiscard]] virtual uint16_t GetTzOffsetMin() const = 0;
  /**
   * \brief Get the daylight saving time offset in minutes.
   * \return The daylight saving time offset in minutes.
   */
  [[nodiscard]] virtual uint16_t GetDstOffsetMin() const = 0;
  /**
   * \brief Get the type of MDF timestamp.
   * \return The MDF timestamp type.
   */
  [[nodiscard]] virtual timetype::MdfTimestampType GetTimeType() const = 0;
};
}  // namespace mdf