#pragma once
#include <cstdint>
#include <memory>

#include "mdf/itimestamp.h"

namespace MdfLibrary {
public interface class IMdfTimeStamp {
  std::unique_ptr<mdf::ITimestamp> GetTimestamp();
};


public ref class MdfUtcTimestamp : public IMdfTimeStamp {
public:
  explicit MdfUtcTimestamp(uint64_t utc_timestamp);
  explicit MdfUtcTimestamp(System::DateTime time);
  virtual std::unique_ptr<mdf::ITimestamp> GetTimestamp();

private:
  uint64_t utc_timestamp_;
};


public ref class MdfLocalTimestamp : public IMdfTimeStamp {
public:
  explicit MdfLocalTimestamp(uint64_t local_timestamp);
  explicit MdfLocalTimestamp(System::DateTime time);
  virtual std::unique_ptr<mdf::ITimestamp> GetTimestamp();

private:
  uint64_t local_timestamp_;
};


public ref class MdfTimezoneTimestamp : public IMdfTimeStamp {
public:
  explicit MdfTimezoneTimestamp(uint64_t utc_timestamp,
                                int16_t timezone_offset_min,
                                int16_t dst_offset_min);
  explicit MdfTimezoneTimestamp(System::DateTime time,
                                int16_t timezone_offset_min,
                                int16_t dst_offset_min);
  virtual std::unique_ptr<mdf::ITimestamp> GetTimestamp();

private:
  uint64_t utc_timestamp_;
  int16_t timezone_offset_min_;
  int16_t dst_offset_min_;
};
}
